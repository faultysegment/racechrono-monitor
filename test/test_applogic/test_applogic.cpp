#include <unity.h>
#include "../../src/AppState.h"
#include "../../src/View.h"
#include "../../src/AppLogic.h"
#include "../../src/Device_All/EventBus.h"
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

AppState state;
View<MockDisplayPolicy, MockHWPolicy> view(state);

MonitorScreen<MockDisplayPolicy> monitorScreen0(0);
MonitorScreen<MockDisplayPolicy> monitorScreen1(1);
DualMonitorScreen<MockDisplayPolicy> dualMonitorScreen;
DisconnectedMsgScreen<MockDisplayPolicy> disconnectedMsgScreen;
ConfigMonitorScreen<MockDisplayPolicy> configSpeedScreen("SPEED LIMIT", &state.speedLimit);
ConfigMonitorScreen<MockDisplayPolicy> configTimeScreen("TIME LIMIT", &state.timeLimit);
using TestAppLogic = AppLogic<MockBLEPolicy, MockHWPolicy, MockStoragePolicy>;

TestAppLogic logic(state);

void flushEvents() {
    Event e;
    while(globalEventBus.try_pop(e)) {
        view.processEvent(e);
        logic.processEvent(e);
    }
}

void setUp(void) {
    state.reset();
    MockDisplayPolicy::reset();
    MockHWPolicy::reset();
    MockStoragePolicy::reset();
    MockBLEPolicy::reset();

    // flush queue
    Event e;
    while(globalEventBus.try_pop(e)) {}

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

void test_applogic_ble_connect(void) {
    logic.setup();
    globalEventBus.push(Event{EventType::BLE_CONNECTED, 0, 0, 0});
    flushEvents();
    TEST_ASSERT_TRUE(MockDisplayPolicy::lastPrint.find("BLE connected!") != std::string::npos);
}

void test_applogic_button_power_off(void) {
    logic.setup();
    MockHWPolicy::powerKeyPressed = true;
    MockHWPolicy::currentMillis = 0;
    
    logic.pollInput();
    flushEvents();
    
    MockHWPolicy::currentMillis = 3500; // 3.5s later
    logic.pollInput();
    flushEvents();
    
    TEST_ASSERT_TRUE(MockHWPolicy::sleepCalled);
    TEST_ASSERT_TRUE(MockDisplayPolicy::lastPrint.find("Powering off...") != std::string::npos);
}

void test_applogic_navigation_scroll(void) {
    logic.setup();
    MockBLEPolicy::simulateConnect();
    state.isConfigured = true;
    state.isConnected = true;
    state.currentScreenIndex = 0;

    MockHWPolicy::navigationDelta = 1;
    logic.pollInput();
    flushEvents();
    TEST_ASSERT_EQUAL(1, state.currentScreenIndex);

    MockHWPolicy::navigationDelta = -1;
    logic.pollInput();
    flushEvents();
    TEST_ASSERT_EQUAL(0, state.currentScreenIndex);

    MockHWPolicy::navigationDelta = -1;
    logic.pollInput();
    flushEvents();
    TEST_ASSERT_EQUAL(2, state.currentScreenIndex);
}

void test_applogic_edit_mode(void) {
    logic.setup();
    MockBLEPolicy::simulateConnect();
    state.isConfigured = true;
    state.isConnected = true;
    state.currentScreenIndex = 0;
    state.timeLimit = 0.1f;
    state.addMonitor("M", 1.0f, "TIME", false, 2, &state.timeLimit);
    state.isEditMode = false;

    MockHWPolicy::actionKeyPressed = true;
    MockHWPolicy::currentMillis = 1000;
    logic.pollInput();
    flushEvents();
    
    MockHWPolicy::actionKeyPressed = false;
    MockHWPolicy::currentMillis = 1200;
    logic.pollInput();
    flushEvents();
    
    TEST_ASSERT_TRUE(state.isEditMode);

    MockHWPolicy::navigationDelta = 1;
    logic.pollInput();
    flushEvents();
    
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 0.2f, state.timeLimit);
    
    MockHWPolicy::navigationDelta = -1;
    logic.pollInput();
    flushEvents();
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 0.1f, state.timeLimit);

    MockHWPolicy::actionKeyPressed = true;
    MockHWPolicy::currentMillis = 2000;
    logic.pollInput();
    flushEvents();
    
    MockHWPolicy::actionKeyPressed = false;
    MockHWPolicy::currentMillis = 2200;
    logic.pollInput();
    flushEvents();
    
    TEST_ASSERT_FALSE(state.isEditMode);
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 0.1f, MockStoragePolicy::store["limit_time"]);
}

#ifdef ARDUINO
void setup() {
    delay(2000);
    UNITY_BEGIN();
    RUN_TEST(test_applogic_ble_connect);
    RUN_TEST(test_applogic_button_power_off);
    RUN_TEST(test_applogic_navigation_scroll);
    RUN_TEST(test_applogic_edit_mode);
    UNITY_END();
}
void loop() {}
#else
int main(int argc, char **argv) {
    UNITY_BEGIN();
    RUN_TEST(test_applogic_ble_connect);
    RUN_TEST(test_applogic_button_power_off);
    RUN_TEST(test_applogic_navigation_scroll);
    RUN_TEST(test_applogic_edit_mode);
    UNITY_END();
    return 0;
}
#endif

