#include <unity.h>
#include "../../src/AppState.h"
#include "View.h"
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
    TEST_ASSERT_EQUAL(TFT_BLACK, MockDisplayPolicy::lastFillScreenColor);
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
    state.currentScreenIndex = 2;
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
    state.currentScreenIndex = 3;
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

void test_view_global_hud_mode(void) {
    state.reset();
    state.isHud = true;
    state.isConnected = false;
    MockDisplayPolicy::reset();

    view.processEvent(Event{EventType::UI_UPDATE, 0, 0, 0});
    TEST_ASSERT_TRUE(MockDisplayPolicy::isHud);

    state.isHud = false;
    view.processEvent(Event{EventType::UI_UPDATE, 0, 0, 0});
    TEST_ASSERT_FALSE(MockDisplayPolicy::isHud);
}

void test_screen_registration(void) {
    state.reset();
    View<MockDisplayPolicy, MockHWPolicy> mockView(state);
    NativeViewPolicy<MockDisplayPolicy> policy(state);
    policy.setupScreens(mockView);

    TEST_ASSERT_EQUAL(5, mockView.getNumConnectedScreens());
    TEST_ASSERT_EQUAL(1, mockView.getNumDisconnectedScreens());
}

void test_circular_monitor_screen_radial_bar(void) {
    state.reset();
    state.isConnected = true;
    state.isConfigured = true;
    state.speedLimit = 5.0f;
    state.timeLimit = 10.0f;
    state.addMonitor("M1", 1.0f, "TIME", false, 2, &state.timeLimit);
    state.setMonitorValue(0, 5); // 5 / 10 = 50%

    // Screen 0 in NativeViewPolicy is circMonitor0
    state.currentScreenIndex = 0;
    MockDisplayPolicy::reset();
    view.processEvent(Event{EventType::UI_UPDATE, 0, 0, 0});

    TEST_ASSERT_TRUE(MockDisplayPolicy::lastPrint.find("TIME") != std::string::npos);
    TEST_ASSERT_TRUE(MockDisplayPolicy::lastPrint.find("+5.00") != std::string::npos);
    TEST_ASSERT_EQUAL(TFT_BLUE, MockDisplayPolicy::lastTextColor); // Value text is always blue
    // Radial bar draws outer circle (filledColor) and inner circle (0x0000)
    TEST_ASSERT_TRUE(MockDisplayPolicy::lastCircles.size() >= 2);
    if (MockDisplayPolicy::lastCircles.size() >= 2) {
        TEST_ASSERT_EQUAL(TFT_RED, MockDisplayPolicy::lastCircles[0].color); // Time positive is bad -> Red
        TEST_ASSERT_EQUAL(TFT_BLACK, MockDisplayPolicy::lastCircles[1].color); // Inner clear circle
    }
}

void test_circular_monitor_screen_radial_bar_min_10_percent(void) {
    state.reset();
    state.isConnected = true;
    state.isConfigured = true;
    state.speedLimit = 5.0f;
    state.timeLimit = 100.0f;
    state.addMonitor("M1", 1.0f, "TIME", false, 2, &state.timeLimit);
    state.setMonitorValue(0, 1); // 1 / 100 = 1% -> clamped to 10% minimum

    // Screen 0 in NativeViewPolicy is circMonitor0
    state.currentScreenIndex = 0;
    MockDisplayPolicy::reset();
    view.processEvent(Event{EventType::UI_UPDATE, 0, 0, 0});

    TEST_ASSERT_TRUE(MockDisplayPolicy::lastCircles.size() >= 2);
    if (MockDisplayPolicy::lastCircles.size() >= 2) {
        int radiusOut = MockDisplayPolicy::lastCircles[0].r;
        int rIn = MockDisplayPolicy::lastCircles[1].r;
        int expectedRin = (int)std::round((float)radiusOut * 0.90f);
        TEST_ASSERT_EQUAL(expectedRin, rIn);
    }
}

#ifdef ARDUINO
void setup() {
    delay(2000);
    UNITY_BEGIN();
    RUN_TEST(test_view_show_connected);
    RUN_TEST(test_view_show_disconnected);
    RUN_TEST(test_view_update_bars);
    RUN_TEST(test_mock_display_hud_mode);
    RUN_TEST(test_view_global_hud_mode);
    RUN_TEST(test_screen_registration);
    RUN_TEST(test_circular_monitor_screen_radial_bar);
    RUN_TEST(test_circular_monitor_screen_radial_bar_min_10_percent);
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
    RUN_TEST(test_view_global_hud_mode);
    RUN_TEST(test_screen_registration);
    RUN_TEST(test_circular_monitor_screen_radial_bar);
    RUN_TEST(test_circular_monitor_screen_radial_bar_min_10_percent);
    UNITY_END();
    return 0;
}
#endif
