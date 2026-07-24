#pragma once
// ============================================================================
// DeskBuddy — WiFi Manager & Captive Portal
// ============================================================================
#include <Arduino.h>
#include "display_manager.h"
#include "storage.h"

enum class WifiState {
    IDLE,
    CONNECTING,
    CONNECTED,
    FAILED,
    PORTAL_ACTIVE
};

class WifiManager {
public:
    void begin(DisplayManager* dm, Storage* storage);
    void update(); // call in main loop for DNS server
    
    // Connect to saved WiFi or start portal if none
    void autoConnect();
    
    // Force start the captive portal setup
    void startPortal();
    
    WifiState getState() const { return _state; }
    bool isConnected() const { return _state == WifiState::CONNECTED; }
    String getIP() const;
    
    // Manually disconnect
    void disconnect();
    
private:
    DisplayManager* _dm = nullptr;
    Storage* _storage = nullptr;
    
    WifiState _state = WifiState::IDLE;
    unsigned long _connectStartTime = 0;
    
    void _startAP();
    void _setupWebRoutes();
    void _drawConnectingScreen();
    void _drawPortalScreen();
};
