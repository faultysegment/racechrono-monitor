#pragma once
#include <string>
#include <map>

class RealStoragePolicy {
    static std::map<std::string, float> data;
public:
    static void init() {}
    static void putFloat(const char* key, float val) {
        data[key] = val;
    }
    static float getFloat(const char* key, float defaultValue) {
        if (data.find(key) != data.end()) {
            return data[key];
        }
        return defaultValue;
    }
};

std::map<std::string, float> RealStoragePolicy::data;
