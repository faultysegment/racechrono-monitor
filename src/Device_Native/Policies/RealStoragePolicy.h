#pragma once
#include <string>
#include <map>

class RealStoragePolicy {
    static std::map<std::string, float> data;
    static std::map<std::string, int> dataInt;
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
    static void putInt(const char* key, int val) {
        dataInt[key] = val;
    }
    static int getInt(const char* key, int defaultValue) {
        if (dataInt.find(key) != dataInt.end()) {
            return dataInt[key];
        }
        return defaultValue;
    }
};

std::map<std::string, float> RealStoragePolicy::data;
std::map<std::string, int> RealStoragePolicy::dataInt;
