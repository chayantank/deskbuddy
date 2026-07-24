// ============================================================================
// DeskBuddy — Display Manager Implementation
// ============================================================================
#include "display_manager.h"
#include "easing.h"
#include <Wire.h>

bool DisplayManager::begin() {
    Wire.begin(PIN_SDA, PIN_SCL);
    Wire.setClock(400000); // 400kHz I2C fast mode
    
    if (!_display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR)) {
        return false;
    }
    
    _display.clearDisplay();
    _display.setTextColor(SSD1306_WHITE);
    _display.setTextSize(1);
    _display.setTextWrap(false);
    _display.display();
    
    _lastFrameTime = millis();
    _fpsTimer = millis();
    return true;
}

void DisplayManager::clear() {
    _display.clearDisplay();
}

void DisplayManager::show() {
    if (_transActive) {
        _renderTransition();
    }
    _display.display();
}

void DisplayManager::setBrightness(uint8_t level) {
    // SSD1306 brightness via contrast command
    const uint8_t levels[] = {0x01, 0x40, 0xA0, 0xFF};
    uint8_t idx = level > 3 ? 3 : level;
    _display.ssd1306_command(SSD1306_SETCONTRAST);
    _display.ssd1306_command(levels[idx]);
}

void DisplayManager::setFlip(bool flipped) {
    if (flipped) {
        _display.ssd1306_command(SSD1306_SEGREMAP);       // flip horizontal
        _display.ssd1306_command(SSD1306_COMSCANINC);      // flip vertical
    } else {
        _display.ssd1306_command(SSD1306_SEGREMAP | 0x1);
        _display.ssd1306_command(SSD1306_COMSCANDEC);
    }
}

void DisplayManager::beginFrame() {
    clear();
}

bool DisplayManager::endFrame() {
    unsigned long now = millis();
    unsigned long elapsed = now - _lastFrameTime;
    
    if (elapsed < FRAME_TIME_MS) {
        return false; // too early
    }
    
    show();
    _lastFrameTime = now;
    _frameCount++;
    
    // Measure FPS every second
    if (now - _fpsTimer >= 1000) {
        _measuredFPS = _frameCount;
        _frameCount = 0;
        _fpsTimer = now;
    }
    
    return true;
}

void DisplayManager::captureScreen() {
    memcpy(_transFromBuf, _display.getBuffer(), sizeof(_transFromBuf));
}

void DisplayManager::startTransition(Transition type, uint16_t durationMs) {
    _transType = type;
    _transDuration = durationMs;
    _transStart = millis();
    _transActive = true;
}

void DisplayManager::_renderTransition() {
    unsigned long elapsed = millis() - _transStart;
    float t = Easing::clamp((float)elapsed / _transDuration, 0.0f, 1.0f);
    float eased = Easing::easeInOutCubic(t);
    
    if (t >= 1.0f) {
        _transActive = false;
        return;
    }
    
    uint8_t* currentBuf = _display.getBuffer();
    int bufSize = SCREEN_W * SCREEN_H / 8;
    
    switch (_transType) {
        case Transition::SLIDE_LEFT: {
            int offset = (int)(eased * SCREEN_W);
            // This is simplified — shifts columns
            // In practice, we'd do a pixel-level shift of the buffer
            uint8_t tempBuf[SCREEN_W * SCREEN_H / 8];
            memcpy(tempBuf, currentBuf, bufSize);
            
            for (int page = 0; page < SCREEN_H / 8; page++) {
                for (int col = 0; col < SCREEN_W; col++) {
                    int srcCol = col + offset;
                    int dstIdx = page * SCREEN_W + col;
                    if (srcCol < SCREEN_W) {
                        int fromIdx = page * SCREEN_W + srcCol;
                        currentBuf[dstIdx] = _transFromBuf[fromIdx];
                    } else {
                        int newCol = srcCol - SCREEN_W;
                        if (newCol < SCREEN_W) {
                            currentBuf[dstIdx] = tempBuf[page * SCREEN_W + newCol];
                        }
                    }
                }
            }
            break;
        }
        
        case Transition::SLIDE_RIGHT: {
            int offset = (int)(eased * SCREEN_W);
            uint8_t tempBuf[SCREEN_W * SCREEN_H / 8];
            memcpy(tempBuf, currentBuf, bufSize);
            
            for (int page = 0; page < SCREEN_H / 8; page++) {
                for (int col = 0; col < SCREEN_W; col++) {
                    int srcCol = col - offset;
                    int dstIdx = page * SCREEN_W + col;
                    if (srcCol >= 0) {
                        currentBuf[dstIdx] = _transFromBuf[page * SCREEN_W + srcCol];
                    } else {
                        int newCol = SCREEN_W + srcCol;
                        if (newCol >= 0 && newCol < SCREEN_W) {
                            currentBuf[dstIdx] = tempBuf[page * SCREEN_W + newCol];
                        }
                    }
                }
            }
            break;
        }
        
        case Transition::SLIDE_UP: {
            int offsetRows = (int)(eased * SCREEN_H);
            uint8_t tempBuf[SCREEN_W * SCREEN_H / 8];
            memcpy(tempBuf, currentBuf, bufSize);
            
            // Simplified vertical slide using page-level operations
            int offsetPages = offsetRows / 8;
            for (int page = 0; page < SCREEN_H / 8; page++) {
                int srcPage = page + offsetPages;
                for (int col = 0; col < SCREEN_W; col++) {
                    int dstIdx = page * SCREEN_W + col;
                    if (srcPage < (int)(SCREEN_H / 8)) {
                        currentBuf[dstIdx] = _transFromBuf[srcPage * SCREEN_W + col];
                    } else {
                        int newPage = srcPage - SCREEN_H / 8;
                        if (newPage >= 0 && newPage < (int)(SCREEN_H / 8)) {
                            currentBuf[dstIdx] = tempBuf[newPage * SCREEN_W + col];
                        }
                    }
                }
            }
            break;
        }
        
        case Transition::FADE: {
            // Dithered crossfade — randomly mix old and new pixels based on progress
            for (int i = 0; i < bufSize; i++) {
                uint8_t oldByte = _transFromBuf[i];
                uint8_t newByte = currentBuf[i];
                uint8_t mask = 0;
                for (int bit = 0; bit < 8; bit++) {
                    if ((float)(esp_random() % 100) / 100.0f < eased) {
                        mask |= (1 << bit);
                    }
                }
                currentBuf[i] = (newByte & mask) | (oldByte & ~mask);
            }
            break;
        }
        
        default:
            break;
    }
}

void DisplayManager::drawProgressBar(int16_t x, int16_t y, int16_t w, int16_t h, uint8_t pct) {
    _display.drawRoundRect(x, y, w, h, 2, SSD1306_WHITE);
    int16_t fillW = ((w - 4) * pct) / 100;
    if (fillW > 0) {
        _display.fillRoundRect(x + 2, y + 2, fillW, h - 4, 1, SSD1306_WHITE);
    }
}

void DisplayManager::drawCenteredText(const char* text, int16_t y, uint8_t textSize) {
    _display.setTextSize(textSize);
    int16_t x1, y1;
    uint16_t w, h;
    _display.getTextBounds(text, 0, y, &x1, &y1, &w, &h);
    _display.setCursor((SCREEN_W - w) / 2, y);
    _display.print(text);
}

void DisplayManager::drawScrollingText(const char* text, int16_t y, int16_t &scrollX, uint8_t textSize) {
    _display.setTextSize(textSize);
    int16_t x1, y1;
    uint16_t tw, th;
    _display.getTextBounds(text, 0, 0, &x1, &y1, &tw, &th);
    
    if ((int16_t)tw <= SCREEN_W) {
        // Fits on screen, just center it
        drawCenteredText(text, y, textSize);
        scrollX = 0;
        return;
    }
    
    // Scroll with padding
    _display.setCursor(-scrollX, y);
    _display.print(text);
    scrollX += MENU_SCROLL_SPEED;
    if (scrollX > (int16_t)(tw + 40)) {
        scrollX = -SCREEN_W;
    }
}

void DisplayManager::drawRoundedBox(int16_t x, int16_t y, int16_t w, int16_t h, int16_t r, bool filled) {
    if (filled) {
        _display.fillRoundRect(x, y, w, h, r, SSD1306_WHITE);
    } else {
        _display.drawRoundRect(x, y, w, h, r, SSD1306_WHITE);
    }
}

void DisplayManager::drawDottedLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1) {
    int16_t dx = abs(x1 - x0), dy = abs(y1 - y0);
    int16_t sx = x0 < x1 ? 1 : -1;
    int16_t sy = y0 < y1 ? 1 : -1;
    int16_t err = dx - dy;
    int count = 0;
    
    while (true) {
        if (count % 3 == 0) {
            _display.drawPixel(x0, y0, SSD1306_WHITE);
        }
        count++;
        if (x0 == x1 && y0 == y1) break;
        int16_t e2 = 2 * err;
        if (e2 > -dy) { err -= dy; x0 += sx; }
        if (e2 < dx)  { err += dx; y0 += sy; }
    }
}

void DisplayManager::drawWifiStrength(int16_t x, int16_t y, int8_t rssi) {
    int bars = 0;
    if (rssi > -50) bars = 4;
    else if (rssi > -60) bars = 3;
    else if (rssi > -70) bars = 2;
    else if (rssi > -80) bars = 1;
    
    for (int i = 0; i < 4; i++) {
        int bh = 2 + i * 2;
        int bx = x + i * 4;
        int by = y + 8 - bh;
        if (i < bars) {
            _display.fillRect(bx, by, 3, bh, SSD1306_WHITE);
        } else {
            _display.drawRect(bx, by, 3, bh, SSD1306_WHITE);
        }
    }
}
