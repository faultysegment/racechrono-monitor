#pragma once
#include <map>
#include <string>

class MockStoragePolicy {
public:
    static std::map<std::string, float> store;
    static std::map<std::string, int> storeInt;
    static bool cardPresent;
    static std::string configFileContent;
    static std::string lastWrittenFileContent;
    
    static void reset() {
        store.clear();
        storeInt.clear();
        cardPresent = true;
        configFileContent = "";
        lastWrittenFileContent = "";
    }
    
    static void init() {}

    static bool isCardPresent() {
        return cardPresent;
    }

    static std::string readConfigFile(const char* filename = "/config.json") {
        return configFileContent;
    }

    static bool writeConfigFile(const char* filename, const char* content) {
        lastWrittenFileContent = content ? content : "";
        configFileContent = lastWrittenFileContent;
        return true;
    }
    
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
bool MockStoragePolicy::cardPresent = true;
std::string MockStoragePolicy::configFileContent = "";
std::string MockStoragePolicy::lastWrittenFileContent = "";
#endif
