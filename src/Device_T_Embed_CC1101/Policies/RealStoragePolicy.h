#pragma once
#include <Preferences.h>
#include <SD.h>
#include <SPI.h>
#include <FS.h>
#include <string>

class RealStoragePolicy {
    bool initialized = false;
    bool mounted = false;

public:
    RealStoragePolicy() = default;

    bool initSD() {
        if (!initialized) {
            initialized = true;
            SPI.begin(12, 13, 11, 13); // SCLK=12, MISO=13, MOSI=11, CS=13
            mounted = SD.begin(13, SPI);
        }
        return mounted;
    }

    void init() {
        initSD();
    }

    bool isCardPresent() {
        if (!initSD()) return false;
        return (SD.cardType() != CARD_NONE);
    }

    std::string readConfigFile(const char* filename = "/config.json") {
        if (!isCardPresent()) return "";
        File file = SD.open(filename, FILE_READ);
        if (!file) return "";
        std::string content;
        content.reserve(file.size());
        while (file.available()) {
            content.push_back((char)file.read());
        }
        file.close();
        return content;
    }

    bool writeConfigFile(const char* filename, const char* content) {
        if (!isCardPresent()) return false;
        File file = SD.open(filename, FILE_WRITE);
        if (!file) return false;
        if (content) {
            file.print(content);
        }
        file.close();
        return true;
    }
    
    float getFloat(const char* key, float defaultValue) {
        Preferences prefs;
        prefs.begin("rcm_settings", true); 
        float val = prefs.getFloat(key, defaultValue);
        prefs.end();
        return val;
    }
    
    void putFloat(const char* key, float value) {
        Preferences prefs;
        prefs.begin("rcm_settings", false);
        prefs.putFloat(key, value);
        prefs.end();
    }

    int getInt(const char* key, int defaultValue) {
        Preferences prefs;
        prefs.begin("rcm_settings", true);
        int val = prefs.getInt(key, defaultValue);
        prefs.end();
        return val;
    }

    void putInt(const char* key, int value) {
        Preferences prefs;
        prefs.begin("rcm_settings", false);
        prefs.putInt(key, value);
        prefs.end();
    }
};
