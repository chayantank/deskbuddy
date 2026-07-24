#pragma once
// ============================================================================
// DeskBuddy — NVS Storage Wrapper
// ============================================================================
#include <Arduino.h>
#include "config.h"

class Storage {
public:
    bool begin();
    
    // String values
    bool saveString(const char* key, const String& value);
    String loadString(const char* key, const String& defaultVal = "");
    
    // Integer values
    bool saveInt(const char* key, int32_t value);
    int32_t loadInt(const char* key, int32_t defaultVal = 0);
    
    // Float values  
    bool saveFloat(const char* key, float value);
    float loadFloat(const char* key, float defaultVal = 0.0f);
    
    // Bool values
    bool saveBool(const char* key, bool value);
    bool loadBool(const char* key, bool defaultVal = false);
    
    // Convenience accessors for common settings
    bool isConfigured();
    void setConfigured(bool val);
    
    // WiFi
    String getWifiSSID();
    String getWifiPass();
    void setWifi(const String& ssid, const String& pass);
    
    // Location
    float getLatitude();
    float getLongitude();
    String getCityName();
    void setLocation(float lat, float lon, const String& cityName = "");
    static bool geocodeCity(const String& cityName, float& lat, float& lon);
    
    // Preferences
    uint8_t getBrightness();
    void setBrightness(uint8_t level);
    bool getTimeFormat24();
    void setTimeFormat24(bool is24h);
    bool getDisplayFlip();
    void setDisplayFlip(bool flipped);
    int32_t getTimezoneOffset();
    void setTimezoneOffset(int32_t offsetSec);
    uint32_t getSleepTimeout();
    void setSleepTimeout(uint32_t ms);
    uint8_t getClockMode();
    void setClockMode(uint8_t mode);
    uint8_t getEyeStyle();
    void setEyeStyle(uint8_t style);
    
    // Game high scores
    int32_t getDinoHighScore();
    void setDinoHighScore(int32_t score);
    int32_t getPongHighScore();
    void setPongHighScore(int32_t score);
    int32_t getSimonHighScore();
    void setSimonHighScore(int32_t score);
    int32_t getReactionBest();
    void setReactionBest(int32_t ms);
    
    // Erase all stored data
    void eraseAll();
};
