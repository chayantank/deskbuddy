#pragma once
// ============================================================================
// DeskBuddy — Menu System
// ============================================================================
#include <Arduino.h>
#include "display_manager.h"
#include "touch_input.h"

enum class AppID {
    FACE = 0,
    CLOCK = 1,
    WEATHER = 2,
    WORLD_INFO = 3,
    SERVER = 4,
    GAMES = 5,
    SETTINGS = 6,
    NONE = 255
};

class MenuSystem {
public:
    void begin(DisplayManager* dm);
    void update();
    void render();
    
    void handleTouch(TouchEvent event);
    
    bool isActive() const { return _active; }
    void show();
    void hide();
    
    AppID getSelectedApp() const { return _selectedApp; }
    void clearSelectedApp() { _selectedApp = AppID::NONE; }
    
private:
    DisplayManager* _dm = nullptr;
    bool _active = false;
    
    int8_t _currentIndex = 0;
    float _scrollOffset = 0.0f; // For smooth scrolling
    float _targetScrollOffset = 0.0f;
    
    AppID _selectedApp = AppID::NONE;
    
    unsigned long _lastInteractionTime = 0;
    
    void _drawCarousel();
};
