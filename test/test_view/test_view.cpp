#include <unity.h>
#include <lvgl.h>
#include "../../src/AppState.h"
#include "View.h"
#include "../../src/Device_Mock/Policies/MockDisplayPolicy.h"
#include "../../src/Device_Mock/Policies/MockHWPolicy.h"
#include "../../src/Device_Mock/Policies/MockViewPolicy.h"
#ifdef ARDUINO
#include <Arduino.h>
#endif

AppState state;
MockHWPolicy mockHw;
View<MockDisplayPolicy, MockHWPolicy> view(state, mockHw);

MockViewPolicy<MockDisplayPolicy> viewPolicy(state);

void setUp(void) {
    state.reset();
    view.getDisplay().reset();
    mockHw.reset();

    state.clearMonitorConfigs();
    state.clearScreenConfigs();
    state.addScreenConfig(ScreenConfig{ScreenType::SINGLE, ScreenSlotConfig{0, 0xF800, 0x07E0, 0x001F, 0x001F}, ScreenSlotConfig{}});
    state.addScreenConfig(ScreenConfig{ScreenType::SINGLE, ScreenSlotConfig{1, 0x07E0, 0xF800, 0x001F, 0x001F}, ScreenSlotConfig{}});
    state.addScreenConfig(ScreenConfig{ScreenType::DUAL, ScreenSlotConfig{0, 0xF800, 0x07E0, 0x001F, 0x001F}, ScreenSlotConfig{1, 0x07E0, 0xF800, 0x001F, 0x001F}});

    viewPolicy.setupScreens(view, state);
}

void tearDown(void) {}

void test_lvgl_init_state(void) {
    TEST_ASSERT_TRUE(lv_is_initialized());
}

class TestScreen : public IScreen {
public:
    bool initCalled = false;
    bool showCalled = false;
    bool updateCalled = false;
    lv_obj_t* root = nullptr;

    void init() override {
        initCalled = true;
        root = lv_obj_create(NULL);
    }
    void show() override { showCalled = true; }
    void hide() override {}
    void update(const AppState& s) override { updateCalled = true; }
    lv_obj_t* getRoot() const override { return root; }
    void destroy() override {
        if (root) { lv_obj_del(root); root = nullptr; }
    }
};

void test_view_init_and_screen_loading(void) {
    View<MockDisplayPolicy, MockHWPolicy> localView(state, mockHw);
    localView.init();
    TEST_ASSERT_TRUE(lv_is_initialized());

    TestScreen testScreen;
    localView.addConnectedScreen(&testScreen);
    state.isConnected = true;
    state.isConfigured = true;
    state.currentScreenIndex = 0;

    localView.processEvent(Event{EventType::UI_UPDATE, 0, 0, 0});
    TEST_ASSERT_TRUE(testScreen.initCalled);
    TEST_ASSERT_TRUE(testScreen.updateCalled);
    TEST_ASSERT_EQUAL_PTR(testScreen.getRoot(), lv_scr_act());
}

void test_view_show_connected(void) {
    view.processEvent(Event{EventType::UI_SHOW_CONNECTED, 0, 0, 0});
    TEST_ASSERT_EQUAL(TFT_BLACK, view.getDisplay().lastFillScreenColor);
    TEST_ASSERT_TRUE(view.getDisplay().lastPrint.find("BLE connected!") != std::string::npos);
}

void test_view_show_disconnected(void) {
    view.processEvent(Event{EventType::UI_SHOW_DISCONNECTED, 0, 0, 0});
    TEST_ASSERT_EQUAL(TFT_BLACK, view.getDisplay().lastFillScreenColor);
    TEST_ASSERT_TRUE(view.getDisplay().lastPrint.find("Disconnected") != std::string::npos);
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
    
    // Test rectangular monitor0 (Time) - index 1 (circ0 is index 0)
    state.currentScreenIndex = 1;
    view.getDisplay().reset();
    view.processEvent(Event{EventType::UI_UPDATE, 0, 0, 0});
    TEST_ASSERT_EQUAL_STRING("TIME", lv_label_get_text(viewPolicy.singleScreens[0].getTitleLabel()));
    TEST_ASSERT_EQUAL_STRING("+5.00", lv_label_get_text(viewPolicy.singleScreens[0].getValueLabel()));
    TEST_ASSERT_EQUAL(500, lv_bar_get_value(viewPolicy.singleScreens[0].getBar()));

    // Test rectangular monitor1 (Speed) - index 3 (circ1 is index 2)
    state.currentScreenIndex = 3;
    view.getDisplay().reset();
    view.processEvent(Event{EventType::UI_UPDATE, 0, 0, 0});
    TEST_ASSERT_EQUAL_STRING("SPEED", lv_label_get_text(viewPolicy.singleScreens[1].getTitleLabel()));
    TEST_ASSERT_EQUAL_STRING("+2.0", lv_label_get_text(viewPolicy.singleScreens[1].getValueLabel()));
    TEST_ASSERT_EQUAL(400, lv_bar_get_value(viewPolicy.singleScreens[1].getBar()));
}

void test_lvgl_monitor_screen_widgets(void) {
    if (!lv_is_initialized()) lv_init();
    MonitorScreen<> screen(ScreenSlotConfig{0, 0xF800, 0x07E0, 0x001F, 0x001F});
    screen.init();

    AppState s;
    s.isConnected = true;
    s.isConfigured = true;
    s.timeLimit = 10.0f;
    s.addMonitor("TIME", 1.0f, "TIME", false, 2, &s.timeLimit);
    s.setMonitorValue(0, 5); // +5.00 -> 50%

    screen.update(s);
    TEST_ASSERT_EQUAL_STRING("TIME", lv_label_get_text(screen.getTitleLabel()));
    TEST_ASSERT_EQUAL_STRING("+5.00", lv_label_get_text(screen.getValueLabel()));
    TEST_ASSERT_EQUAL(500, lv_bar_get_value(screen.getBar()));

    // Test negative formatting and gauge mapping
    s.setMonitorValue(0, -3);
    screen.update(s);
    TEST_ASSERT_EQUAL_STRING("-3.00", lv_label_get_text(screen.getValueLabel()));
    TEST_ASSERT_EQUAL(300, lv_bar_get_value(screen.getBar()));

    // Test zero limit guard
    s.timeLimit = 0.0f;
    screen.update(s);
    TEST_ASSERT_NOT_NULL(screen.getValueLabel());

    // Test exception state
    s.monitors[0].hasException = true;
    screen.update(s);
    TEST_ASSERT_EQUAL_STRING("ERR", lv_label_get_text(screen.getValueLabel()));
    TEST_ASSERT_EQUAL(0, lv_bar_get_value(screen.getBar()));
}

void test_lvgl_dual_monitor_screen_widgets(void) {
    if (!lv_is_initialized()) lv_init();
    DualMonitorScreen<> screen(ScreenSlotConfig{0, 0xF800, 0x07E0, 0x001F, 0x001F},
                               ScreenSlotConfig{1, 0x07E0, 0xF800, 0x001F, 0x001F});
    screen.init();

    AppState s;
    s.isConnected = true;
    s.isConfigured = true;
    s.timeLimit = 10.0f;
    s.speedLimit = 5.0f;
    s.addMonitor("TIME", 1.0f, "TIME", false, 2, &s.timeLimit);
    s.addMonitor("SPEED", 1.0f, "SPEED", true, 1, &s.speedLimit);
    s.setMonitorValue(0, 5); // 50%
    s.setMonitorValue(1, 2); // 40%

    screen.update(s);
    TEST_ASSERT_EQUAL_STRING("+5.00", lv_label_get_text(screen.getTopValueLabel()));
    TEST_ASSERT_EQUAL_STRING("+2.0", lv_label_get_text(screen.getBtmValueLabel()));
    TEST_ASSERT_EQUAL(500, lv_bar_get_value(screen.getTopBar()));
    TEST_ASSERT_EQUAL(400, lv_bar_get_value(screen.getBtmBar()));
}

void test_mock_display_hud_mode(void) {
    MockDisplayPolicy display;
    TEST_ASSERT_FALSE(display.isHud);
    display.setHudMode(true);
    TEST_ASSERT_TRUE(display.isHud);
    display.setHudMode(false);
    TEST_ASSERT_FALSE(display.isHud);
}

void test_view_global_hud_mode(void) {
    state.reset();
    state.isHud = true;
    state.isConnected = false;
    view.getDisplay().reset();

    view.processEvent(Event{EventType::UI_UPDATE, 0, 0, 0});
    TEST_ASSERT_TRUE(view.getDisplay().isHud);

    state.isHud = false;
    view.processEvent(Event{EventType::UI_UPDATE, 0, 0, 0});
    TEST_ASSERT_FALSE(view.getDisplay().isHud);
}

void test_screen_registration(void) {
    state.reset();
    state.clearScreenConfigs();
    state.addScreenConfig(ScreenConfig{ScreenType::SINGLE, ScreenSlotConfig{0, 0xF800, 0x07E0, 0x001F, 0x001F}, ScreenSlotConfig{}});
    state.addScreenConfig(ScreenConfig{ScreenType::SINGLE, ScreenSlotConfig{1, 0x07E0, 0xF800, 0x001F, 0x001F}, ScreenSlotConfig{}});
    state.addScreenConfig(ScreenConfig{ScreenType::DUAL, ScreenSlotConfig{0, 0xF800, 0x07E0, 0x001F, 0x001F}, ScreenSlotConfig{1, 0x07E0, 0xF800, 0x001F, 0x001F}});

    View<MockDisplayPolicy, MockHWPolicy> mockView(state, mockHw);
    MockViewPolicy<MockDisplayPolicy> policy(state);
    policy.setupScreens(mockView, state);

    TEST_ASSERT_EQUAL(5, mockView.getNumConnectedScreens());
    TEST_ASSERT_EQUAL(1, mockView.getNumDisconnectedScreens());
}

void test_screen_registration_single_monitor(void) {
    state.reset();
    state.clearScreenConfigs();
    state.addScreenConfig(ScreenConfig{ScreenType::SINGLE, ScreenSlotConfig{0, 0xF800, 0x07E0, 0x001F, 0x001F}, ScreenSlotConfig{}});

    View<MockDisplayPolicy, MockHWPolicy> mockView(state, mockHw);
    MockViewPolicy<MockDisplayPolicy> policy(state);
    policy.setupScreens(mockView, state);

    TEST_ASSERT_EQUAL(2, mockView.getNumConnectedScreens()); // circ0 + rect0
    TEST_ASSERT_EQUAL(1, mockView.getNumDisconnectedScreens());
}

void test_view_custom_screen_composition(void) {
    state.reset();
    state.clearScreenConfigs();
    state.addScreenConfig(ScreenConfig{ScreenType::DUAL, ScreenSlotConfig{1, 0x07E0, 0xF800, 0x001F, 0x001F}, ScreenSlotConfig{0, 0xF800, 0x07E0, 0x001F, 0x001F}}); // top: SPEED (idx 1), btm: TIME (idx 0)
    state.addScreenConfig(ScreenConfig{ScreenType::SINGLE, ScreenSlotConfig{1, 0x07E0, 0xF800, 0x001F, 0x001F}, ScreenSlotConfig{}}); // single: SPEED (idx 1)

    View<MockDisplayPolicy, MockHWPolicy> mockView(state, mockHw);
    MockViewPolicy<MockDisplayPolicy> policy(state);
    policy.setupScreens(mockView, state);

    TEST_ASSERT_EQUAL(3, mockView.getNumConnectedScreens()); // dual + circ1 + rect1
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
    view.getDisplay().reset();
    view.processEvent(Event{EventType::UI_UPDATE, 0, 0, 0});

    TEST_ASSERT_TRUE(view.getDisplay().lastPrint.find("TIME") != std::string::npos);
    TEST_ASSERT_TRUE(view.getDisplay().lastPrint.find("+5.00") != std::string::npos);
    TEST_ASSERT_EQUAL(TFT_BLUE, view.getDisplay().lastTextColor); // Value text is always blue
    // Radial bar draws outer circle (filledColor) and inner circle (0x0000)
    TEST_ASSERT_TRUE(view.getDisplay().lastCircles.size() >= 2);
    if (view.getDisplay().lastCircles.size() >= 2) {
        TEST_ASSERT_EQUAL(TFT_RED, view.getDisplay().lastCircles[0].color); // Time positive is bad -> Red
        TEST_ASSERT_EQUAL(TFT_BLACK, view.getDisplay().lastCircles[1].color); // Inner clear circle
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
    view.getDisplay().reset();
    view.processEvent(Event{EventType::UI_UPDATE, 0, 0, 0});

    TEST_ASSERT_TRUE(view.getDisplay().lastCircles.size() >= 2);
    if (view.getDisplay().lastCircles.size() >= 2) {
        int radiusOut = view.getDisplay().lastCircles[0].r;
        int rIn = view.getDisplay().lastCircles[1].r;
        int maxThickness = std::max(12, (int)std::round((float)radiusOut * 0.15f));
        int minThickness = 4;
        int expectedRin = radiusOut - (minThickness + (int)std::round((float)(maxThickness - minThickness) * 0.10f));
        TEST_ASSERT_EQUAL(expectedRin, rIn);
    }
}

void test_view_configuring_screen(void) {
    state.reset();
    state.isConnected = false;
    mockHw.currentMillis = 5000;
    state.lastHeartbeatMillis = 5000;
    view.getDisplay().reset();

    view.processEvent(Event{EventType::UI_UPDATE, 0, 0, 0});
    TEST_ASSERT_TRUE(view.getDisplay().lastPrint.find("CONFIG MODE") != std::string::npos);
}

#ifdef ARDUINO
void setup() {
    delay(2000);
    UNITY_BEGIN();
    RUN_TEST(test_lvgl_init_state);
    RUN_TEST(test_view_init_and_screen_loading);
    RUN_TEST(test_lvgl_monitor_screen_widgets);
    RUN_TEST(test_lvgl_dual_monitor_screen_widgets);
    RUN_TEST(test_view_show_connected);
    RUN_TEST(test_view_show_disconnected);
    RUN_TEST(test_view_update_bars);
    RUN_TEST(test_mock_display_hud_mode);
    RUN_TEST(test_view_global_hud_mode);
    RUN_TEST(test_screen_registration);
    RUN_TEST(test_screen_registration_single_monitor);
    RUN_TEST(test_view_custom_screen_composition);
    RUN_TEST(test_circular_monitor_screen_radial_bar);
    RUN_TEST(test_circular_monitor_screen_radial_bar_min_10_percent);
    RUN_TEST(test_view_configuring_screen);
    UNITY_END();
}
void loop() {}
#else
int main(int argc, char **argv) {
    UNITY_BEGIN();
    RUN_TEST(test_lvgl_init_state);
    RUN_TEST(test_view_init_and_screen_loading);
    RUN_TEST(test_lvgl_monitor_screen_widgets);
    RUN_TEST(test_lvgl_dual_monitor_screen_widgets);
    RUN_TEST(test_view_show_connected);
    RUN_TEST(test_view_show_disconnected);
    RUN_TEST(test_view_update_bars);
    RUN_TEST(test_mock_display_hud_mode);
    RUN_TEST(test_view_global_hud_mode);
    RUN_TEST(test_screen_registration);
    RUN_TEST(test_screen_registration_single_monitor);
    RUN_TEST(test_view_custom_screen_composition);
    RUN_TEST(test_circular_monitor_screen_radial_bar);
    RUN_TEST(test_circular_monitor_screen_radial_bar_min_10_percent);
    RUN_TEST(test_view_configuring_screen);
    UNITY_END();
    return 0;
}
#endif
