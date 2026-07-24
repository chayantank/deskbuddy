#pragma once
// ============================================================================
// DeskBuddy — World Info App
// ============================================================================
#include <Arduino.h>
#include "display_manager.h"
#include "storage.h"
#include "touch_input.h"

enum class InfoType {
    GOLD,
    SPORTS,
    INDIA_NEWS,
    TECH,
    CRYPTO,
    QUOTE,
    FACT,
    COUNT
};

class AppWorldInfo {
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
    
    InfoType _type = InfoType::GOLD;
    bool _isFetching = false;
    
    String _infoText = "";
    String _authorText = "";
    unsigned long _lastFetchTime[(int)InfoType::COUNT] = {0};
    
    // For scrolling long text
    int16_t _scrollX = 0;
    
    void _fetchData(InfoType type);
    void _fetchGold();
    void _fetchSports();
    void _fetchIndiaNews();
    void _fetchTechNews();
    void _fetchCrypto();
    void _fetchQuote();
    void _fetchFact();
};
