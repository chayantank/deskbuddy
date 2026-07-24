#pragma once
// ============================================================================
// DeskBuddy — Pi Server Monitor App (Pironman 5 Stats)
// ============================================================================
#include <Arduino.h>
#include "display_manager.h"
#include "storage.h"
#include "touch_input.h"

class AppServer {
public:
    void begin(DisplayManager* dm, Storage* storage);
    
    void onEnter();
    void onExit();
    void update();
    void render();
    void handleTouch(TouchEvent event);

private:
    DisplayManager* _dm = nullptr;
    Storage* _storage = nullptr;
    
    bool _isFetching = false;
    unsigned long _lastFetchTime = 0;
    const unsigned long REFRESH_INTERVAL_MS = 3000; // Auto refresh every 3 seconds
    
    // Server metrics
    float _cpuTemp = 0.0f;
    float _gpuTemp = 0.0f;
    float _cpuUsage = 0.0f;
    float _ramUsage = 0.0f;
    int _fanSpeed = 0;
    long _downloadBps = 0;
    long _uploadBps = 0;
    bool _hasData = false;
    
    void _fetchServerStats();
};
