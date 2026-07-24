// ============================================================================
// DeskBuddy — Clock App Implementation
// ============================================================================
#include "app_clock.h"
#include "fonts.h"
#include "icons.h"
#include <math.h>

void AppClock::begin(DisplayManager* dm, TimeService* timeSvc, Storage* storage) {
    _dm = dm;
    _time = timeSvc;
    _storage = storage;
}

void AppClock::onEnter() {
    if (_storage) {
        uint8_t saved = _storage->getClockMode();
        if (saved < (uint8_t)ClockMode::COUNT) {
            _mode = (ClockMode)saved;
        }
    }
}

void AppClock::onExit() {
    // nothing to clean up
}

void AppClock::update() {
    unsigned long now = millis();
    
    if (_mode == ClockMode::MODE_STOPWATCH && _swRunning) {
        _swElapsed = now - _swStart;
    }
    else if (_mode == ClockMode::MODE_TIMER && _tmrRunning) {
        if (now - _tmrStart >= _tmrDuration) {
            _tmrRunning = false;
            _tmrRemaining = 0;
        } else {
            _tmrRemaining = _tmrDuration - (now - _tmrStart);
        }
    }
}

void AppClock::handleTouch(TouchEvent event) {
    switch (event) {
        case TouchEvent::TAP:
            if (_mode == ClockMode::MODE_STOPWATCH) {
                // Start / Stop stopwatch
                if (_swRunning) {
                    _swRunning = false;
                } else {
                    if (_swElapsed == 0) {
                        _swStart = millis();
                    } else {
                        // Resume
                        _swStart = millis() - _swElapsed;
                    }
                    _swRunning = true;
                }
            } else if (_mode == ClockMode::MODE_TIMER && !_tmrRunning) {
                // Cycle timer presets (7 presets)
                _tmrPresetIdx = (_tmrPresetIdx + 1) % 7;
                _tmrRemaining = _tmrPresets[_tmrPresetIdx] * 60000UL;
            } else if (_mode == ClockMode::MODE_TIMER && _tmrRunning) {
                // Stop timer
                _tmrRunning = false;
            } else {
                // Cycle mode
                _mode = (ClockMode)(((int)_mode + 1) % (int)ClockMode::COUNT);
                if (_storage) {
                    _storage->setClockMode((uint8_t)_mode);
                }
                // Reset states
                _swRunning = false;
                _swElapsed = 0;
                _tmrRunning = false;
                _tmrPresetIdx = 0;
                _tmrRemaining = _tmrPresets[0] * 60000UL;
            }
            break;
            
        case TouchEvent::LONG_PRESS:
            if (_mode == ClockMode::MODE_STOPWATCH) {
                // Reset
                _swRunning = false;
                _swElapsed = 0;
            } else if (_mode == ClockMode::MODE_TIMER) {
                // Start timer
                if (!_tmrRunning && _tmrRemaining > 0) {
                    _tmrRunning = true;
                    _tmrDuration = _tmrRemaining;
                    _tmrStart = millis();
                } else {
                    // Reset
                    _tmrRunning = false;
                    _tmrRemaining = _tmrPresets[_tmrPresetIdx] * 60000UL;
                }
            }
            break;
            
        default:
            break;
    }
}

void AppClock::render() {
    if (!_dm || !_time) return;
    
    switch (_mode) {
        case ClockMode::MODE_DIGITAL:    _renderDigital(); break;
        case ClockMode::MODE_ANALOG:     _renderAnalog(); break;
        case ClockMode::MODE_BINARY:     _renderBinary(); break;
        case ClockMode::MODE_STOPWATCH:  _renderStopwatch(); break;
        case ClockMode::MODE_TIMER:      _renderTimer(); break;
        default: break;
    }
}

void AppClock::_renderDigital() {
    Adafruit_SSD1306& d = _dm->display();
    
    bool is24 = _storage ? _storage->getTimeFormat24() : true;
    
    // Header: Live date (e.g. FRI, JUL 24)
    d.setFont();
    d.setTextSize(1);
    _dm->drawCenteredText(_time->getFormattedDate().c_str(), 2, 1);
    d.drawFastHLine(0, 12, SCREEN_W, SSD1306_WHITE);
    
    int h = _time->getHour();
    int m = _time->getMinute();
    int s = _time->getSecond();
    
    bool pm = false;
    if (!is24) {
        pm = h >= 12;
        if (h == 0) h = 12;
        else if (h > 12) h -= 12;
    }
    
    // Jumbo Size 3 time string (90px wide, 24px tall)
    char timeStr[10];
    if (s % 2 == 0) {
        snprintf(timeStr, sizeof(timeStr), "%02d:%02d", h, m);
    } else {
        snprintf(timeStr, sizeof(timeStr), "%02d %02d", h, m);
    }
    
    // Draw Jumbo bold digits
    d.setTextSize(3);
    d.setCursor(19, 20);
    d.print(timeStr);
    
    // 12h AM / PM badge
    d.setTextSize(1);
    if (!is24) {
        d.setCursor(111, 20);
        d.print(pm ? "P" : "A");
        d.setCursor(111, 30);
        d.print("M");
    }
    
    // Footer: Seconds Progress Track & Status
    d.drawFastHLine(0, 52, SCREEN_W, SSD1306_WHITE);
    int secW = (s * (SCREEN_W - 4)) / 59;
    if (secW > 0) {
        d.fillRect(2, 55, secW, 4, SSD1306_WHITE);
    }
}

void AppClock::_renderAnalog() {
    Adafruit_SSD1306& d = _dm->display();
    
    int cx = SCREEN_W / 2;
    int cy = 32;
    int r = 30;
    
    // Clock face
    d.drawCircle(cx, cy, r, SSD1306_WHITE);
    for (int i = 0; i < 12; i++) {
        float angle = i * 30.0f * (M_PI / 180.0f);
        int px1 = cx + sinf(angle) * (r - 2);
        int py1 = cy - cosf(angle) * (r - 2);
        int px2 = cx + sinf(angle) * r;
        int py2 = cy - cosf(angle) * r;
        d.drawLine(px1, py1, px2, py2, SSD1306_WHITE);
    }
    
    int h = _time->getHour();
    int m = _time->getMinute();
    int s = _time->getSecond();
    
    // Angles
    float hAngle = (h % 12 + m / 60.0f) * 30.0f * (M_PI / 180.0f);
    float mAngle = (m + s / 60.0f) * 6.0f * (M_PI / 180.0f);
    float sAngle = s * 6.0f * (M_PI / 180.0f);
    
    // Hands
    _drawHand(cx, cy, hAngle, r * 0.5f, true);
    _drawHand(cx, cy, mAngle, r * 0.8f, true);
    _drawHand(cx, cy, sAngle, r * 0.9f, false);
    
    // Center dot
    d.fillCircle(cx, cy, 2, SSD1306_WHITE);
}

void AppClock::_drawHand(int cx, int cy, float angle, int length, bool thick) {
    Adafruit_SSD1306& d = _dm->display();
    int px = cx + sinf(angle) * length;
    int py = cy - cosf(angle) * length;
    
    d.drawLine(cx, cy, px, py, SSD1306_WHITE);
    if (thick) {
        // draw slightly offset lines to make it thicker
        int ox = cosf(angle) * 1;
        int oy = sinf(angle) * 1;
        d.drawLine(cx + ox, cy + oy, px + ox, py + oy, SSD1306_WHITE);
        d.drawLine(cx - ox, cy - oy, px - ox, py - oy, SSD1306_WHITE);
    }
}

void AppClock::_renderBinary() {
    Adafruit_SSD1306& d = _dm->display();
    
    int h = _time->getHour();
    int m = _time->getMinute();
    int s = _time->getSecond();
    
    // H, M, S as BCD
    int h10 = h / 10, h1 = h % 10;
    int m10 = m / 10, m1 = m % 10;
    int s10 = s / 10, s1 = s % 10;
    
    int vals[6] = {h10, h1, m10, m1, s10, s1};
    
    int startX = 20;
    int gapX = 16;
    int gapY = 12;
    int startY = 50;
    
    d.setFont();
    d.setTextSize(1);
    
    // Header labels
    d.setCursor(startX + gapX/2 - 2, 4); d.print("H");
    d.setCursor(startX + 2*gapX + gapX/2 - 2, 4); d.print("M");
    d.setCursor(startX + 4*gapX + gapX/2 - 2, 4); d.print("S");
    
    for (int col = 0; col < 6; col++) {
        for (int row = 0; row < 4; row++) {
            // Check if this bit is valid for this column (e.g., tens of hours only needs 2 bits)
            if (col == 0 && row > 1) continue;
            if ((col == 2 || col == 4) && row > 2) continue;
            
            int bx = startX + col * gapX;
            int by = startY - row * gapY;
            
            bool isOn = (vals[col] & (1 << row)) != 0;
            
            if (isOn) {
                d.fillCircle(bx, by, 4, SSD1306_WHITE);
            } else {
                d.drawCircle(bx, by, 4, SSD1306_WHITE);
            }
        }
    }
}

void AppClock::_renderStopwatch() {
    Adafruit_SSD1306& d = _dm->display();
    
    d.setFont();
    d.setTextSize(1);
    _dm->drawCenteredText("STOPWATCH", 4, 1);
    
    unsigned long ms = _swElapsed % 1000;
    unsigned long sec = (_swElapsed / 1000) % 60;
    unsigned long min = (_swElapsed / 60000) % 60;
    
    char buf[16];
    snprintf(buf, sizeof(buf), "%02lu:%02lu.%03lu", min, sec, ms);
    
    d.setTextSize(2);
    int16_t x1, y1;
    uint16_t w, h;
    d.getTextBounds(buf, 0, 0, &x1, &y1, &w, &h);
    d.setCursor((SCREEN_W - w) / 2, 28);
    d.print(buf);
    
    d.setTextSize(1);
    if (_swRunning) {
        _dm->drawCenteredText("Tap to Stop", 52, 1);
    } else {
        if (_swElapsed == 0) {
            _dm->drawCenteredText("Tap to Start", 52, 1);
        } else {
            _dm->drawCenteredText("Tap Res | L-Press Rst", 52, 1);
        }
    }
}

void AppClock::_renderTimer() {
    Adafruit_SSD1306& d = _dm->display();
    
    d.setFont();
    d.setTextSize(1);
    _dm->drawCenteredText("TIMER", 4, 1);
    
    unsigned long sec = (_tmrRemaining / 1000) % 60;
    unsigned long min = (_tmrRemaining / 60000);
    
    char buf[16];
    snprintf(buf, sizeof(buf), "%02lu:%02lu", min, sec);
    
    d.setTextSize(2);
    int16_t x1, y1;
    uint16_t w, h;
    d.getTextBounds(buf, 0, 0, &x1, &y1, &w, &h);
    d.setCursor((SCREEN_W - w) / 2, 28);
    d.print(buf);
    
    d.setTextSize(1);
    if (_tmrRunning) {
        _dm->drawCenteredText("Tap to Stop", 52, 1);
        
        // Progress bar
        float pct = 1.0f - ((float)_tmrRemaining / _tmrDuration);
        _dm->drawProgressBar(14, 46, 100, 4, (uint8_t)(pct * 100));
    } else {
        if (_tmrRemaining > 0) {
            _dm->drawCenteredText("Tap Preset | Hold Start", 52, 1);
        } else {
            _dm->drawCenteredText("DONE!", 52, 1);
        }
    }
}
