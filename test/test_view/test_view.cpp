#include <unity.h>
#include "../../src/AppState.h"
#include "View.h"
#include "../../src/Screens/HudScreenWrapper.h"
#include "../../src/Device_Mock/Policies/MockDisplayPolicy.h"
#include "../../src/Device_Mock/Policies/MockHWPolicy.h"
#include "../../src/Device_Native/Policies/NativeViewPolicy.h"
#ifdef ARDUINO
#include <Arduino.h>
#endif

AppState state;
View<MockDisplayPolicy, MockHWPolicy> view(state);

NativeViewPolicy<MockDisplayPolicy> viewPolicy(state);

void setUp(void) {
    state.reset();
    MockDisplayPolicy::reset();
    MockHWPolicy::reset();

    if (view.getNumConnectedScreens() == 0) {
        viewPolicy.setupScreens(view);
    }
}

void tearDown(void) {}

void test_view_show_connected(void) {
    view.processEvent(Event{EventType::UI_SHOW_CONNECTED, 0, 0, 0});
    TEST_ASSERT_EQUAL(TFT_BLACK, MockDisplayPolicy::lastFillScreenColor);
    TEST_ASSERT_TRUE(MockDisplayPolicy::lastPrint.find("BLE connected!") != std::string::npos);
}

void test_view_show_disconnected(void) {
    view.processEvent(Event{EventType::UI_SHOW_DISCONNECTED, 0, 0, 0});
    TEST_ASSERT_EQUAL(TFT_RED, MockDisplayPolicy::lastFillScreenColor);
    TEST_ASSERT_TRUE(MockDisplayPolicy::lastPrint.find("Disconnected") != std::string::npos);
}

void test_view_update_bars(void) {
    state.isConnected = true;
    state.isConfigured = true;
    state.speedLimit = 5.0f;
    state.timeLimit = 10.0f;
    state.addMonitor("M1", 1.0f, "TIME", false, 2, &state.timeLimit);
    state.addMonitor("M2", 1.0f, "SPEED", true, 1, &state.speedLimit);
    state.setMonitorValue(0, 5); // 5 / 10 = 50%
    state.setMonitorValue(1, 2); // 2 / 5 = 40%
    
    // Test rectangular monitor0 (Time)
    state.currentScreenIndex = 3;
    MockDisplayPolicy::reset();
    view.processEvent(Event{EventType::UI_UPDATE, 0, 0, 0});
    TEST_ASSERT_TRUE(MockDisplayPolicy::lastPrint.find("TIME") != std::string::npos);
    TEST_ASSERT_TRUE(MockDisplayPolicy::lastPrint.find("+5.00") != std::string::npos);
    TEST_ASSERT_EQUAL(2, MockDisplayPolicy::lastRects.size());
    if (MockDisplayPolicy::lastRects.size() >= 2) {
        TEST_ASSERT_EQUAL(TFT_RED, MockDisplayPolicy::lastRects[0].color); // Time positive is bad
        TEST_ASSERT_EQUAL(160, MockDisplayPolicy::lastRects[0].w); // 50% of 320
        TEST_ASSERT_EQUAL(TFT_DARKGREY, MockDisplayPolicy::lastRects[1].color);
        TEST_ASSERT_EQUAL(160, MockDisplayPolicy::lastRects[1].w);
    }

    // Test rectangular monitor1 (Speed)
    state.currentScreenIndex = 4;
    MockDisplayPolicy::reset();
    view.processEvent(Event{EventType::UI_UPDATE, 0, 0, 0});
    TEST_ASSERT_TRUE(MockDisplayPolicy::lastPrint.find("SPEED") != std::string::npos);
    TEST_ASSERT_TRUE(MockDisplayPolicy::lastPrint.find("+2.0") != std::string::npos);
    TEST_ASSERT_EQUAL(2, MockDisplayPolicy::lastRects.size());
    if (MockDisplayPolicy::lastRects.size() >= 2) {
        TEST_ASSERT_EQUAL(TFT_GREEN, MockDisplayPolicy::lastRects[0].color); // Speed positive is good
        TEST_ASSERT_EQUAL(128, MockDisplayPolicy::lastRects[0].w); // 40% of 320
        TEST_ASSERT_EQUAL(TFT_DARKGREY, MockDisplayPolicy::lastRects[1].color);
        TEST_ASSERT_EQUAL(192, MockDisplayPolicy::lastRects[1].w); // 320 - 128
    }
}

void test_mock_display_hud_mode(void) {
    MockDisplayPolicy display;
    TEST_ASSERT_FALSE(MockDisplayPolicy::isHud);
    display.setHudMode(true);
    TEST_ASSERT_TRUE(MockDisplayPolicy::isHud);
    display.setHudMode(false);
    TEST_ASSERT_FALSE(MockDisplayPolicy::isHud);
}
void test_hud_screen_wrapper(void) {
    MockDisplayPolicy display;
    MockDisplayPolicy::reset();
    MonitorScreen<MockDisplayPolicy> innerMonitor{0};
    HudScreenWrapper<MockDisplayPolicy> hudWrapper(&innerMonitor);

    state.isConnected = true;
    state.speedLimit = 5.0f;
    state.addMonitor("M1", 1.0f, "SPEED", true, 1, &state.speedLimit);
    state.setMonitorValue(0, 10);

    TEST_ASSERT_FALSE(MockDisplayPolicy::isHud);
    hudWrapper.onShow(display, state);
    hudWrapper.onUpdate(display, state);
    TEST_ASSERT_TRUE(MockDisplayPolicy::isHud);
    TEST_ASSERT_TRUE(MockDisplayPolicy::lastPrint.find("SPEED") != std::string::npos);
}

void test_hud_screen_registration(void) {
    state.reset();
    View<MockDisplayPolicy, MockHWPolicy> mockView(state);
    NativeViewPolicy<MockDisplayPolicy> policy(state);
    policy.setupScreens(mockView);

    TEST_ASSERT_EQUAL(12, mockView.getNumConnectedScreens());
    TEST_ASSERT_EQUAL(10, mockView.getNumDisconnectedScreens());
}

#ifdef ARDUINO
void setup() {
    delay(2000);
    UNITY_BEGIN();
    RUN_TEST(test_view_show_connected);
    RUN_TEST(test_view_show_disconnected);
    RUN_TEST(test_view_update_bars);
    RUN_TEST(test_mock_display_hud_mode);
    RUN_TEST(test_hud_screen_wrapper);
    RUN_TEST(test_hud_screen_registration);
    UNITY_END();
}
void loop() {}
#else
int main(int argc, char **argv) {
    UNITY_BEGIN();
    RUN_TEST(test_view_show_connected);
    RUN_TEST(test_view_show_disconnected);
    RUN_TEST(test_view_update_bars);
    RUN_TEST(test_mock_display_hud_mode);
    RUN_TEST(test_hud_screen_wrapper);
    RUN_TEST(test_hud_screen_registration);
    UNITY_END();
    return 0;
}
#endif
