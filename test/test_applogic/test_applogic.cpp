#include <unity.h>
#include "../../src/AppState.h"
#include "../../src/View.h"
#include "../../src/AppLogic.h"
#include "../../src/EventBus.h"
#include "../../src/Device_Mock/Policies/MockDisplayPolicy.h"
#include "../../src/Device_Mock/Policies/MockHWPolicy.h"
#include "../../src/Device_Mock/Policies/MockBLEPolicy.h"
#include "../../src/Device_Mock/Policies/MockStoragePolicy.h"
#include "../../src/Device_Native/Policies/NativeViewPolicy.h"
#ifdef ARDUINO
#include <Arduino.h>
#endif

AppState state;
View<MockDisplayPolicy, MockHWPolicy> view(state);

NativeViewPolicy<MockDisplayPolicy> viewPolicy(state);
using TestAppLogic = AppLogic<MockBLEPolicy, MockHWPolicy, MockStoragePolicy>;

EventBus testBus;
TestAppLogic logic(state, testBus);

void flushEvents() {
    Event e;
    while(testBus.try_pop(e)) {
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
    while(testBus.try_pop(e)) {}

    if (view.getNumConnectedScreens() == 0) {
        viewPolicy.setupScreens(view);
    }
}

void tearDown(void) {}

void test_applogic_ble_connect(void) {
    logic.setup();
    testBus.push(Event{EventType::BLE_CONNECTED, 0, 0, 0});
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

