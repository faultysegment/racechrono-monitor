#pragma once
#include <Preferences.h>

class AmoledStoragePolicy {
public:
    static void init() {}
    
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
