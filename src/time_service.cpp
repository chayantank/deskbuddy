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
    _baseMillis = millis();
    
    if (_storage) {
        configTime(_storage->getTimezoneOffset(), 0, NTP_SERVER_1, NTP_SERVER_2);
    }
}

time_t TimeService::_getCurrentEpoch() const {
    time_t nowTime = 0;
    time(&nowTime);
    if (nowTime > 1600000000) {
        return nowTime;
    }
    return _baseEpoch + (millis() - _baseMillis) / 1000;
}

void TimeService::update() {
    unsigned long now = millis();
    
    if (WiFi.status() == WL_CONNECTED) {
        if (!_isSynced || (now - _lastSyncTime > NTP_SYNC_INTERVAL_MS)) {
            syncTime();
        }
    }
}

bool TimeService::syncTime() {
    if (WiFi.status() != WL_CONNECTED) return false;
    
    if (_storage) {
        configTime(_storage->getTimezoneOffset(), 0, NTP_SERVER_1, NTP_SERVER_2);
    }
    
    struct tm timeinfo;
    if (getLocalTime(&timeinfo, 500)) { // 500ms non-blocking check
        time_t t = mktime(&timeinfo);
        if (t > 1600000000) {
            _isSynced = true;
            _baseEpoch = t;
            _baseMillis = millis();
            _lastSyncTime = millis();
            return true;
        }
    }
    
    return false;
}

int TimeService::getHour() const {
    time_t ep = _getCurrentEpoch();
    struct tm* tm_info = localtime(&ep);
    return tm_info ? tm_info->tm_hour : 12;
}

int TimeService::getMinute() const {
    time_t ep = _getCurrentEpoch();
    struct tm* tm_info = localtime(&ep);
    return tm_info ? tm_info->tm_min : 0;
}

int TimeService::getSecond() const {
    time_t ep = _getCurrentEpoch();
    struct tm* tm_info = localtime(&ep);
    return tm_info ? tm_info->tm_sec : 0;
}

int TimeService::getDay() const {
    time_t ep = _getCurrentEpoch();
    struct tm* tm_info = localtime(&ep);
    return tm_info ? tm_info->tm_mday : 24;
}

int TimeService::getMonth() const {
    time_t ep = _getCurrentEpoch();
    struct tm* tm_info = localtime(&ep);
    return tm_info ? (tm_info->tm_mon + 1) : 7;
}

int TimeService::getYear() const {
    time_t ep = _getCurrentEpoch();
    struct tm* tm_info = localtime(&ep);
    return tm_info ? (tm_info->tm_year + 1900) : 2026;
}

String TimeService::getFormattedTime(bool format12h, bool showSeconds) const {
    time_t ep = _getCurrentEpoch();
    struct tm* tm_info = localtime(&ep);
    if (!tm_info) return "12:00";
    
    char buffer[16];
    int h = tm_info->tm_hour;
    
    if (format12h) {
        bool pm = h >= 12;
        if (h == 0) h = 12;
        else if (h > 12) h -= 12;
        
        if (showSeconds) {
            snprintf(buffer, sizeof(buffer), "%d:%02d:%02d %s", h, tm_info->tm_min, tm_info->tm_sec, pm ? "PM" : "AM");
        } else {
            snprintf(buffer, sizeof(buffer), "%d:%02d %s", h, tm_info->tm_min, pm ? "PM" : "AM");
        }
    } else {
        if (showSeconds) {
            snprintf(buffer, sizeof(buffer), "%02d:%02d:%02d", h, tm_info->tm_min, tm_info->tm_sec);
        } else {
            snprintf(buffer, sizeof(buffer), "%02d:%02d", h, tm_info->tm_min);
        }
    }
    
    return String(buffer);
}

String TimeService::getFormattedDate() const {
    time_t ep = _getCurrentEpoch();
    struct tm* tm_info = localtime(&ep);
    if (!tm_info) return "FRI, JUL 24";
    
    char buffer[24];
    strftime(buffer, sizeof(buffer), "%a, %b %d", tm_info);
    for (int i = 0; buffer[i]; i++) buffer[i] = toupper(buffer[i]);
    return String(buffer);
}
