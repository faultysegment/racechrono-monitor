#pragma once
#include <Arduino.h>
#include <Preferences.h>
#include <SD.h>
#include <SPI.h>
#include <FS.h>
#include <string>

#define BOARD_SD_CS 13

class RealStoragePolicy {
    bool checked = false;
    bool mounted = false;

public:
    RealStoragePolicy() = default;

    bool initSD() {
        if (checked) return mounted;
        checked = true;

        pinMode(BOARD_SD_CS, OUTPUT);
        digitalWrite(BOARD_SD_CS, HIGH);

        // On T-Embed, SPI bus is shared with TFT_eSPI (pins 11, 9, 10).
        // Never re-initialize or call sdSPI.end() as that detaches the SPI hardware pins.
        if (SD.begin(BOARD_SD_CS, SPI, 4000000)) {
            if (SD.cardType() != CARD_NONE) {
                mounted = true;
                return true;
            }
            SD.end();
        }
        
        pinMode(BOARD_SD_CS, OUTPUT);
        digitalWrite(BOARD_SD_CS, HIGH);
        mounted = false;
        return false;
    }

    void init() {
        initSD();
    }

    bool isCardPresent() {
        return mounted;
    }

    std::string readConfigFile(const char* filename = "/config.json") {
        if (isCardPresent()) {
            File file = SD.open(filename, FILE_READ);
            if (file) {
                std::string content;
                content.reserve(file.size());
                while (file.available()) {
                    content.push_back((char)file.read());
                }
                file.close();
                if (!content.empty()) {
                    return content;
                }
            }
        }

        // Fallback to internal Preferences (NVS flash)
        Preferences prefs;
        prefs.begin("rcm_cfg", true);
        String s = prefs.getString("json", "");
        prefs.end();
        return std::string(s.c_str());
    }

    bool writeConfigFile(const char* filename, const char* content) {
        if (!content) return false;
        bool writtenToSD = false;
        if (isCardPresent()) {
            File file = SD.open(filename, FILE_WRITE);
            if (file) {
                file.print(content);
                file.close();
                writtenToSD = true;
            }
        }

        // Always also save to Preferences for redundancy
        Preferences prefs;
        prefs.begin("rcm_cfg", false);
        prefs.putString("json", content);
        prefs.end();
        return writtenToSD || true;
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
