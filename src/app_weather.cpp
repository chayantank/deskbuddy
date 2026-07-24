// ============================================================================
// DeskBuddy — Weather App Implementation
// ============================================================================
#include "app_weather.h"
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include "icons.h"

void AppWeather::begin(DisplayManager* dm, Storage* storage, FaceRenderer* face) {
    _dm = dm;
    _storage = storage;
    _face = face;
}

void AppWeather::onEnter() {
    _viewMode = 0;
    // Force fetch if invalid or old
    if (!_data.isValid || millis() - _data.lastFetch > WEATHER_REFRESH_MS) {
        _fetchWeather();
    }
}

void AppWeather::onExit() {
}

void AppWeather::update() {
    // nothing needed per frame
}

void AppWeather::backgroundUpdate() {
    if (WiFi.status() == WL_CONNECTED) {
        if (!_data.isValid || (millis() - _data.lastFetch > WEATHER_REFRESH_MS)) {
            _fetchWeather();
        }
    }
}

void AppWeather::handleTouch(TouchEvent event) {
    if (event == TouchEvent::TAP) {
        _viewMode = (_viewMode + 1) % 2; // Toggle between current and forecast
    } else if (event == TouchEvent::LONG_PRESS) {
        _fetchWeather(); // Force refresh
    }
}

void AppWeather::render() {
    if (!_dm) return;
    
    if (_isFetching) {
        _dm->drawCenteredText("Fetching...", 30, 1);
        return;
    }
    
    if (!_data.isValid) {
        _dm->drawCenteredText("No Data", 24, 1);
        if (WiFi.status() != WL_CONNECTED) {
            _dm->drawCenteredText("WiFi Disconnected", 40, 1);
        } else {
            _dm->drawCenteredText("Long press to retry", 40, 1);
        }
        return;
    }
    
    if (_viewMode == 0) {
        _renderCurrent();
    } else {
        _renderForecast();
    }
    
    // Pagination dots
    Adafruit_SSD1306& d = _dm->display();
    int startX = SCREEN_W / 2 - 4;
    d.drawPixel(startX, 60, _viewMode == 0 ? SSD1306_WHITE : SSD1306_BLACK);
    if (_viewMode == 0) d.fillCircle(startX, 60, 2, SSD1306_WHITE);
    
    d.drawPixel(startX + 8, 60, _viewMode == 1 ? SSD1306_WHITE : SSD1306_BLACK);
    if (_viewMode == 1) d.fillCircle(startX + 8, 60, 2, SSD1306_WHITE);
}

#include <WiFiClientSecure.h>

void AppWeather::_renderCurrent() {
    Adafruit_SSD1306& d = _dm->display();
    
    d.setTextSize(1);
    String header = "WEATHER";
    if (_storage) {
        String city = _storage->getCityName();
        if (city.length() > 0) {
            header = city;
            header.toUpperCase();
        }
    }
    _dm->drawCenteredText(header.c_str(), 2, 1);
    
    // Icon
    const uint8_t* icon = _getIconForCode(_data.weatherCode);
    d.drawBitmap(24, 20, icon, 16, 16, SSD1306_WHITE);
    
    // Temp
    d.setTextSize(2);
    d.setCursor(50, 20);
    d.print((int)round(_data.currentTemp));
    d.print((char)247); // Degree symbol
    
    // Wind
    d.setTextSize(1);
    char buf[20];
    snprintf(buf, sizeof(buf), "Wind: %.1f km/h", _data.currentWind);
    _dm->drawCenteredText(buf, 44, 1);
}

void AppWeather::_renderForecast() {
    Adafruit_SSD1306& d = _dm->display();
    
    d.setTextSize(1);
    _dm->drawCenteredText("3-DAY FORECAST", 2, 1);
    
    for (int i = 0; i < 3; i++) {
        int x = 12 + i * 40;
        
        // Day label
        d.setCursor(x, 16);
        if (i == 0) d.print("Tdy");
        else if (i == 1) d.print("Tmw");
        else d.print("Nxt");
        
        // Icon
        const uint8_t* icon = _getIconForCode(_data.dailyCode[i]);
        d.drawBitmap(x + 2, 28, icon, 16, 16, SSD1306_WHITE);
        
        // Temp range
        char buf[8];
        snprintf(buf, sizeof(buf), "%d-%d", (int)round(_data.minTemp[i]), (int)round(_data.maxTemp[i]));
        d.setCursor(x - 2, 48);
        d.print(buf);
    }
}

void AppWeather::_fetchWeather() {
    if (WiFi.status() != WL_CONNECTED || !_storage) return;
    
    _isFetching = true;
    if (_dm) {
        _dm->beginFrame();
        _dm->drawCenteredText("Fetching Weather...", 30, 1);
        _dm->show();
    }
    
    WiFiClientSecure client;
    client.setInsecure();
    
    HTTPClient http;
    http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);
    http.setTimeout(7000);
    
    float lat = _storage->getLatitude();
    float lon = _storage->getLongitude();
    
    String url = String(API_WEATHER) + 
                 "?latitude=" + String(lat, 4) + 
                 "&longitude=" + String(lon, 4) + 
                 "&current=temperature_2m,weather_code,wind_speed_10m" + 
                 "&daily=temperature_2m_max,temperature_2m_min,weather_code" + 
                 "&timezone=auto";
                 
    if (http.begin(client, url)) {
        int httpCode = http.GET();
        if (httpCode == HTTP_CODE_OK) {
            String payload = http.getString();
            JsonDocument doc;
            DeserializationError error = deserializeJson(doc, payload);
            
            if (!error) {
                _data.currentTemp = doc["current"]["temperature_2m"];
                _data.weatherCode = doc["current"]["weather_code"];
                _data.currentWind = doc["current"]["wind_speed_10m"];
                
                JsonArray minT = doc["daily"]["temperature_2m_min"];
                JsonArray maxT = doc["daily"]["temperature_2m_max"];
                JsonArray codes = doc["daily"]["weather_code"];
                
                for (int i = 0; i < 3 && i < minT.size(); i++) {
                    _data.minTemp[i] = minT[i];
                    _data.maxTemp[i] = maxT[i];
                    _data.dailyCode[i] = codes[i];
                }
                
                _data.isValid = true;
                _data.lastFetch = millis();
                
                if (_face) {
                    if (_data.weatherCode == 0 || _data.weatherCode == 1) { // Clear/Sunny
                        _face->onGoodWeather(); 
                    }
                }
            }
        }
        http.end();
    }
    _isFetching = false;
}

const uint8_t* AppWeather::_getIconForCode(int code) {
    // WMO Weather interpretation codes
    if (code == 0) return weather_sun_16; // Clear
    if (code == 1 || code == 2) return weather_partcloud_16; // Partly cloudy
    if (code == 3) return weather_cloud_16; // Overcast
    if (code >= 45 && code <= 48) return weather_fog_16; // Fog
    if (code >= 51 && code <= 67) return weather_rain_16; // Drizzle/Rain
    if (code >= 71 && code <= 77) return weather_snow_16; // Snow
    if (code >= 80 && code <= 82) return weather_rain_16; // Showers
    if (code >= 85 && code <= 86) return weather_snow_16; // Snow showers
    if (code >= 95) return weather_storm_16; // Thunderstorm
    
    return weather_sun_16; // Default
}
