#include <unity.h>
#include "../../src/AppState.h"
#include "View.h"
#include "../../src/Device_Mock/Policies/MockDisplayPolicy.h"
#include "../../src/Device_Mock/Policies/MockHWPolicy.h"
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

void setUp(void) {
    state.reset();
    MockDisplayPolicy::reset();
    MockHWPolicy::reset();

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

void test_view_show_connected(void) {
    view.processEvent(Event{EventType::UI_SHOW_CONNECTED, 0, 0, 0});
    TEST_ASSERT_EQUAL(TFT_BLACK, MockDisplayPolicy::lastFillScreenColor);
    TEST_ASSERT_TRUE(MockDisplayPolicy::lastPrint.find("BLE connected!") != std::string::npos);
}

void test_view_show_disconnected(void) {
    view.processEvent(Event{EventType::UI_SHOW_DISCONNECTED, 0, 0, 0});
    TEST_ASSERT_EQUAL(TFT_RED, MockDisplayPolicy::lastFillScreenColor);
    TEST_ASSERT_TRUE(MockDisplayPolicy::lastPrint.find("No BLE connection!") != std::string::npos);
}

void test_view_update_bars(void) {
    state.isConnected = true;
    state.speedLimit = 5.0f;
    state.timeLimit = 10.0f;
    state.addMonitor("M1", 1.0f, "TIME", false, 2, &state.timeLimit);
    state.addMonitor("M2", 1.0f, "SPEED", true, 1, &state.speedLimit);
    state.setMonitorValue(0, 5); // 5 / 10 = 50%
    state.setMonitorValue(1, 2); // 2 / 5 = 40%
    
    // Test screen 0 (Time)
    state.currentScreenIndex = 0;
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

    // Test screen 1 (Speed)
    state.currentScreenIndex = 1;
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
