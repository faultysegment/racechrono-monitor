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
        viewPolicy.setupScreens(view, state);
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
    TEST_ASSERT_EQUAL(state.numConnectedScreens - 1, state.currentScreenIndex);
}

void test_applogic_screen_persistence() {
    setUp();
    MockStoragePolicy::reset();
    logic.setup();

    state.numConnectedScreens = 3;
    state.numDisconnectedScreens = 1;
    
    // Simulate BLE Connected
    testBus.push(Event{EventType::BLE_CONNECTED, 0, 0, 0});
    flushEvents();
    TEST_ASSERT_TRUE(state.isConnected);

    // Scroll to screen 1
    MockHWPolicy::navigationDelta = 1;
    logic.pollInput();
    flushEvents();
    TEST_ASSERT_EQUAL(1, state.currentScreenIndex);
    TEST_ASSERT_EQUAL(1, MockStoragePolicy::storeInt["last_screen"]);

    // Disconnect -> should switch to Disconnected screen
    testBus.push(Event{EventType::BLE_DISCONNECTED, 0, 0, 0});
    flushEvents();
    TEST_ASSERT_FALSE(state.isConnected);
    TEST_ASSERT_EQUAL(0, state.disconnectedScreenIndex);

    // Reconnect -> should restore screen 1
    testBus.push(Event{EventType::BLE_CONNECTED, 0, 0, 0});
    flushEvents();
    TEST_ASSERT_TRUE(state.isConnected);
    TEST_ASSERT_EQUAL(1, state.currentScreenIndex);
}

void test_applogic_json_config_custom_monitors(void) {
    setUp();
    MockStoragePolicy::reset();
    MockStoragePolicy::configFileContent = "{\n"
        "  \"isHud\": true,\n"
        "  \"monitors\": [\n"
        "    {\n"
        "      \"id\": \"custom_delta\",\n"
        "      \"title\": \"CDELTA\",\n"
        "      \"formula\": \"channel(device(lap), delta_lap_time)*100.0\",\n"
        "      \"multiplier\": 0.01,\n"
        "      \"positive_is_good\": false,\n"
        "      \"decimals\": 2,\n"
        "      \"limit\": 0.25\n"
        "    }\n"
        "  ]\n"
        "}";

    logic.setup();
    TEST_ASSERT_TRUE(state.isHud);
    TEST_ASSERT_EQUAL(1, state.numMonitorConfigs);
    TEST_ASSERT_EQUAL_STRING("custom_delta", state.monitorConfigs[0].id);
    TEST_ASSERT_EQUAL_STRING("CDELTA", state.monitorConfigs[0].title);
    TEST_ASSERT_FLOAT_WITHIN(0.001f, 0.25f, state.monitorConfigs[0].limit);
    TEST_ASSERT_EQUAL(1, state.numScreenConfigs);
    TEST_ASSERT_EQUAL(ScreenType::SINGLE, state.screenConfigs[0].type);
    TEST_ASSERT_EQUAL(0, state.screenConfigs[0].primaryMonitorIndex);
}

void test_applogic_json_custom_screens(void) {
    setUp();
    MockStoragePolicy::reset();
    MockStoragePolicy::configFileContent = R"json({
        "isHud": false,
        "monitors": [
            {
                "id": "delta_time",
                "title": "TIME",
                "formula": "channel(device(lap), delta_lap_time)*100.0",
                "multiplier": 0.01,
                "positive_is_good": false,
                "decimals": 2,
                "limit": 0.5
            },
            {
                "id": "delta_speed",
                "title": "SPEED",
                "formula": "channel(device(calc), delta_speed)*100",
                "multiplier": 0.036,
                "positive_is_good": true,
                "decimals": 1,
                "limit": 1.0
            }
        ],
        "screens": [
            { "type": "dual", "top": "delta_speed", "bottom": "delta_time" },
            { "type": "single", "monitor": "delta_speed" }
        ]
    })json";

    logic.setup();
    TEST_ASSERT_EQUAL(2, state.numMonitorConfigs);
    TEST_ASSERT_EQUAL(2, state.numScreenConfigs);

    // Screen 0 is dual with top=speed (idx 1) and bottom=time (idx 0)
    TEST_ASSERT_EQUAL(ScreenType::DUAL, state.screenConfigs[0].type);
    TEST_ASSERT_EQUAL(1, state.screenConfigs[0].primaryMonitorIndex);
    TEST_ASSERT_EQUAL(0, state.screenConfigs[0].secondaryMonitorIndex);

    // Screen 1 is single with monitor=speed (idx 1)
    TEST_ASSERT_EQUAL(ScreenType::SINGLE, state.screenConfigs[1].type);
    TEST_ASSERT_EQUAL(1, state.screenConfigs[1].primaryMonitorIndex);
}

void test_applogic_json_config_fallback_defaults(void) {
    setUp();
    MockStoragePolicy::reset();
    MockStoragePolicy::configFileContent = ""; // Empty file / missing

    logic.setup();
    TEST_ASSERT_FALSE(state.isHud);
    TEST_ASSERT_EQUAL(2, state.numMonitorConfigs);
    TEST_ASSERT_EQUAL_STRING("TIME", state.monitorConfigs[0].title);
    TEST_ASSERT_EQUAL_STRING("SPEED", state.monitorConfigs[1].title);
    TEST_ASSERT_EQUAL(3, state.numScreenConfigs);
}

void test_applogic_json_config_auto_create(void) {
    setUp();
    MockStoragePolicy::reset();
    MockStoragePolicy::cardPresent = true;
    MockStoragePolicy::configFileContent = "";

    logic.setup();
    TEST_ASSERT_TRUE(MockStoragePolicy::lastWrittenFileContent.find("\"monitors\"") != std::string::npos);
    TEST_ASSERT_TRUE(MockStoragePolicy::lastWrittenFileContent.find("\"screens\"") != std::string::npos);
    TEST_ASSERT_TRUE(MockStoragePolicy::lastWrittenFileContent.find("\"TIME\"") != std::string::npos);
}

void test_applogic_reconfigure_on_cmd_type_update_all(void) {
    logic.setup();
    MockBLEPolicy::simulateConnect();
    MockBLEPolicy::indicating = true;
    logic.pollLogic();
    flushEvents();

    TEST_ASSERT_TRUE(state.isConfigured);
    size_t initialCount = MockBLEPolicy::sentConfigCommands.size();
    TEST_ASSERT_TRUE(initialCount > 0);

    // Simulate RaceChrono sending CMD_TYPE_UPDATE_ALL (4) upon session continue
    uint8_t cmd[1] = {4};
    MockBLEPolicy::simulateConfigWrite(std::string((char*)cmd, 1));
    flushEvents();

    TEST_ASSERT_TRUE(state.isConfigured);
    TEST_ASSERT_TRUE(MockBLEPolicy::sentConfigCommands.size() > initialCount);
}

void test_applogic_reconfigure_on_indication_toggle(void) {
    logic.setup();
    MockBLEPolicy::simulateConnect();
    MockBLEPolicy::indicating = true;
    logic.pollLogic();
    flushEvents();

    TEST_ASSERT_TRUE(state.isConfigured);
    size_t initialCount = MockBLEPolicy::sentConfigCommands.size();

    // RaceChrono drops indication on session pause/stop
    MockBLEPolicy::indicating = false;
    logic.pollLogic();
    flushEvents();

    // RaceChrono re-enables indication on session resume
    MockBLEPolicy::indicating = true;
    logic.pollLogic();
    flushEvents();

    TEST_ASSERT_TRUE(state.isConfigured);
    TEST_ASSERT_TRUE(MockBLEPolicy::sentConfigCommands.size() > initialCount);
}

#ifdef ARDUINO
void setup() {
    delay(2000);
    UNITY_BEGIN();
    RUN_TEST(test_applogic_ble_connect);
    RUN_TEST(test_applogic_button_power_off);
    RUN_TEST(test_applogic_navigation_scroll);
    RUN_TEST(test_applogic_screen_persistence);
    RUN_TEST(test_applogic_json_config_custom_monitors);
    RUN_TEST(test_applogic_json_custom_screens);
    RUN_TEST(test_applogic_json_config_fallback_defaults);
    RUN_TEST(test_applogic_json_config_auto_create);
    RUN_TEST(test_applogic_reconfigure_on_cmd_type_update_all);
    RUN_TEST(test_applogic_reconfigure_on_indication_toggle);
    UNITY_END();
}
void loop() {}
#else
int main(int argc, char **argv) {
    UNITY_BEGIN();
    RUN_TEST(test_applogic_ble_connect);
    RUN_TEST(test_applogic_button_power_off);
    RUN_TEST(test_applogic_navigation_scroll);
    RUN_TEST(test_applogic_screen_persistence);
    RUN_TEST(test_applogic_json_config_custom_monitors);
    RUN_TEST(test_applogic_json_custom_screens);
    RUN_TEST(test_applogic_json_config_fallback_defaults);
    RUN_TEST(test_applogic_json_config_auto_create);
    RUN_TEST(test_applogic_reconfigure_on_cmd_type_update_all);
    RUN_TEST(test_applogic_reconfigure_on_indication_toggle);
    UNITY_END();
    return 0;
}
#endif
