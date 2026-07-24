// ============================================================================
// DeskBuddy — NVS Storage Implementation
// ============================================================================
#include "storage.h"
#include <Preferences.h>

static Preferences prefs;

bool Storage::begin() {
    return prefs.begin(NVS_NAMESPACE, false);
}

bool Storage::saveString(const char* key, const String& value) {
    return prefs.putString(key, value) > 0;
}

String Storage::loadString(const char* key, const String& defaultVal) {
    return prefs.getString(key, defaultVal);
}

bool Storage::saveInt(const char* key, int32_t value) {
    return prefs.putInt(key, value) > 0;
}

int32_t Storage::loadInt(const char* key, int32_t defaultVal) {
    return prefs.getInt(key, defaultVal);
}

bool Storage::saveFloat(const char* key, float value) {
    return prefs.putFloat(key, value) > 0;
}

float Storage::loadFloat(const char* key, float defaultVal) {
    return prefs.getFloat(key, defaultVal);
}

bool Storage::saveBool(const char* key, bool value) {
    return prefs.putBool(key, value);
}

bool Storage::loadBool(const char* key, bool defaultVal) {
    return prefs.getBool(key, defaultVal);
}

// --- Convenience Accessors ---

bool Storage::isConfigured() {
    return loadBool(NVS_KEY_CONFIGURED, false);
}

void Storage::setConfigured(bool val) {
    saveBool(NVS_KEY_CONFIGURED, val);
}

String Storage::getWifiSSID() {
    return loadString(NVS_KEY_WIFI_SSID);
}

String Storage::getWifiPass() {
    return loadString(NVS_KEY_WIFI_PASS);
}

void Storage::setWifi(const String& ssid, const String& pass) {
    saveString(NVS_KEY_WIFI_SSID, ssid);
    saveString(NVS_KEY_WIFI_PASS, pass);
}

float Storage::getLatitude() {
    return loadFloat(NVS_KEY_LATITUDE, 28.6139f); // Default: New Delhi
}

float Storage::getLongitude() {
    return loadFloat(NVS_KEY_LONGITUDE, 77.2090f);
}

String Storage::getCityName() {
    return loadString(NVS_KEY_CITY_NAME, "New Delhi");
}

void Storage::setLocation(float lat, float lon, const String& cityName) {
    saveFloat(NVS_KEY_LATITUDE, lat);
    saveFloat(NVS_KEY_LONGITUDE, lon);
    if (cityName.length() > 0) {
        saveString(NVS_KEY_CITY_NAME, cityName);
    }
}

bool Storage::geocodeCity(const String& cityName, float& lat, float& lon) {
    if (cityName.length() == 0) return false;
    
    String c = cityName;
    c.toLowerCase();
    c.trim();
    
    // Indian Cities
    if (c.indexOf("delhi") >= 0 || c.indexOf("noida") >= 0 || c.indexOf("gurgaon") >= 0) { lat = 28.6139f; lon = 77.2090f; return true; }
    if (c.indexOf("mumbai") >= 0 || c.indexOf("bombay") >= 0) { lat = 19.0760f; lon = 72.8777f; return true; }
    if (c.indexOf("bangalore") >= 0 || c.indexOf("bengaluru") >= 0) { lat = 12.9716f; lon = 77.5946f; return true; }
    if (c.indexOf("kolkata") >= 0 || c.indexOf("calcutta") >= 0) { lat = 22.5726f; lon = 88.3639f; return true; }
    if (c.indexOf("chennai") >= 0 || c.indexOf("madras") >= 0) { lat = 13.0827f; lon = 80.2707f; return true; }
    if (c.indexOf("hyderabad") >= 0) { lat = 17.3850f; lon = 78.4867f; return true; }
    if (c.indexOf("pune") >= 0) { lat = 18.5204f; lon = 73.8567f; return true; }
    if (c.indexOf("ahmedabad") >= 0) { lat = 23.0225f; lon = 72.5714f; return true; }
    if (c.indexOf("jaipur") >= 0) { lat = 26.9124f; lon = 75.7873f; return true; }
    if (c.indexOf("chandigarh") >= 0) { lat = 30.7333f; lon = 76.7794f; return true; }
    
    // Global Cities
    if (c.indexOf("london") >= 0) { lat = 51.5074f; lon = -0.1278f; return true; }
    if (c.indexOf("new york") >= 0 || c.indexOf("nyc") >= 0) { lat = 40.7128f; lon = -74.0060f; return true; }
    if (c.indexOf("tokyo") >= 0) { lat = 35.6762f; lon = 139.6503f; return true; }
    if (c.indexOf("paris") >= 0) { lat = 48.8566f; lon = 2.3522f; return true; }
    if (c.indexOf("dubai") >= 0) { lat = 25.2048f; lon = 55.2708f; return true; }
    if (c.indexOf("singapore") >= 0) { lat = 1.3521f; lon = 103.8198f; return true; }
    
    return false;
}

uint8_t Storage::getBrightness() {
    return (uint8_t)loadInt(NVS_KEY_BRIGHTNESS, 3); // Default: max
}

void Storage::setBrightness(uint8_t level) {
    saveInt(NVS_KEY_BRIGHTNESS, level);
}

bool Storage::getTimeFormat24() {
    return loadBool(NVS_KEY_TIME_FMT, true); // Default: 24h
}

void Storage::setTimeFormat24(bool is24h) {
    saveBool(NVS_KEY_TIME_FMT, is24h);
}

bool Storage::getDisplayFlip() {
    return loadBool(NVS_KEY_DISPLAY_FLIP, false);
}

void Storage::setDisplayFlip(bool flipped) {
    saveBool(NVS_KEY_DISPLAY_FLIP, flipped);
}

int32_t Storage::getTimezoneOffset() {
    return loadInt(NVS_KEY_TIMEZONE, 19800); // Default: IST (+5:30)
}

void Storage::setTimezoneOffset(int32_t offsetSec) {
    saveInt(NVS_KEY_TIMEZONE, offsetSec);
}

uint32_t Storage::getSleepTimeout() {
    return (uint32_t)loadInt(NVS_KEY_SLEEP_TIMEOUT, 120000); // Default: 2 min
}

void Storage::setSleepTimeout(uint32_t ms) {
    saveInt(NVS_KEY_SLEEP_TIMEOUT, (int32_t)ms);
}

uint8_t Storage::getClockMode() {
    return (uint8_t)loadInt(NVS_KEY_CLOCK_MODE, 0); // Default: MODE_FULL_DIGITAL (0)
}

void Storage::setClockMode(uint8_t mode) {
    saveInt(NVS_KEY_CLOCK_MODE, (int32_t)mode);
}

uint8_t Storage::getEyeStyle() {
    return (uint8_t)loadInt(NVS_KEY_EYE_STYLE, 0); // Default: CLASSIC (0)
}

void Storage::setEyeStyle(uint8_t style) {
    saveInt(NVS_KEY_EYE_STYLE, (int32_t)style);
}

int32_t Storage::getDinoHighScore() {
    return loadInt(NVS_KEY_DINO_HISCORE, 0);
}

void Storage::setDinoHighScore(int32_t score) {
    saveInt(NVS_KEY_DINO_HISCORE, score);
}

int32_t Storage::getPongHighScore() {
    return loadInt(NVS_KEY_PONG_HISCORE, 0);
}

void Storage::setPongHighScore(int32_t score) {
    saveInt(NVS_KEY_PONG_HISCORE, score);
}

int32_t Storage::getSimonHighScore() {
    return loadInt(NVS_KEY_SIMON_HISCORE, 0);
}

void Storage::setSimonHighScore(int32_t score) {
    saveInt(NVS_KEY_SIMON_HISCORE, score);
}

int32_t Storage::getReactionBest() {
    return loadInt(NVS_KEY_REACT_BEST, 9999);
}

void Storage::setReactionBest(int32_t ms) {
    saveInt(NVS_KEY_REACT_BEST, ms);
}

void Storage::eraseAll() {
    prefs.clear();
}
