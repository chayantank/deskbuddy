// ============================================================================
// DeskBuddy — Main Application Entry
// ============================================================================
#include <Arduino.h>
#include "config.h"
#include "display_manager.h"
#include "touch_input.h"
#include "storage.h"
#include "face_renderer.h"
#include "menu_system.h"
#include "wifi_manager.h"
#include "time_service.h"

// Apps
#include "app_clock.h"
#include "app_weather.h"
#include "app_world_info.h"
#include "app_server.h"
#include "app_games.h"
#include "app_settings.h"

// Core subsystems
DisplayManager  g_display;
TouchInput      g_touch;
Storage         g_storage;
FaceRenderer    g_face;
MenuSystem      g_menu;
WifiManager     g_wifi;
TimeService     g_time;

// Apps
AppClock        appClock;
AppWeather      appWeather;
AppWorldInfo    appWorldInfo;
AppServer       appServer;
AppGames        appGames;
AppSettings     appSettings;

// Current state
AppID g_currentApp = AppID::FACE;

void setup() {
    Serial.begin(115200);
    delay(500); // Give serial time to attach
    
    Serial.println("DeskBuddy Starting...");
    
    // Init subsystems
    g_storage.begin();
    
    if (!g_display.begin()) {
        Serial.println("OLED init failed!");
        while (1) delay(100);
    }
    
    // Apply initial settings
    g_display.setBrightness(g_storage.getBrightness());
    g_display.setFlip(g_storage.getDisplayFlip());
    
    g_touch.begin();
    g_face.begin(&g_display);
    g_face.setEyeStyle((EyeStyle)g_storage.getEyeStyle());
    
    g_menu.begin(&g_display);
    g_wifi.begin(&g_display, &g_storage);
    g_time.begin(&g_storage);
    
    // Init apps
    appClock.begin(&g_display, &g_time, &g_storage);
    appWeather.begin(&g_display, &g_storage, &g_face);
    appWorldInfo.begin(&g_display, &g_storage);
    appServer.begin(&g_display, &g_storage);
    appGames.begin(&g_display, &g_storage, &g_face);
    appSettings.begin(&g_display, &g_storage, &g_wifi, &g_face);
    
    // Check if configured
    if (!g_storage.isConfigured()) {
        g_wifi.startPortal();
        return; // Stay in portal until restarted
    }
    
    g_wifi.autoConnect();
}

void switchApp(AppID newApp) {
    if (g_currentApp == newApp) return;
    
    // Exit current
    switch (g_currentApp) {
        case AppID::CLOCK: appClock.onExit(); break;
        case AppID::WEATHER: appWeather.onExit(); break;
        case AppID::WORLD_INFO: appWorldInfo.onExit(); break;
        case AppID::SERVER: appServer.onExit(); break;
        case AppID::GAMES: appGames.onExit(); break;
        case AppID::SETTINGS: appSettings.onExit(); break;
        default: break;
    }
    
    g_currentApp = newApp;
    g_display.startTransition(Transition::FADE, 300);
    
    // Enter new
    switch (g_currentApp) {
        case AppID::FACE: g_face.resetIdleTimer(); break;
        case AppID::CLOCK: appClock.onEnter(); break;
        case AppID::WEATHER: appWeather.onEnter(); break;
        case AppID::WORLD_INFO: appWorldInfo.onEnter(); break;
        case AppID::SERVER: appServer.onEnter(); break;
        case AppID::GAMES: appGames.onEnter(); break;
        case AppID::SETTINGS: appSettings.onEnter(); break;
        default: break;
    }
}

void loop() {
    // 1. Update core services
    g_wifi.update();
    g_time.update();
    
    if (g_wifi.getState() == WifiState::PORTAL_ACTIVE) {
        // Portal takes over the loop entirely
        return;
    }
    
    // 2. Periodic background tasks
    static unsigned long lastBackground = 0;
    if (millis() - lastBackground > 60000) { // Every minute
        appWeather.backgroundUpdate();
        lastBackground = millis();
    }
    
    // Auto-cycle tabs every 2 minutes (Clock -> Weather -> Info -> Server -> Face)
    static unsigned long lastAutoCycle = millis();
    if (millis() - lastAutoCycle > 120000UL) {
        lastAutoCycle = millis();
        if (!g_menu.isActive() && g_currentApp != AppID::GAMES && g_currentApp != AppID::SETTINGS) {
            AppID nextTab;
            switch (g_currentApp) {
                case AppID::FACE:       nextTab = AppID::CLOCK; break;
                case AppID::CLOCK:      nextTab = AppID::WEATHER; break;
                case AppID::WEATHER:    nextTab = AppID::WORLD_INFO; break;
                case AppID::WORLD_INFO: nextTab = AppID::SERVER; break;
                case AppID::SERVER:     nextTab = AppID::FACE; break;
                default:                nextTab = AppID::CLOCK; break;
            }
            switchApp(nextTab);
        }
    }
    
    // 3. Frame rate cap
    if (!g_display.endFrame()) {
        // Yield to let WiFi/background tasks run while waiting for frame time
        delay(1); 
        return; 
    }
    
    // --- FRAME START ---
    g_display.beginFrame();
    
    // 4. Input processing
    TouchEvent ev = g_touch.update();
    if (ev != TouchEvent::NONE) {
        if (g_menu.isActive()) {
            g_menu.handleTouch(ev);
            if (g_menu.getSelectedApp() != AppID::NONE) {
                AppID nextApp = g_menu.getSelectedApp();
                g_menu.hide();
                g_menu.clearSelectedApp();
                switchApp(nextApp);
                lastAutoCycle = millis(); // Reset auto-cycle on user action
            }
        }
        else if (g_currentApp == AppID::GAMES) {
            appGames.handleTouch(ev);
            if (ev == TouchEvent::DOUBLE_TAP) {
                switchApp(AppID::FACE);
                lastAutoCycle = millis();
            }
        }
        else if (g_currentApp == AppID::SETTINGS) {
            appSettings.handleTouch(ev);
            if (ev == TouchEvent::DOUBLE_TAP) {
                switchApp(AppID::FACE);
                lastAutoCycle = millis();
            }
        }
        else {
            // TAP on any tab (Face, Clock, Weather, Info, Server) opens Menu!
            if (ev == TouchEvent::TAP) {
                g_display.captureScreen();
                g_menu.show();
                lastAutoCycle = millis();
            } else if (ev == TouchEvent::DOUBLE_TAP) {
                switchApp(AppID::FACE);
                lastAutoCycle = millis();
            } else {
                // Route long-press or other gestures
                switch (g_currentApp) {
                    case AppID::FACE: g_face.onTouch(); break;
                    case AppID::CLOCK: appClock.handleTouch(ev); break;
                    case AppID::WEATHER: appWeather.handleTouch(ev); break;
                    case AppID::WORLD_INFO: appWorldInfo.handleTouch(ev); break;
                    case AppID::SERVER: appServer.handleTouch(ev); break;
                    default: break;
                }
            }
        }
    }
    
    // 5. Update logic
    if (g_menu.isActive()) {
        g_menu.update();
    } else {
        switch (g_currentApp) {
            case AppID::FACE: g_face.update(); break;
            case AppID::CLOCK: appClock.update(); break;
            case AppID::WEATHER: appWeather.update(); break;
            case AppID::WORLD_INFO: appWorldInfo.update(); break;
            case AppID::SERVER: appServer.update(); break;
            case AppID::GAMES: appGames.update(); break;
            case AppID::SETTINGS: appSettings.update(); break;
            default: break;
        }
    }
    
    // 6. Render
    if (g_display.isTransitioning()) {
        // Transition handles its own rendering blending the from-buffer with the current drawn buffer
        // So we still need to draw the current frame beneath it
    }
    
    if (g_menu.isActive()) {
        g_menu.render();
    } else {
        switch (g_currentApp) {
            case AppID::FACE: g_face.render(); break;
            case AppID::CLOCK: appClock.render(); break;
            case AppID::WEATHER: appWeather.render(); break;
            case AppID::WORLD_INFO: appWorldInfo.render(); break;
            case AppID::SERVER: appServer.render(); break;
            case AppID::GAMES: appGames.render(); break;
            case AppID::SETTINGS: appSettings.render(); break;
            default: break;
        }
    }
    
    // (display.show() is called by endFrame() on the next loop iteration)
}
