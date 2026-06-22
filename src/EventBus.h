#pragma once

#include <queue>
#include <mutex>
#include <condition_variable>
#include <cstdint>
#include <chrono>

enum class EventType {
    HW_PWR_BTN_PRESS,
    HW_PWR_BTN_LONG_PRESS,
    HW_PWR_BTN_RELEASE,
    HW_ACTION_BTN_PRESS,
    HW_ACTION_BTN_RELEASE,
    HW_NAV_DELTA,
    
    BLE_CONNECTED,
    BLE_DISCONNECTED,
    BLE_CONFIG_MONITOR,
    BLE_MONITOR_UPDATE,

    SYS_BATTERY_UPDATE,
    SYS_TICK,

    UI_UPDATE,
    UI_SHOW_CONNECTED,
    UI_SHOW_DISCONNECTED,
    UI_SHOW_CONFIGURING,
    UI_SHOW_CONFIG_DONE,
    UI_SHOW_CONFIG_FAIL,
    UI_SHOW_POWER_OFF
};

#include <string>

struct Event {
    EventType type;
    int32_t arg1;
    int32_t arg2;
    float f_arg;
    std::string str_arg;
};

class EventBus {
    std::queue<Event> q;
    std::mutex m;
    std::condition_variable cv;
public:
    void push(const Event& e) {
        std::lock_guard<std::mutex> lock(m);
        q.push(e);
        cv.notify_one();
    }
    
    Event pop() {
        std::unique_lock<std::mutex> lock(m);
        cv.wait(lock, [this]{ return !q.empty(); });
        Event e = q.front();
        q.pop();
        return e;
    }

    bool pop_with_timeout(Event& e, int timeout_ms) {
        std::unique_lock<std::mutex> lock(m);
        if (cv.wait_for(lock, std::chrono::milliseconds(timeout_ms), [this]{ return !q.empty(); })) {
            e = q.front();
            q.pop();
            return true;
        }
        return false;
    }

    bool try_pop(Event& e) {
        std::lock_guard<std::mutex> lock(m);
        if (q.empty()) return false;
        e = q.front();
        q.pop();
        return true;
    }
};
