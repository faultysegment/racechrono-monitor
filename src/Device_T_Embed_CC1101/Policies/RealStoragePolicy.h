#pragma once
#include <Arduino.h>
#include <Preferences.h>
#include <SD.h>
#include <SPI.h>
#include <FS.h>
#include <string>

#define BOARD_SD_CS   13
#define BOARD_SD_SCLK 11
#define BOARD_SD_MOSI 9
#define BOARD_SD_MISO 10

class RealStoragePolicy {
    SPIClass sdSPI;
    bool checked = false;
    bool mounted = false;

public:
    RealStoragePolicy() : sdSPI(HSPI), checked(false), mounted(false) {}

    ~RealStoragePolicy() {
        if (mounted) {
            SD.end();
            sdSPI.end();
            mounted = false;
        }
    }

    bool initSD() {
        if (checked) return mounted;
        checked = true;

        pinMode(BOARD_SD_CS, OUTPUT);
        digitalWrite(BOARD_SD_CS, HIGH);

        sdSPI.begin(BOARD_SD_SCLK, BOARD_SD_MISO, BOARD_SD_MOSI, BOARD_SD_CS);
        if (SD.begin(BOARD_SD_CS, sdSPI, 20000000)) {
            if (SD.cardType() != CARD_NONE) {
                mounted = true;
                return true;
            }
        }
        
        SD.end();
        sdSPI.end();
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

        // Fallback to internal Preferences
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
