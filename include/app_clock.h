#pragma once
// ============================================================================
// DeskBuddy — Clock App
// ============================================================================
#include <Arduino.h>
#include "display_manager.h"
#include "time_service.h"
#include "touch_input.h"

enum class ClockMode {
    MODE_DIGITAL,
    MODE_ANALOG,
    MODE_BINARY,
    MODE_STOPWATCH,
    MODE_TIMER,
    COUNT
};

class AppClock {
public:
    void begin(DisplayManager* dm, TimeService* timeSvc, Storage* storage);
    
    void onEnter();
    void onExit();
    void update();
    void render();
    void handleTouch(TouchEvent event);
    
private:
    DisplayManager* _dm = nullptr;
    TimeService* _time = nullptr;
    Storage* _storage = nullptr;
    
    ClockMode _mode = ClockMode::MODE_DIGITAL;
    
    // Stopwatch state
    unsigned long _swStart = 0;
    unsigned long _swElapsed = 0;
    bool _swRunning = false;
    
    // Timer state
    unsigned long _tmrDuration = 0;
    unsigned long _tmrStart = 0;
    unsigned long _tmrRemaining = 0;
    bool _tmrRunning = false;
    int _tmrPresetIdx = 0;
    const int _tmrPresets[7] = {1, 3, 5, 10, 15, 25, 60}; // minutes including 25m Pomodoro
    
    // Render modes
    void _renderDigital();
    void _renderAnalog();
    void _renderBinary();
    void _renderStopwatch();
    void _renderTimer();
    
    void _drawHand(int cx, int cy, float angle, int length, bool thick = false);
};
