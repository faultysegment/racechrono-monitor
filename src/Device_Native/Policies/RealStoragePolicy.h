#pragma once
#include <string>
#include <map>
#include <fstream>
#include <sstream>

class RealStoragePolicy {
    static std::map<std::string, float> data;
    static std::map<std::string, int> dataInt;
public:
    static void init() {}

    static bool isCardPresent() {
        return true;
    }

    static std::string readConfigFile(const char* filename = "config.json") {
        const char* path = (filename && filename[0] == '/') ? filename + 1 : filename;
        std::ifstream file(path);
        if (!file.is_open()) return "";
        std::stringstream buffer;
        buffer << file.rdbuf();
        return buffer.str();
    }

    static bool writeConfigFile(const char* filename, const char* content) {
        const char* path = (filename && filename[0] == '/') ? filename + 1 : filename;
        std::ofstream file(path);
        if (!file.is_open()) return false;
        file << (content ? content : "");
        return true;
    }

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
