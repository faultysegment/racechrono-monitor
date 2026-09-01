#pragma once
#include <Arduino.h>
#include <Preferences.h>
#include <SD.h>
#include <SPI.h>
#include <FS.h>
#include "pin_config.h"
#include <string>

class AmoledStoragePolicy {
    static SPIClass& getSPI() {
        static SPIClass sdSPI(HSPI);
        return sdSPI;
    }

    static bool& getMounted() {
        static bool mounted = false;
        return mounted;
    }

public:
    static bool initSD() {
        static bool checked = false;
        if (checked) return getMounted();
        checked = true;

        pinMode(SD_CS, OUTPUT);
        digitalWrite(SD_CS, HIGH);

        SPIClass& sdSPI = getSPI();
        sdSPI.begin(SD_SCLK, SD_MISO, SD_MOSI, SD_CS);
        if (SD.begin(SD_CS, sdSPI, 20000000)) {
            if (SD.cardType() != CARD_NONE) {
                getMounted() = true;
                return true;
            }
        }
        
        SD.end();
        sdSPI.end();
        pinMode(SD_CS, OUTPUT);
        digitalWrite(SD_CS, HIGH);
        getMounted() = false;
        return false;
    }

    static void init() {
        initSD();
    }

    static bool isCardPresent() {
        return getMounted();
    }

    static std::string readConfigFile(const char* filename = "/config.json") {
        if (isCardPresent()) {
            File file = SD.open(filename, FILE_READ);
            if (file) {
                std::string content;
                content.reserve(file.size());
                while (file.available()) {
                    content.push_back((char)file.read());
                }
                file.close();
                return content;
            }
        }
        
        // Fallback to internal Preferences
        Preferences prefs;
        prefs.begin("rcm_cfg", true);
        String s = prefs.getString("json", "");
        prefs.end();
        return std::string(s.c_str());
    }

    static bool writeConfigFile(const char* filename, const char* content) {
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
    
    static float getFloat(const char* key, float defaultValue) {
        Preferences prefs;
        prefs.begin("rcm_settings", true); 
        float val = prefs.getFloat(key, defaultValue);
        prefs.end();
        return val;
    }

    static void putFloat(const char* key, float value) {
        Preferences prefs;
        prefs.begin("rcm_settings", false);
        prefs.putFloat(key, value);
        prefs.end();
    }

    static int getInt(const char* key, int defaultValue) {
        Preferences prefs;
        prefs.begin("rcm_settings", true);
        int val = prefs.getInt(key, defaultValue);
        prefs.end();
        return val;
    }

    static void putInt(const char* key, int value) {
        Preferences prefs;
        prefs.begin("rcm_settings", false);
        prefs.putInt(key, value);
        prefs.end();
    }
};
