#pragma once
// ============================================================================
// DeskBuddy — Display Manager
// ============================================================================
#include <Adafruit_SSD1306.h>
#include <Adafruit_GFX.h>
#include "config.h"

enum class Transition {
    NONE,
    SLIDE_LEFT,
    SLIDE_RIGHT,
    SLIDE_UP,
    FADE
};

class DisplayManager {
public:
    bool begin();
    void clear();
    void show();
    void setBrightness(uint8_t level); // 0-3 (25%, 50%, 75%, 100%)
    void setFlip(bool flipped);
    
    // Frame rate control
    void beginFrame();
    bool endFrame(); // returns true if frame was shown (enough time passed)
    uint16_t getFPS() const { return _measuredFPS; }
    
    // Transition effects
    void startTransition(Transition type, uint16_t durationMs = 300);
    bool isTransitioning() const { return _transActive; }
    void captureScreen(); // save current screen as "from" for transition
    
    // Drawing helpers
    void drawProgressBar(int16_t x, int16_t y, int16_t w, int16_t h, uint8_t pct);
    void drawCenteredText(const char* text, int16_t y, uint8_t textSize = 1);
    void drawScrollingText(const char* text, int16_t y, int16_t &scrollX, uint8_t textSize = 1);
    void drawRoundedBox(int16_t x, int16_t y, int16_t w, int16_t h, int16_t r, bool filled = false);
    void drawDottedLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1);
    void drawBatteryIcon(int16_t x, int16_t y, uint8_t pct);
    void drawWifiStrength(int16_t x, int16_t y, int8_t rssi);
    
    // Direct access to Adafruit display for custom drawing
    Adafruit_SSD1306& display() { return _display; }
    
private:
    Adafruit_SSD1306 _display{SCREEN_W, SCREEN_H, &Wire, OLED_RESET};
    
    // Frame timing
    unsigned long _lastFrameTime = 0;
    unsigned long _frameCount = 0;
    unsigned long _fpsTimer = 0;
    uint16_t _measuredFPS = 0;
    
    // Transition state
    bool _transActive = false;
    Transition _transType = Transition::NONE;
    unsigned long _transStart = 0;
    uint16_t _transDuration = 300;
    uint8_t _transFromBuf[SCREEN_W * SCREEN_H / 8];
    
    void _renderTransition();
};
