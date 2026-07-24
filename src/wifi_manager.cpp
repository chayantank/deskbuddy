// ============================================================================
// DeskBuddy — WiFi Manager Implementation
// ============================================================================
#include "wifi_manager.h"
#include <WiFi.h>
#include <WebServer.h>
#include <DNSServer.h>
#include "icons.h"

static WebServer server(80);
static DNSServer dnsServer;
static const byte DNS_PORT = 53;

// --- Captive Portal HTML (PROGMEM) ---
static const char PORTAL_HTML[] PROGMEM = R"rawliteral(
<!DOCTYPE html><html><head><meta name="viewport" content="width=device-width, initial-scale=1">
<title>DeskBuddy Setup</title>
<style>
body { font-family: sans-serif; background: #121212; color: #fff; display: flex; flex-direction: column; align-items: center; padding: 20px; }
h2 { margin-bottom: 20px; }
form { background: #1e1e1e; padding: 20px; border-radius: 10px; width: 100%; max-width: 300px; box-shadow: 0 4px 6px rgba(0,0,0,0.3); }
input { width: 100%; padding: 10px; margin: 10px 0; border: 1px solid #333; border-radius: 5px; background: #2a2a2a; color: #fff; box-sizing: border-box; }
input[type=submit] { background: #4CAF50; color: white; border: none; font-weight: bold; cursor: pointer; }
.note { font-size: 12px; color: #aaa; text-align: center; margin-top: 10px; }
</style></head><body>
<h2>🤖 DeskBuddy Setup</h2>
<form action="/save" method="post">
  <label>WiFi SSID</label>
  <input type="text" name="ssid" placeholder="Network Name" required>
  <label>WiFi Password</label>
  <input type="password" name="pass" placeholder="Password">
  <label>Your City (For Weather)</label>
  <input type="text" name="city" placeholder="e.g. New Delhi">
  <label>Timezone Offset (hours)</label>
  <input type="number" step="0.5" name="tz" placeholder="e.g. 5.5 for IST" value="5.5">
  <input type="submit" value="Save & Connect">
</form>
<div class="note">DeskBuddy will restart after saving.</div>
</body></html>
)rawliteral";

// --- End HTML ---

// Global pointer for web server callbacks
static Storage* g_storage = nullptr;
static bool g_shouldRestart = false;

void WifiManager::begin(DisplayManager* dm, Storage* storage) {
    _dm = dm;
    _storage = storage;
    g_storage = storage;
    _state = WifiState::IDLE;
    WiFi.mode(WIFI_STA);
    WiFi.disconnect();
}

void WifiManager::autoConnect() {
    String ssid = _storage->getWifiSSID();
    String pass = _storage->getWifiPass();
    
    if (ssid.length() == 0) {
        startPortal();
        return;
    }
    
    _state = WifiState::CONNECTING;
    _connectStartTime = millis();
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid.c_str(), pass.length() > 0 ? pass.c_str() : NULL);
    
    _drawConnectingScreen();
}

void WifiManager::startPortal() {
    _state = WifiState::PORTAL_ACTIVE;
    _startAP();
}

void WifiManager::_startAP() {
    WiFi.mode(WIFI_AP);
    WiFi.softAP(AP_SSID, AP_PASS, AP_CHANNEL);
    
    // Setup DNS to redirect all requests to ESP IP
    dnsServer.setErrorReplyCode(DNSReplyCode::NoError);
    dnsServer.start(DNS_PORT, "*", WiFi.softAPIP());
    
    _setupWebRoutes();
    server.begin();
    
    _drawPortalScreen();
}

void WifiManager::_setupWebRoutes() {
    server.on("/", HTTP_GET, []() {
        server.send_P(200, "text/html", PORTAL_HTML);
    });
    
    server.on("/save", HTTP_POST, []() {
        String ssid = server.arg("ssid");
        String pass = server.arg("pass");
        String city = server.arg("city"); // In real app, would use geocoding API to convert to lat/lon
        String tz = server.arg("tz");
        
        if (ssid.length() > 0 && g_storage) {
            g_storage->setWifi(ssid, pass);
            if (tz.length() > 0) {
                float tzHours = tz.toFloat();
                g_storage->setTimezoneOffset((int32_t)(tzHours * 3600));
            }
            
            float lat = 28.6139f, lon = 77.2090f; // New Delhi default
            if (city.length() > 0) {
                Storage::geocodeCity(city, lat, lon);
                g_storage->setLocation(lat, lon, city);
            } else {
                g_storage->setLocation(lat, lon, "New Delhi");
            }
            
            g_storage->setConfigured(true);
            
            server.send(200, "text/html", "<html><body style='background:#121212;color:#fff;text-align:center;font-family:sans-serif;margin-top:50px;'><h2>Saved!</h2><p>DeskBuddy is restarting...</p></body></html>");
            g_shouldRestart = true;
        } else {
            server.send(400, "text/plain", "Missing SSID");
        }
    });
    
    // Captive portal redirects
    server.onNotFound([]() {
        server.sendHeader("Location", String("http://") + WiFi.softAPIP().toString(), true);
        server.send(302, "text/plain", "");
    });
}

void WifiManager::update() {
    if (g_shouldRestart) {
        delay(1000);
        ESP.restart();
    }
    
    if (_state == WifiState::PORTAL_ACTIVE) {
        dnsServer.processNextRequest();
        server.handleClient();
        
        // Redraw portal screen occasionally to show it's active
        static unsigned long lastDraw = 0;
        if (millis() - lastDraw > 1000) {
            _drawPortalScreen();
            lastDraw = millis();
        }
    } 
    else if (_state == WifiState::CONNECTING) {
        if (WiFi.status() == WL_CONNECTED) {
            _state = WifiState::CONNECTED;
            Serial.println("WiFi Connected!");
        } else if (millis() - _connectStartTime > 15000) {
            // Timeout after 15s
            _state = WifiState::FAILED;
            Serial.println("WiFi Connect Failed");
        } else {
            _drawConnectingScreen();
        }
    }
}

void WifiManager::disconnect() {
    WiFi.disconnect();
    _state = WifiState::IDLE;
}

String WifiManager::getIP() const {
    if (_state == WifiState::CONNECTED) {
        return WiFi.localIP().toString();
    } else if (_state == WifiState::PORTAL_ACTIVE) {
        return WiFi.softAPIP().toString();
    }
    return "0.0.0.0";
}

void WifiManager::_drawConnectingScreen() {
    if (!_dm) return;
    _dm->beginFrame();
    Adafruit_SSD1306& d = _dm->display();
    
    d.drawBitmap(SCREEN_W/2 - 12, 10, icon_settings_24, 24, 24, SSD1306_WHITE);
    _dm->drawCenteredText("Connecting...", 40, 1);
    
    // simple animation
    int dots = (millis() / 300) % 4;
    String txt = "";
    for(int i=0; i<dots; i++) txt += ".";
    d.setCursor(SCREEN_W/2 + 35, 40);
    d.print(txt);
    
    _dm->show(); // Force show without FPS limit
}

void WifiManager::_drawPortalScreen() {
    if (!_dm) return;
    _dm->beginFrame();
    Adafruit_SSD1306& d = _dm->display();
    
    d.drawBitmap(SCREEN_W/2 - 12, 2, sym_wifi_8, 8, 8, SSD1306_WHITE); // Should use a proper 24x24 wifi icon, reusing settings for now
    _dm->drawCenteredText("Setup WiFi", 2, 1);
    
    d.drawLine(0, 12, SCREEN_W, 12, SSD1306_WHITE);
    
    _dm->drawCenteredText("Connect to AP:", 20, 1);
    _dm->drawCenteredText(AP_SSID, 32, 1);
    
    _dm->drawCenteredText("Password:", 44, 1);
    _dm->drawCenteredText(AP_PASS, 54, 1);
    
    _dm->show();
}
