// ============================================================================
// DeskBuddy — Time Service Implementation
// ============================================================================
#include "time_service.h"
#include <time.h>
#include <WiFi.h>

void TimeService::begin(Storage* storage) {
    _storage = storage;
    _isSynced = false;
    _lastSyncTime = 0;
    
    if (_storage) {
        // Init time config with stored timezone
        configTime(_storage->getTimezoneOffset(), 0, NTP_SERVER_1, NTP_SERVER_2);
    }
}

void TimeService::update() {
    unsigned long now = millis();
    
    // Try to sync periodically if connected to WiFi
    if (WiFi.status() == WL_CONNECTED) {
        if (!_isSynced || (now - _lastSyncTime > NTP_SYNC_INTERVAL_MS)) {
            syncTime();
        }
    }
}

bool TimeService::syncTime() {
    if (WiFi.status() != WL_CONNECTED) return false;
    
    configTime(_storage->getTimezoneOffset(), 0, NTP_SERVER_1, NTP_SERVER_2);
    
    struct tm timeinfo;
    if (getLocalTime(&timeinfo, 5000)) { // 5s timeout
        _isSynced = true;
        _lastSyncTime = millis();
        return true;
    }
    
    return false;
}

int TimeService::getHour() const {
    struct tm timeinfo;
    if (getLocalTime(&timeinfo, 10)) {
        return timeinfo.tm_hour;
    }
    return 0;
}

int TimeService::getMinute() const {
    struct tm timeinfo;
    if (getLocalTime(&timeinfo, 10)) {
        return timeinfo.tm_min;
    }
    return 0;
}

int TimeService::getSecond() const {
    struct tm timeinfo;
    if (getLocalTime(&timeinfo, 10)) {
        return timeinfo.tm_sec;
    }
    return 0;
}

int TimeService::getDay() const {
    struct tm timeinfo;
    if (getLocalTime(&timeinfo, 10)) {
        return timeinfo.tm_mday;
    }
    return 1;
}

int TimeService::getMonth() const {
    struct tm timeinfo;
    if (getLocalTime(&timeinfo, 10)) {
        return timeinfo.tm_mon + 1;
    }
    return 1;
}

int TimeService::getYear() const {
    struct tm timeinfo;
    if (getLocalTime(&timeinfo, 10)) {
        return timeinfo.tm_year + 1900;
    }
    return 2024;
}

String TimeService::getFormattedTime(bool format12h, bool showSeconds) const {
    struct tm timeinfo;
    if (!getLocalTime(&timeinfo, 10)) {
        return showSeconds ? "--:--:--" : "--:--";
    }
    
    char buffer[16];
    int h = timeinfo.tm_hour;
    
    if (format12h) {
        bool pm = h >= 12;
        if (h == 0) h = 12;
        else if (h > 12) h -= 12;
        
        if (showSeconds) {
            snprintf(buffer, sizeof(buffer), "%d:%02d:%02d %s", h, timeinfo.tm_min, timeinfo.tm_sec, pm ? "PM" : "AM");
        } else {
            snprintf(buffer, sizeof(buffer), "%d:%02d %s", h, timeinfo.tm_min, pm ? "PM" : "AM");
        }
    } else {
        if (showSeconds) {
            snprintf(buffer, sizeof(buffer), "%02d:%02d:%02d", h, timeinfo.tm_min, timeinfo.tm_sec);
        } else {
            snprintf(buffer, sizeof(buffer), "%02d:%02d", h, timeinfo.tm_min);
        }
    }
    
    return String(buffer);
}

String TimeService::getFormattedDate() const {
    struct tm timeinfo;
    if (!getLocalTime(&timeinfo, 10)) {
        return "No Date";
    }
    
    char buffer[24];
    strftime(buffer, sizeof(buffer), "%a, %b %d", &timeinfo);
    return String(buffer);
}
