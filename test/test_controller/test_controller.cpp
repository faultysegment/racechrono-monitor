#include <unity.h>
#include "../../src/Model.h"
#include "../../src/View.h"
#include "../../src/Controller.h"
#include "../../src/Device_Mock/Policies/MockDisplayPolicy.h"
#include "../../src/Device_Mock/Policies/MockHWPolicy.h"
#include "../../src/Device_Mock/Policies/MockBLEPolicy.h"
#include "../../src/Device_Mock/Policies/MockStoragePolicy.h"
#include "../../src/Device_T_Embed_CC1101/Screens/MonitorScreen.h"
#include "../../src/Device_T_Embed_CC1101/Screens/DualMonitorScreen.h"
#include "../../src/Device_T_Embed_CC1101/Screens/DisconnectedMsgScreen.h"
#include "../../src/Device_T_Embed_CC1101/Screens/ConfigMonitorScreen.h"
#ifdef ARDUINO
#include <Arduino.h>
#endif

Model model;
View<MockDisplayPolicy, MockHWPolicy> view(model);

MonitorScreen<MockDisplayPolicy> monitorScreen0(0);
MonitorScreen<MockDisplayPolicy> monitorScreen1(1);
DualMonitorScreen<MockDisplayPolicy> dualMonitorScreen;
DisconnectedMsgScreen<MockDisplayPolicy> disconnectedMsgScreen;
ConfigMonitorScreen<MockDisplayPolicy> configSpeedScreen("SPEED LIMIT", &model.speedLimit);
ConfigMonitorScreen<MockDisplayPolicy> configTimeScreen("TIME LIMIT", &model.timeLimit);
using TestController = Controller<Model, View<MockDisplayPolicy, MockHWPolicy>, MockBLEPolicy, MockHWPolicy, MockStoragePolicy>;

TestController controller(model, view);

void setUp(void) {
    model.reset();
    MockDisplayPolicy::reset();
    MockHWPolicy::reset();
    MockStoragePolicy::reset();
    MockBLEPolicy::reset();

    if (view.getNumConnectedScreens() == 0) {
        view.addConnectedScreen(&monitorScreen0);
        view.addConnectedScreen(&monitorScreen1);
        view.addConnectedScreen(&dualMonitorScreen);
        
        view.addDisconnectedScreen(&disconnectedMsgScreen);
        view.addDisconnectedScreen(&configSpeedScreen);
        view.addDisconnectedScreen(&configTimeScreen);
    }
}

void tearDown(void) {}

void test_controller_ble_connect(void) {
    controller.setup();
    controller.onBLEConnected();
    // In our loop logic, isConnected is updated in loop().
    // But onBLEConnected also calls view.showConnected() which sets displayStarted = false.
    TEST_ASSERT_TRUE(MockDisplayPolicy::lastPrint.find("BLE connected!") != std::string::npos);
}

void test_controller_button_power_off(void) {
    MockHWPolicy::powerKeyPressed = true;
    MockHWPolicy::currentMillis = 0;
    
    controller.loop(); // Detects press
    
    MockHWPolicy::currentMillis = 3500; // 3.5s later
    controller.loop(); // Triggers power off
    
    TEST_ASSERT_TRUE(MockHWPolicy::sleepCalled);
    TEST_ASSERT_TRUE(MockDisplayPolicy::lastPrint.find("Powering off...") != std::string::npos);
}

void test_controller_navigation_scroll(void) {
    controller.setup();
    MockBLEPolicy::simulateConnect();
    model.isConfigured = true; // And configured
    model.currentScreenIndex = 0;

    // Simulate navigation right (+1)
    MockHWPolicy::navigationDelta = 1;
    controller.loop();
    TEST_ASSERT_EQUAL(1, model.currentScreenIndex);

    // Simulate navigation left (-1)
    MockHWPolicy::navigationDelta = -1;
    controller.loop();
    TEST_ASSERT_EQUAL(0, model.currentScreenIndex);

    // Turning left again should wrap around to NUM_SCREENS - 1 (which is 2)
    MockHWPolicy::navigationDelta = -1;
    controller.loop();
    TEST_ASSERT_EQUAL(2, model.currentScreenIndex);
}

void test_controller_edit_mode(void) {
    controller.setup();
    MockBLEPolicy::simulateConnect();
    model.isConfigured = true;
    model.currentScreenIndex = 0; // Time Screen
    model.timeLimit = 0.1f;
    model.addMonitor("M", 1.0f, "TIME", false, 2, &model.timeLimit);
    model.isEditMode = false;

    // Simulate action button short press
    MockHWPolicy::actionKeyPressed = true; // Pressed
    MockHWPolicy::currentMillis = 1000;
    controller.loop();
    
    MockHWPolicy::actionKeyPressed = false; // Released
    MockHWPolicy::currentMillis = 1200; // 200ms duration
    controller.loop();
    
    TEST_ASSERT_TRUE(model.isEditMode);

    // Navigate right
    MockHWPolicy::navigationDelta = 1;
    controller.loop();
    
    // limit should increase by 0.1
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 0.2f, model.timeLimit);
    
    // Navigate left
    MockHWPolicy::navigationDelta = -1;
    controller.loop();
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 0.1f, model.timeLimit);

    // Simulate action button short press again to save
    MockHWPolicy::actionKeyPressed = true; // Pressed
    MockHWPolicy::currentMillis = 2000;
    controller.loop();
    
    MockHWPolicy::actionKeyPressed = false; // Released
    MockHWPolicy::currentMillis = 2200; // 200ms duration
    controller.loop();
    
    TEST_ASSERT_FALSE(model.isEditMode);
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 0.1f, MockStoragePolicy::store["limit_time"]);
}

#ifdef ARDUINO
void setup() {
    delay(2000);
    UNITY_BEGIN();
    RUN_TEST(test_controller_ble_connect);
    RUN_TEST(test_controller_button_power_off);
    RUN_TEST(test_controller_navigation_scroll);
    RUN_TEST(test_controller_edit_mode);
    UNITY_END();
}
void loop() {}
#else
int main(int argc, char **argv) {
    UNITY_BEGIN();
    RUN_TEST(test_controller_ble_connect);
    RUN_TEST(test_controller_button_power_off);
    RUN_TEST(test_controller_navigation_scroll);
    RUN_TEST(test_controller_edit_mode);
    UNITY_END();
    return 0;
}
#endif
