#include <unity.h>
#include "Model.h"
#include "View.h"
#include "../Mocks/MockDisplayPolicy.h"
#include "../Mocks/MockHWPolicy.h"
#ifdef ARDUINO
#include <Arduino.h>
#endif

Model model;
View<MockDisplayPolicy, MockHWPolicy> view(model);

void setUp(void) {
    model.reset();
    MockDisplayPolicy::reset();
    MockHWPolicy::reset();
}

void tearDown(void) {}

void test_view_show_connected(void) {
    view.showConnected();
    TEST_ASSERT_EQUAL(TFT_BLACK, MockDisplayPolicy::lastFillScreenColor);
    TEST_ASSERT_TRUE(MockDisplayPolicy::lastPrint.find("BLE connected!") != std::string::npos);
}

void test_view_show_disconnected(void) {
    view.showDisconnected();
    TEST_ASSERT_EQUAL(TFT_RED, MockDisplayPolicy::lastFillScreenColor);
    TEST_ASSERT_TRUE(MockDisplayPolicy::lastPrint.find("No BLE connection!") != std::string::npos);
}

void test_view_update_bars(void) {
    model.isConnected = true;
    model.speedLimit = 5.0f;
    model.timeLimit = 0.1f;
    model.addMonitor("M1", 1.0f);
    model.addMonitor("M2", 1.0f);
    model.setMonitorValue(0, 10);
    model.setMonitorValue(1, -5);
    
    // Test screen 0 (Time)
    model.currentScreenIndex = 0;
    view.update();
    TEST_ASSERT_TRUE(MockDisplayPolicy::lastPrint.find("TIME") != std::string::npos);

    // Test screen 1 (Speed)
    model.currentScreenIndex = 1;
    view.update();
    TEST_ASSERT_TRUE(MockDisplayPolicy::lastPrint.find("SPEED") != std::string::npos);
}

#ifdef ARDUINO
void setup() {
    delay(2000);
    UNITY_BEGIN();
    RUN_TEST(test_view_show_connected);
    RUN_TEST(test_view_show_disconnected);
    RUN_TEST(test_view_update_bars);
    UNITY_END();
}
void loop() {}
#else
int main(int argc, char **argv) {
    UNITY_BEGIN();
    RUN_TEST(test_view_show_connected);
    RUN_TEST(test_view_show_disconnected);
    RUN_TEST(test_view_update_bars);
    UNITY_END();
    return 0;
}
#endif
