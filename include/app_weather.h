#pragma once
// ============================================================================
// DeskBuddy — Weather App
// ============================================================================
#include <Arduino.h>
#include "display_manager.h"
#include "storage.h"
#include "face_renderer.h"
#include "touch_input.h"

struct WeatherData {
    float currentTemp = 0;
    float currentWind = 0;
    int weatherCode = -1;
    
    // Forecast (next 3 days including today)
    float minTemp[3] = {0, 0, 0};
    float maxTemp[3] = {0, 0, 0};
    int dailyCode[3] = {-1, -1, -1};
    
    bool isValid = false;
    unsigned long lastFetch = 0;
};

class AppWeather {
public:
    void begin(DisplayManager* dm, Storage* storage, FaceRenderer* face);
    
    void onEnter();
    void onExit();
    void update();
    void render();
    void handleTouch(TouchEvent event);
    
    // Call periodically from main to update data in background
    void backgroundUpdate();

private:
    DisplayManager* _dm = nullptr;
    Storage* _storage = nullptr;
    FaceRenderer* _face = nullptr;
    
    WeatherData _data;
    bool _isFetching = false;
    int _viewMode = 0; // 0 = current, 1 = forecast
    
    void _fetchWeather();
    const uint8_t* _getIconForCode(int code);
    
    void _renderCurrent();
    void _renderForecast();
};
