#include <unity.h>
#include "Model.h"
#ifdef ARDUINO
#include <Arduino.h>
#endif

Model model;

void setUp(void) {
    model.reset();
}

void tearDown(void) {
}

void test_model_initialization(void) {
    TEST_ASSERT_EQUAL(0, model.nextMonitorId);
    TEST_ASSERT_FALSE(model.isConnected);
    TEST_ASSERT_FALSE(model.isConfigured);
    TEST_ASSERT_FALSE(model.isConfiguring);
    TEST_ASSERT_EQUAL(0, model.currentScreenIndex);
    TEST_ASSERT_EQUAL(0, model.disconnectedScreenIndex);
    TEST_ASSERT_FALSE(model.isEditMode);
}

void test_add_monitor(void) {
    bool added = model.addMonitor("TestMonitor", 1.5f, "TEST", true, 1, nullptr);
    TEST_ASSERT_TRUE(added);
    TEST_ASSERT_EQUAL(1, model.nextMonitorId);
    TEST_ASSERT_EQUAL_STRING("TestMonitor", model.monitors[0].name);
    TEST_ASSERT_EQUAL_STRING("TEST", model.monitors[0].title);
    TEST_ASSERT_TRUE(model.monitors[0].positiveIsGood);
    TEST_ASSERT_EQUAL(1, model.monitors[0].decimals);
    TEST_ASSERT_EQUAL_FLOAT(1.5f, model.monitors[0].multiplier);
    TEST_ASSERT_EQUAL(Model::INVALID_VALUE, model.monitors[0].value);
}

void test_add_max_monitors(void) {
    for (int i = 0; i < Model::MAX_MONITORS; i++) {
        TEST_ASSERT_TRUE(model.addMonitor("M", 1.0f, "T", false, 2, nullptr));
    }
    TEST_ASSERT_FALSE(model.addMonitor("Overflow", 1.0f, "T", false, 2, nullptr));
    TEST_ASSERT_EQUAL(Model::MAX_MONITORS, model.nextMonitorId);
}

void test_set_monitor_value(void) {
    model.addMonitor("M1", 1.0f, "T", false, 2, nullptr);
    model.setMonitorValue(0, 12345);
    TEST_ASSERT_EQUAL(12345, model.monitors[0].value);
}

#ifdef ARDUINO
void setup() {
    delay(2000);
    UNITY_BEGIN();
    RUN_TEST(test_model_initialization);
    RUN_TEST(test_add_monitor);
    RUN_TEST(test_add_max_monitors);
    RUN_TEST(test_set_monitor_value);
    UNITY_END();
}
void loop() {}
#else
int main(int argc, char **argv) {
    UNITY_BEGIN();
    RUN_TEST(test_model_initialization);
    RUN_TEST(test_add_monitor);
    RUN_TEST(test_add_max_monitors);
    RUN_TEST(test_set_monitor_value);
    UNITY_END();
    return 0;
}
#endif
