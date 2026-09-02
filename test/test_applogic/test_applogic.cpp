#include <unity.h>
#include "../../src/AppState.h"
#include "../../src/View.h"
#include "../../src/AppLogic.h"
#include "../../src/EventBus.h"
#include "../../src/Device_Mock/Policies/MockDisplayPolicy.h"
#include "../../src/Device_Mock/Policies/MockHWPolicy.h"
#include "../../src/Device_Mock/Policies/MockBLEPolicy.h"
#include "../../src/Device_Mock/Policies/MockStoragePolicy.h"
#include "../../src/Device_Mock/Policies/MockViewPolicy.h"
#ifdef ARDUINO
#include <Arduino.h>
#endif

AppState state;
MockHWPolicy mockHw;
MockStoragePolicy mockStorage;
View<MockDisplayPolicy, MockHWPolicy> view(state, mockHw);

MockViewPolicy<MockDisplayPolicy> viewPolicy(state);
using TestAppLogic = AppLogic<MockBLEPolicy, MockHWPolicy, MockStoragePolicy>;

EventBus testBus;
TestAppLogic logic(state, testBus, mockHw, mockStorage);

void flushEvents() {
    Event e;
    while(testBus.try_pop(e)) {
        view.processEvent(e);
        logic.processEvent(e);
    }
}

void setUp(void) {
    state.reset();
    view.getDisplay().reset();
    mockHw.reset();
    mockStorage.reset();
    logic.getBLE().reset();

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
    TEST_ASSERT_TRUE(view.getDisplay().lastPrint.find("BLE connected!") != std::string::npos);
}

void test_applogic_button_power_off(void) {
    logic.setup();
    mockHw.powerKeyPressed = true;
    mockHw.currentMillis = 0;
    
    logic.pollInput();
    flushEvents();
    
    mockHw.currentMillis = 3500; // 3.5s later
    logic.pollInput();
    flushEvents();
    
    TEST_ASSERT_TRUE(mockHw.sleepCalled);
    TEST_ASSERT_TRUE(view.getDisplay().lastPrint.find("Powering off...") != std::string::npos);
}

void test_applogic_navigation_scroll(void) {
    logic.setup();
    logic.getBLE().simulateConnect();
    state.isConfigured = true;
    state.isConnected = true;
    state.currentScreenIndex = 0;

    mockHw.navigationDelta = 1;
    logic.pollInput();
    flushEvents();
    TEST_ASSERT_EQUAL(1, state.currentScreenIndex);

    mockHw.navigationDelta = -1;
    logic.pollInput();
    flushEvents();
    TEST_ASSERT_EQUAL(0, state.currentScreenIndex);

    mockHw.navigationDelta = -1;
    logic.pollInput();
    flushEvents();
    TEST_ASSERT_EQUAL(state.numConnectedScreens - 1, state.currentScreenIndex);
}

void test_applogic_screen_persistence() {
    setUp();
    mockStorage.reset();
    logic.setup();

    state.numConnectedScreens = 3;
    state.numDisconnectedScreens = 1;
    
    // Simulate BLE Connected
    testBus.push(Event{EventType::BLE_CONNECTED, 0, 0, 0});
    flushEvents();
    TEST_ASSERT_TRUE(state.isConnected);

    // Scroll to screen 1
    mockHw.navigationDelta = 1;
    logic.pollInput();
    flushEvents();
    TEST_ASSERT_EQUAL(1, state.currentScreenIndex);
    TEST_ASSERT_EQUAL(1, mockStorage.storeInt["last_screen"]);

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
    mockStorage.reset();
    mockStorage.configFileContent = "{\n"
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
    TEST_ASSERT_EQUAL(0, state.screenConfigs[0].primary.monitorIndex);
}

void test_applogic_json_custom_screens(void) {
    setUp();
    mockStorage.reset();
    mockStorage.configFileContent = R"json({
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
            {
                "type": "dual",
                "top": {
                    "monitor": "delta_speed",
                    "positive_color": "#00FFFF",
                    "negative_color": "#FFA500"
                },
                "bottom": {
                    "monitor": "delta_time",
                    "positive_color": "#FF0000",
                    "negative_color": "#00FF00"
                }
            },
            {
                "type": "single",
                "monitor": "delta_speed",
                "positive_color": "#00FFFF",
                "negative_color": "#FFA500",
                "title_color": "#0000FF",
                "value_color": "#FFFFFF"
            }
        ]
    })json";

    logic.setup();
    TEST_ASSERT_EQUAL(2, state.numMonitorConfigs);
    TEST_ASSERT_EQUAL(2, state.numScreenConfigs);

    // Screen 0 is dual with top=speed (idx 1, cyan/orange) and bottom=time (idx 0, red/green)
    TEST_ASSERT_EQUAL(ScreenType::DUAL, state.screenConfigs[0].type);
    TEST_ASSERT_EQUAL(1, state.screenConfigs[0].primary.monitorIndex);
    TEST_ASSERT_EQUAL(0x07FF, state.screenConfigs[0].primary.positiveColor); // Cyan
    TEST_ASSERT_EQUAL(0xFD20, state.screenConfigs[0].primary.negativeColor); // Orange
    TEST_ASSERT_EQUAL(0, state.screenConfigs[0].secondary.monitorIndex);
    TEST_ASSERT_EQUAL(0xF800, state.screenConfigs[0].secondary.positiveColor); // Red
    TEST_ASSERT_EQUAL(0x07E0, state.screenConfigs[0].secondary.negativeColor); // Green

    // Screen 1 is single with monitor=speed (idx 1)
    TEST_ASSERT_EQUAL(ScreenType::SINGLE, state.screenConfigs[1].type);
    TEST_ASSERT_EQUAL(1, state.screenConfigs[1].primary.monitorIndex);
    TEST_ASSERT_EQUAL(0x07FF, state.screenConfigs[1].primary.positiveColor);
    TEST_ASSERT_EQUAL(0xFD20, state.screenConfigs[1].primary.negativeColor);
    TEST_ASSERT_EQUAL(0x001F, state.screenConfigs[1].primary.titleColor); // Blue
    TEST_ASSERT_EQUAL(0xFFFF, state.screenConfigs[1].primary.valueColor); // White
}

void test_applogic_json_config_fallback_defaults(void) {
    setUp();
    mockStorage.reset();
    mockStorage.configFileContent = ""; // Empty file / missing

    logic.setup();
    TEST_ASSERT_FALSE(state.isHud);
    TEST_ASSERT_EQUAL(2, state.numMonitorConfigs);
    TEST_ASSERT_EQUAL_STRING("TIME", state.monitorConfigs[0].title);
    TEST_ASSERT_EQUAL_STRING("SPEED", state.monitorConfigs[1].title);
    TEST_ASSERT_EQUAL(3, state.numScreenConfigs);
}

void test_applogic_json_config_auto_create(void) {
    setUp();
    mockStorage.reset();
    mockStorage.cardPresent = true;
    mockStorage.configFileContent = "";

    logic.setup();
    TEST_ASSERT_TRUE(mockStorage.lastWrittenFileContent.find("\"monitors\"") != std::string::npos);
    TEST_ASSERT_TRUE(mockStorage.lastWrittenFileContent.find("\"screens\"") != std::string::npos);
    TEST_ASSERT_TRUE(mockStorage.lastWrittenFileContent.find("\"TIME\"") != std::string::npos);
}

void test_applogic_reconfigure_on_cmd_type_update_all(void) {
    logic.setup();
    logic.getBLE().simulateConnect();
    logic.getBLE().indicating = true;
    logic.pollLogic();
    flushEvents();

    TEST_ASSERT_TRUE(state.isConfigured);
    size_t initialCount = logic.getBLE().sentConfigCommands.size();
    TEST_ASSERT_TRUE(initialCount > 0);

    // Simulate RaceChrono sending CMD_TYPE_UPDATE_ALL (4) upon session continue
    uint8_t cmd[1] = {4};
    logic.getBLE().simulateConfigWrite(std::string((char*)cmd, 1));
    flushEvents();

    TEST_ASSERT_TRUE(state.isConfigured);
    TEST_ASSERT_TRUE(logic.getBLE().sentConfigCommands.size() > initialCount);
}

void test_applogic_reconfigure_on_indication_toggle(void) {
    logic.setup();
    logic.getBLE().simulateConnect();
    logic.getBLE().indicating = true;
    logic.pollLogic();
    flushEvents();

    TEST_ASSERT_TRUE(state.isConfigured);
    size_t initialCount = logic.getBLE().sentConfigCommands.size();

    // RaceChrono drops indication on session pause/stop
    logic.getBLE().indicating = false;
    logic.pollLogic();
    flushEvents();

    // RaceChrono re-enables indication on session resume
    logic.getBLE().indicating = true;
    logic.pollLogic();
    flushEvents();

    TEST_ASSERT_TRUE(state.isConfigured);
    TEST_ASSERT_TRUE(logic.getBLE().sentConfigCommands.size() > initialCount);
}

void test_applogic_json_webui_config(void) {
    setUp();
    mockStorage.reset();
    mockStorage.configFileContent = R"json({
        "isHud": false,
        "webui": {
            "enabled": true,
            "ssid": "Test-AP",
            "password": "secretpassword"
        },
        "monitors": [
            { "id": "m1", "title": "TIME", "formula": "1", "multiplier": 1.0, "decimals": 2, "limit": 0.5 }
        ]
    })json";

    logic.setup();
    TEST_ASSERT_TRUE(state.webuiConfig.enabled);
    TEST_ASSERT_EQUAL_STRING("Test-AP", state.webuiConfig.ssid);
    TEST_ASSERT_EQUAL_STRING("secretpassword", state.webuiConfig.password);
}

void test_applogic_config_mode_events(void) {
    setUp();
    mockHw.reset();
    logic.setup();
    TEST_ASSERT_FALSE(state.isConfiguring(mockHw.millis()));

    // Enter config mode
    mockHw.currentMillis = 5000;
    logic.processEvent(Event{EventType::EVENT_CONFIG_MODE_ENTER, 0, 0, 0});
    TEST_ASSERT_EQUAL_UINT32(5000, state.lastHeartbeatMillis);
    TEST_ASSERT_TRUE(state.isConfiguring(mockHw.millis()));

    // Exit config mode
    logic.processEvent(Event{EventType::EVENT_CONFIG_MODE_EXIT, 0, 0, 0});
    TEST_ASSERT_EQUAL_UINT32(0, state.lastHeartbeatMillis);
    TEST_ASSERT_FALSE(state.isConfiguring(mockHw.millis()));

    // Enter config mode again
    mockHw.currentMillis = 10000;
    logic.processEvent(Event{EventType::EVENT_CONFIG_MODE_ENTER, 0, 0, 0});
    TEST_ASSERT_TRUE(state.isConfiguring(mockHw.millis()));

    // Config reload exits config mode
    logic.processEvent(Event{EventType::EVENT_CONFIG_RELOAD, 0, 0, 0});
    TEST_ASSERT_EQUAL_UINT32(0, state.lastHeartbeatMillis);
    TEST_ASSERT_FALSE(state.isConfiguring(mockHw.millis()));

    // Test automatic timeout in pollLogic
    mockHw.currentMillis = 20000;
    logic.processEvent(Event{EventType::EVENT_CONFIG_MODE_ENTER, 0, 0, 0});
    TEST_ASSERT_TRUE(state.isConfiguring(mockHw.millis()));

    // Advance time by 30s -> still configuring
    mockHw.currentMillis += 30000;
    logic.pollLogic();
    TEST_ASSERT_TRUE(state.isConfiguring(mockHw.millis()));

    // Advance time past 60s -> pollLogic expires config mode
    mockHw.currentMillis += 30001;
    logic.pollLogic();
    TEST_ASSERT_EQUAL_UINT32(0, state.lastHeartbeatMillis);
    TEST_ASSERT_FALSE(state.isConfiguring(mockHw.millis()));
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
    RUN_TEST(test_applogic_json_webui_config);
    RUN_TEST(test_applogic_config_mode_events);
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
    RUN_TEST(test_applogic_json_webui_config);
    RUN_TEST(test_applogic_config_mode_events);
    UNITY_END();
    return 0;
}
#endif
