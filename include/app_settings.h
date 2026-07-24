#pragma once
// ============================================================================
// DeskBuddy — Settings App
// ============================================================================
#include <Arduino.h>
#include "display_manager.h"
#include "storage.h"
#include "wifi_manager.h"
#include "touch_input.h"

#include "face_renderer.h"

class AppSettings {
public:
    void begin(DisplayManager* dm, Storage* storage, WifiManager* wifi, FaceRenderer* face = nullptr);
    
    void onEnter();
    void onExit();
    void update();
    void render();
    void handleTouch(TouchEvent event);

private:
    DisplayManager* _dm = nullptr;
    Storage* _storage = nullptr;
    WifiManager* _wifi = nullptr;
    FaceRenderer* _face = nullptr;
    
    int _menuIndex = 0;
    static const int NUM_SETTINGS = 9;
    int _previewExprIndex = 0;
    
    // Setting values loaded on enter
    uint8_t _brightness = 3;
    bool _is24h = true;
    bool _isFlipped = false;
    uint8_t _eyeStyle = 0;
    uint32_t _sleepTimeout = 120000;
    
    void _drawSettingRow(int y, const char* label, const char* valStr, bool selected);
};
