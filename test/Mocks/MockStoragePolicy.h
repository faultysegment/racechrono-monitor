#pragma once
#include <map>
#include <string>

class MockStoragePolicy {
public:
    static std::map<std::string, float> store;
    
    static void reset() {
        store.clear();
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
};

#ifdef PIO_UNIT_TESTING
std::map<std::string, float> MockStoragePolicy::store;
#endif
