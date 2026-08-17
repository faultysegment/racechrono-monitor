#pragma once
#include <map>
#include <string>

class MockStoragePolicy {
public:
    static std::map<std::string, float> store;
    static std::map<std::string, int> storeInt;
    
    static void reset() {
        store.clear();
        storeInt.clear();
    }
    
    static void init() {}
    
    static float getFloat(const char* key, float defaultValue) {
        if (store.find(key) != store.end()) {
            return store[key];
        }
        return defaultValue;
    }
    
    static void putFloat(const char* key, float value) {
        store[key] = value;
    }

    static int getInt(const char* key, int defaultValue) {
        if (storeInt.find(key) != storeInt.end()) {
            return storeInt[key];
        }
        return defaultValue;
    }

    static void putInt(const char* key, int value) {
        storeInt[key] = value;
    }
};

#ifdef PIO_UNIT_TESTING
std::map<std::string, float> MockStoragePolicy::store;
std::map<std::string, int> MockStoragePolicy::storeInt;
#endif
