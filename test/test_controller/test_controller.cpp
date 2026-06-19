#include <unity.h>
#include "Model.h"
#include "View.h"
#include "Controller.h"
#include "../Mocks/MockDisplayPolicy.h"
#include "../Mocks/MockHWPolicy.h"
#include "../Mocks/MockBLEPolicy.h"
#include "../Mocks/MockStoragePolicy.h"
#ifdef ARDUINO
#include <Arduino.h>
#endif

Model model;
View<MockDisplayPolicy, MockHWPolicy> view(model);
using TestController = Controller<Model, View<MockDisplayPolicy, MockHWPolicy>, MockBLEPolicy, MockHWPolicy, MockStoragePolicy>;

TestController controller(model, view);

void setUp(void) {
    model.reset();
    MockDisplayPolicy::reset();
    MockHWPolicy::reset();
    MockStoragePolicy::reset();
    MockBLEPolicy::reset();
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
    MockHWPolicy::buttonState = 0; // LOW (pressed)
    MockHWPolicy::currentMillis = 0;
    
    controller.loop(); // Detects press
    
    MockHWPolicy::currentMillis = 3500; // 3.5s later
    controller.loop(); // Triggers power off
    
    TEST_ASSERT_TRUE(MockHWPolicy::sleepCalled);
    TEST_ASSERT_TRUE(MockDisplayPolicy::lastPrint.find("Powering off...") != std::string::npos);
}

void test_controller_encoder_scroll(void) {
    controller.setup();
    MockBLEPolicy::simulateConnect();
    model.isConfigured = true; // And configured
    model.currentScreenIndex = 0;

    // Simulate encoder turning right (+1)
    MockHWPolicy::encoderDelta = 1;
    controller.loop();
    TEST_ASSERT_EQUAL(1, model.currentScreenIndex);

    // Simulate encoder turning left (-1)
    MockHWPolicy::encoderDelta = -1;
    controller.loop();
    TEST_ASSERT_EQUAL(0, model.currentScreenIndex);

    // Turning left again should wrap around to NUM_SCREENS - 1 (which is 1)
    MockHWPolicy::encoderDelta = -1;
    controller.loop();
    TEST_ASSERT_EQUAL(1, model.currentScreenIndex);
}

void test_controller_edit_mode(void) {
    controller.setup();
    MockBLEPolicy::simulateConnect();
    model.isConfigured = true;
    model.currentScreenIndex = 0; // Time Screen
    model.timeLimit = 0.1f;
    model.isEditMode = false;

    // Simulate encoder button short press
    MockHWPolicy::encoderKeyState = 0; // Pressed
    MockHWPolicy::currentMillis = 1000;
    controller.loop();
    
    MockHWPolicy::encoderKeyState = 1; // Released
    MockHWPolicy::currentMillis = 1200; // 200ms duration
    controller.loop();
    
    TEST_ASSERT_TRUE(model.isEditMode);

    // Turn encoder right
    MockHWPolicy::encoderDelta = 1;
    controller.loop();
    
    // limit should increase by 0.1
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 0.2f, model.timeLimit);
    
    // Turn encoder left
    MockHWPolicy::encoderDelta = -1;
    controller.loop();
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 0.1f, model.timeLimit);

    // Simulate encoder button short press again to save
    MockHWPolicy::encoderKeyState = 0; // Pressed
    MockHWPolicy::currentMillis = 2000;
    controller.loop();
    
    MockHWPolicy::encoderKeyState = 1; // Released
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
    RUN_TEST(test_controller_encoder_scroll);
    RUN_TEST(test_controller_edit_mode);
    UNITY_END();
}
void loop() {}
#else
int main(int argc, char **argv) {
    UNITY_BEGIN();
    RUN_TEST(test_controller_ble_connect);
    RUN_TEST(test_controller_button_power_off);
    RUN_TEST(test_controller_encoder_scroll);
    RUN_TEST(test_controller_edit_mode);
    UNITY_END();
    return 0;
}
#endif
