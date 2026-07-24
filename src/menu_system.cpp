// ============================================================================
// DeskBuddy — Menu System Implementation
// ============================================================================
#include "menu_system.h"
#include "icons.h"
#include "easing.h"

struct MenuItem {
    AppID id;
    const char* name;
    const uint8_t* icon;
};

static const MenuItem MENU_ITEMS[MENU_ITEM_COUNT] = {
    {AppID::FACE,       "Face",     icon_face_24},
    {AppID::CLOCK,      "Clock",    icon_clock_24},
    {AppID::WEATHER,    "Weather",  icon_weather_24},
    {AppID::WORLD_INFO, "Info",     icon_globe_24},
    {AppID::GAMES,      "Games",    icon_games_24},
    {AppID::SETTINGS,   "Settings", icon_settings_24}
};

void MenuSystem::begin(DisplayManager* dm) {
    _dm = dm;
    _currentIndex = 0;
    _scrollOffset = 0.0f;
    _targetScrollOffset = 0.0f;
    _active = false;
    _selectedApp = AppID::NONE;
}

void MenuSystem::show() {
    _active = true;
    _lastInteractionTime = millis();
    _dm->startTransition(Transition::SLIDE_UP, 300);
}

void MenuSystem::hide() {
    _active = false;
}

void MenuSystem::update() {
    if (!_active) return;
    
    // Smooth scroll interpolation
    if (abs(_scrollOffset - _targetScrollOffset) > 0.01f) {
        _scrollOffset = Easing::lerp(_scrollOffset, _targetScrollOffset, 0.2f);
    } else {
        _scrollOffset = _targetScrollOffset;
    }
    
    // Auto-hide menu if no interaction for 10 seconds
    if (millis() - _lastInteractionTime > 10000) {
        _selectedApp = AppID::FACE; // default back to face
    }
}

void MenuSystem::render() {
    if (!_active || !_dm) return;
    
    Adafruit_SSD1306& d = _dm->display();
    
    // Header
    d.setTextSize(1);
    int16_t x1, y1;
    uint16_t w, h;
    d.getTextBounds(MENU_ITEMS[_currentIndex].name, 0, 0, &x1, &y1, &w, &h);
    d.setCursor((SCREEN_W - w) / 2, 4);
    d.print(MENU_ITEMS[_currentIndex].name);
    
    // Line under header
    d.drawLine(0, 14, SCREEN_W, 14, SSD1306_WHITE);
    
    _drawCarousel();
    
    // Pagination dots
    int dotSpacing = 8;
    int totalWidth = (MENU_ITEM_COUNT - 1) * dotSpacing;
    int startX = (SCREEN_W - totalWidth) / 2;
    for (int i = 0; i < MENU_ITEM_COUNT; i++) {
        if (i == _currentIndex) {
            d.fillCircle(startX + i * dotSpacing, SCREEN_H - 6, 2, SSD1306_WHITE);
        } else {
            d.drawPixel(startX + i * dotSpacing, SCREEN_H - 6, SSD1306_WHITE);
        }
    }
}

void MenuSystem::_drawCarousel() {
    Adafruit_SSD1306& d = _dm->display();
    
    const int itemWidth = 40; // Spacing between items
    const int centerY = 32;
    const int centerX = SCREEN_W / 2;
    
    for (int i = 0; i < MENU_ITEM_COUNT; i++) {
        // Calculate position based on current index and scroll offset
        float distFromCenter = i - (_currentIndex + (_scrollOffset - _targetScrollOffset) / itemWidth);
        
        // Wrap around logic for circular carousel (visual only)
        if (distFromCenter > MENU_ITEM_COUNT / 2.0f) distFromCenter -= MENU_ITEM_COUNT;
        if (distFromCenter < -MENU_ITEM_COUNT / 2.0f) distFromCenter += MENU_ITEM_COUNT;
        
        int x = centerX + (int)(distFromCenter * itemWidth);
        
        // Only draw if on screen
        if (x > -30 && x < SCREEN_W + 30) {
            // Scale icon based on distance from center (optional effect, here we just use fixed size)
            int iconX = x - MENU_ICON_SIZE / 2;
            int iconY = centerY - MENU_ICON_SIZE / 2;
            
            d.drawBitmap(iconX, iconY, MENU_ITEMS[i].icon, MENU_ICON_SIZE, MENU_ICON_SIZE, SSD1306_WHITE);
            
            // Draw a bounding box for the selected item
            if (i == _currentIndex && abs(distFromCenter) < 0.1f) {
                d.drawRoundRect(iconX - 4, iconY - 4, MENU_ICON_SIZE + 8, MENU_ICON_SIZE + 8, 4, SSD1306_WHITE);
            }
        }
    }
}

void MenuSystem::handleTouch(TouchEvent event) {
    if (!_active) return;
    
    _lastInteractionTime = millis();
    
    switch (event) {
        case TouchEvent::TAP:
            // Next item
            _currentIndex = (_currentIndex + 1) % MENU_ITEM_COUNT;
            _targetScrollOffset += 40.0f; // itemWidth
            break;
            
        case TouchEvent::DOUBLE_TAP:
            // Back to face
            _selectedApp = AppID::FACE;
            break;
            
        case TouchEvent::LONG_PRESS:
            // Select current
            _selectedApp = MENU_ITEMS[_currentIndex].id;
            break;
            
        default:
            break;
    }
}
