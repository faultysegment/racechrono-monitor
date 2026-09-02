#pragma once
#include <map>
#include <string>

class MockStoragePolicy {
public:
    std::map<std::string, float> store;
    std::map<std::string, int> storeInt;
    bool cardPresent = true;
    std::string configFileContent = "";
    std::string lastWrittenFileContent = "";
    
    void reset() {
        store.clear();
        storeInt.clear();
        cardPresent = true;
        configFileContent = "";
        lastWrittenFileContent = "";
    }
    
    void init() {}
 
    bool isCardPresent() {
        return cardPresent;
    }

    std::string readConfigFile(const char* filename = "/config.json") {
        return configFileContent;
    }

    bool writeConfigFile(const char* filename, const char* content) {
        lastWrittenFileContent = content ? content : "";
        configFileContent = lastWrittenFileContent;
        return true;
    }
    
    float getFloat(const char* key, float defaultValue) {
        if (store.find(key) != store.end()) {
            return store[key];
        }
        return defaultValue;
    }
    
    void putFloat(const char* key, float value) {
        store[key] = value;
    }

    int getInt(const char* key, int defaultValue) {
        if (storeInt.find(key) != storeInt.end()) {
            return storeInt[key];
        }
        return defaultValue;
    }

    void putInt(const char* key, int value) {
        storeInt[key] = value;
    }
};
