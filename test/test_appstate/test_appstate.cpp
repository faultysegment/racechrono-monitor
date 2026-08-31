#include <unity.h>
#include "../../src/AppState.h"
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
    strncpy(cfg.name, "Delta time", sizeof(cfg.name));
    strncpy(cfg.title, "TIME", sizeof(cfg.title));
    strncpy(cfg.formula, "channel(device(lap), delta_lap_time)*100.0", sizeof(cfg.formula));
    cfg.multiplier = 0.01f;
    cfg.positiveIsGood = false;
    cfg.decimals = 2;
    cfg.limit = 0.1f;

    s.addMonitorConfig(cfg);
    TEST_ASSERT_EQUAL(1, s.numMonitorConfigs);
    TEST_ASSERT_EQUAL_STRING("TIME", s.monitorConfigs[0].title);
    TEST_ASSERT_FLOAT_WITHIN(0.001f, 0.1f, s.monitorConfigs[0].limit);
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
    UNITY_END();
    return 0;
}
#endif

