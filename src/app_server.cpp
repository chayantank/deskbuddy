#include "app_server.h"
#include "config.h"
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

void AppServer::begin(DisplayManager* dm, Storage* storage) {
    _dm = dm;
    _storage = storage;
}

void AppServer::onEnter() {
    _fetchServerStats();
}

void AppServer::onExit() {
}

void AppServer::update() {
    if (millis() - _lastFetchTime > REFRESH_INTERVAL_MS) {
        _fetchServerStats();
    }
}

void AppServer::handleTouch(TouchEvent event) {
    if (event == TouchEvent::TAP || event == TouchEvent::LONG_PRESS) {
        _fetchServerStats(); // Force refresh
    }
}

void AppServer::_fetchServerStats() {
    if (WiFi.status() != WL_CONNECTED) return;
    
    _isFetching = true;
    _lastFetchTime = millis();
    
    HTTPClient http;
    http.setTimeout(2500); // Quick timeout for local network
    
    // Local Pi Server endpoint from config
    if (http.begin(SERVER_MONITOR_URL)) {
        int code = http.GET();
        if (code == HTTP_CODE_OK) {
            String payload = http.getString();
            JsonDocument doc;
            DeserializationError err = deserializeJson(doc, payload);
            
            if (!err) {
                JsonObject obj;
                if (doc["data"].is<JsonArray>() && doc["data"].size() > 0) {
                    obj = doc["data"][0];
                } else if (doc["data"].is<JsonObject>()) {
                    obj = doc["data"];
                }
                
                if (!obj.isNull()) {
                    _cpuTemp = obj["cpu_temperature"] | 0.0f;
                    _gpuTemp = obj["gpu_temperature"] | 0.0f;
                    _cpuUsage = obj["cpu_percent"] | 0.0f;
                    _ramUsage = obj["memory_percent"] | 0.0f;
                    _fanSpeed = obj["pwm_fan_speed"] | 0;
                    _downloadBps = obj["network_download_speed"] | 0L;
                    _uploadBps = obj["network_upload_speed"] | 0L;
                    _hasData = true;
                }
            }
        }
        http.end();
    }
    _isFetching = false;
}

void AppServer::render() {
    if (!_dm) return;
    Adafruit_SSD1306& d = _dm->display();
    
    d.setTextSize(1);
    
    // Header
    _dm->drawCenteredText(SERVER_DISPLAY_NAME, 2, 1);
    d.drawLine(0, 12, SCREEN_W, 12, SSD1306_WHITE);
    
    if (!_hasData && _isFetching) {
        _dm->drawCenteredText("Connecting...", 32, 1);
        return;
    }
    
    if (!_hasData) {
        if (WiFi.status() != WL_CONNECTED) {
            _dm->drawCenteredText("WiFi Disconnected", 30, 1);
        } else {
            _dm->drawCenteredText("Server Unreachable", 26, 1);
            _dm->drawCenteredText("Check Config IP", 38, 1);
            _dm->drawCenteredText("Tap to Retry", 52, 1);
        }
        return;
    }
    
    // Line 1: CPU Temp & CPU Usage %
    char bufCPU[32];
    snprintf(bufCPU, sizeof(bufCPU), "CPU: %.1fC  %.0f%%", _cpuTemp, _cpuUsage);
    d.setCursor(4, 16);
    d.print(bufCPU);
    
    // CPU Usage mini bar graph
    d.drawRect(92, 16, 32, 7, SSD1306_WHITE);
    int barW = (int)((_cpuUsage / 100.0f) * 30.0f);
    if (barW > 30) barW = 30;
    if (barW > 0) d.fillRect(93, 17, barW, 5, SSD1306_WHITE);
    
    // Line 2: RAM Usage & Fan Speed
    char bufRAM[32];
    snprintf(bufRAM, sizeof(bufRAM), "RAM: %.0f%%   Fan: %d", _ramUsage, _fanSpeed);
    d.setCursor(4, 28);
    d.print(bufRAM);
    
    // Line 3: Network Download & Upload speeds
    char bufDN[32], bufUP[32];
    long dnKB = _downloadBps / 1024;
    long upKB = _uploadBps / 1024;
    
    if (dnKB > 1024) {
        snprintf(bufDN, sizeof(bufDN), "DN: %.1fMB/s", dnKB / 1024.0f);
    } else {
        snprintf(bufDN, sizeof(bufDN), "DN: %ldKB/s", dnKB);
    }
    
    if (upKB > 1024) {
        snprintf(bufUP, sizeof(bufUP), "UP: %.1fMB/s", upKB / 1024.0f);
    } else {
        snprintf(bufUP, sizeof(bufUP), "UP: %ldKB/s", upKB);
    }
    
    d.setCursor(4, 40);
    d.print(bufDN);
    d.setCursor(68, 40);
    d.print(bufUP);
    
    // Footer: Live status dot & refresh hint
    d.drawFastHLine(0, 52, SCREEN_W, SSD1306_WHITE);
    if (_isFetching) {
        _dm->drawCenteredText("Updating...", 55, 1);
    } else {
        // Pulse live indicator dot
        if ((millis() / 500) % 2 == 0) {
            d.fillCircle(6, 58, 2, SSD1306_WHITE);
        }
        _dm->drawCenteredText("SERVER MONITOR", 55, 1);
    }
}
