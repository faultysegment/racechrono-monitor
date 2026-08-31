#include <unity.h>
#include "../../src/AppState.h"
#include "../../src/ColorUtils.h"
#ifdef ARDUINO
#include <Arduino.h>
#endif

AppState state;

void setUp(void) {
    state.reset();
}

void tearDown(void) {
}

void test_appstate_initialization(void) {
    TEST_ASSERT_EQUAL(0, state.nextMonitorId);
    TEST_ASSERT_FALSE(state.isConnected);
    TEST_ASSERT_FALSE(state.isConfigured);
    TEST_ASSERT_FALSE(state.isConfiguring);
    TEST_ASSERT_EQUAL(0, state.currentScreenIndex);
    TEST_ASSERT_EQUAL(0, state.disconnectedScreenIndex);
    TEST_ASSERT_FALSE(state.isHud);
    TEST_ASSERT_EQUAL(0, state.numMonitorConfigs);
}

void test_add_monitor(void) {
    bool added = state.addMonitor("TestMonitor", 1.5f, "TEST", true, 1, nullptr);
    TEST_ASSERT_TRUE(added);
    TEST_ASSERT_EQUAL(1, state.nextMonitorId);
    TEST_ASSERT_EQUAL_STRING("TestMonitor", state.monitors[0].name);
    TEST_ASSERT_EQUAL_STRING("TEST", state.monitors[0].title);
    TEST_ASSERT_TRUE(state.monitors[0].positiveIsGood);
    TEST_ASSERT_EQUAL(1, state.monitors[0].decimals);
    TEST_ASSERT_EQUAL_FLOAT(1.5f, state.monitors[0].multiplier);
    TEST_ASSERT_EQUAL(AppState::INVALID_VALUE, state.monitors[0].value);
}

void test_add_max_monitors(void) {
    for (int i = 0; i < MAX_MONITORS; i++) {
        TEST_ASSERT_TRUE(state.addMonitor("M", 1.0f, "T", false, 2, nullptr));
    }
    TEST_ASSERT_FALSE(state.addMonitor("Overflow", 1.0f, "T", false, 2, nullptr));
    TEST_ASSERT_EQUAL(MAX_MONITORS, state.nextMonitorId);
}

void test_set_monitor_value(void) {
    state.addMonitor("M1", 1.0f, "T", false, 2, nullptr);
    state.setMonitorValue(0, 12345);
    TEST_ASSERT_EQUAL(12345, state.monitors[0].value);
}

void test_appstate_monitor_configs(void) {
    AppState s;
    s.reset();
    TEST_ASSERT_EQUAL(0, s.numMonitorConfigs);
    TEST_ASSERT_FALSE(s.isHud);

    s.isHud = true;
    TEST_ASSERT_TRUE(s.isHud);

    MonitorConfig cfg;
    strncpy(cfg.id, "lap_delta", sizeof(cfg.id));
    strncpy(cfg.title, "TIME", sizeof(cfg.title));
    strncpy(cfg.formula, "channel(device(lap), delta_lap_time)*100.0", sizeof(cfg.formula));
    cfg.multiplier = 0.01f;
    cfg.positiveIsGood = false;
    cfg.decimals = 2;
    cfg.limit = 0.1f;

    s.addMonitorConfig(cfg);
    TEST_ASSERT_EQUAL(1, s.numMonitorConfigs);
    TEST_ASSERT_EQUAL_STRING("lap_delta", s.monitorConfigs[0].id);
    TEST_ASSERT_EQUAL_STRING("TIME", s.monitorConfigs[0].title);
    TEST_ASSERT_FLOAT_WITHIN(0.001f, 0.1f, s.monitorConfigs[0].limit);
}

void test_appstate_screen_configs(void) {
    AppState s;
    s.reset();
    TEST_ASSERT_EQUAL(0, s.numMonitorConfigs);
    TEST_ASSERT_EQUAL(0, s.numScreenConfigs);

    MonitorConfig m1;
    strncpy(m1.id, "lap_delta", sizeof(m1.id));
    strncpy(m1.title, "TIME", sizeof(m1.title));
    s.addMonitorConfig(m1);

    MonitorConfig m2;
    strncpy(m2.id, "speed_delta", sizeof(m2.id));
    strncpy(m2.title, "SPEED", sizeof(m2.title));
    s.addMonitorConfig(m2);

    TEST_ASSERT_EQUAL(0, s.findMonitorIndexById("lap_delta"));
    TEST_ASSERT_EQUAL(1, s.findMonitorIndexById("speed_delta"));
    TEST_ASSERT_EQUAL(-1, s.findMonitorIndexById("non_existent"));

    ScreenConfig sc1{ScreenType::SINGLE, ScreenSlotConfig{0, 0xF800, 0x07E0, 0x001F, 0x001F}, ScreenSlotConfig{}};
    ScreenConfig sc2{ScreenType::DUAL, ScreenSlotConfig{0, 0xF800, 0x07E0, 0x001F, 0x001F}, ScreenSlotConfig{1, 0x07FF, 0xFD20, 0x001F, 0x001F}};
    s.addScreenConfig(sc1);
    s.addScreenConfig(sc2);

    TEST_ASSERT_EQUAL(2, s.numScreenConfigs);
    TEST_ASSERT_EQUAL(ScreenType::SINGLE, s.screenConfigs[0].type);
    TEST_ASSERT_EQUAL(0, s.screenConfigs[0].primary.monitorIndex);
    TEST_ASSERT_EQUAL(0xF800, s.screenConfigs[0].primary.positiveColor);
    TEST_ASSERT_EQUAL(0x07E0, s.screenConfigs[0].primary.negativeColor);
    TEST_ASSERT_EQUAL(ScreenType::DUAL, s.screenConfigs[1].type);
    TEST_ASSERT_EQUAL(1, s.screenConfigs[1].secondary.monitorIndex);
    TEST_ASSERT_EQUAL(0x07FF, s.screenConfigs[1].secondary.positiveColor);
    TEST_ASSERT_EQUAL(0xFD20, s.screenConfigs[1].secondary.negativeColor);
}

void test_color_utils_hex_parsing(void) {
    // Red #FF0000 -> 0xF800
    TEST_ASSERT_EQUAL_HEX16(0xF800, ColorUtils::parseHexColor565("#FF0000", 0));
    TEST_ASSERT_EQUAL_HEX16(0xF800, ColorUtils::parseHexColor565("FF0000", 0));
    TEST_ASSERT_EQUAL_HEX16(0xF800, ColorUtils::parseHexColor565("0xFF0000", 0));
    TEST_ASSERT_EQUAL_HEX16(0xF800, ColorUtils::parseHexColor565("0xF800", 0));

    // Green #00FF00 -> 0x07E0
    TEST_ASSERT_EQUAL_HEX16(0x07E0, ColorUtils::parseHexColor565("#00FF00", 0));

    // Blue #0000FF -> 0x001F
    TEST_ASSERT_EQUAL_HEX16(0x001F, ColorUtils::parseHexColor565("#0000FF", 0));

    // Cyan #00FFFF -> 0x07FF
    TEST_ASSERT_EQUAL_HEX16(0x07FF, ColorUtils::parseHexColor565("#00FFFF", 0));

    // Fallbacks
    TEST_ASSERT_EQUAL_HEX16(0x1234, ColorUtils::parseHexColor565("", 0x1234));
    TEST_ASSERT_EQUAL_HEX16(0x1234, ColorUtils::parseHexColor565(nullptr, 0x1234));
    TEST_ASSERT_EQUAL_HEX16(0x1234, ColorUtils::parseHexColor565("invalid", 0x1234));
}

#ifdef ARDUINO
void setup() {
    delay(2000);
    UNITY_BEGIN();
    RUN_TEST(test_appstate_initialization);
    RUN_TEST(test_add_monitor);
    RUN_TEST(test_add_max_monitors);
    RUN_TEST(test_set_monitor_value);
    RUN_TEST(test_appstate_monitor_configs);
    RUN_TEST(test_appstate_screen_configs);
    RUN_TEST(test_color_utils_hex_parsing);
    UNITY_END();
}
void loop() {}
#else
int main(int argc, char **argv) {
    UNITY_BEGIN();
    RUN_TEST(test_appstate_initialization);
    RUN_TEST(test_add_monitor);
    RUN_TEST(test_add_max_monitors);
    RUN_TEST(test_set_monitor_value);
    RUN_TEST(test_appstate_monitor_configs);
    RUN_TEST(test_appstate_screen_configs);
    RUN_TEST(test_color_utils_hex_parsing);
    UNITY_END();
    return 0;
}
#endif

