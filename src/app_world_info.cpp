// ============================================================================
// DeskBuddy — World Info App Implementation
// ============================================================================
#include "app_world_info.h"
#include "config.h"
#include <WiFi.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>
#include "icons.h"

void AppWorldInfo::begin(DisplayManager* dm, Storage* storage) {
    _dm = dm;
    _storage = storage;
}

void AppWorldInfo::onEnter() {
    _scrollX = 0;
    
    // Fetch if empty or old
    if (_infoText.length() == 0 || millis() - _lastFetchTime[(int)_type] > INFO_REFRESH_MS) {
        _fetchData(_type);
    }
}

void AppWorldInfo::onExit() {
}

void AppWorldInfo::update() {
    // Scroll update handled in render
}

void AppWorldInfo::handleTouch(TouchEvent event) {
    if (event == TouchEvent::TAP) {
        _type = (InfoType)(((int)_type + 1) % (int)InfoType::COUNT);
        _scrollX = 0;
        
        if (_infoText.length() == 0 || millis() - _lastFetchTime[(int)_type] > INFO_REFRESH_MS) {
            _fetchData(_type);
        }
    } else if (event == TouchEvent::LONG_PRESS) {
        _fetchData(_type); // force refresh
    }
}

void AppWorldInfo::render() {
    if (!_dm) return;
    Adafruit_SSD1306& d = _dm->display();
    
    d.setTextSize(1);
    
    // Header
    const char* header = "";
    switch (_type) {
        case InfoType::GOLD:       header = "GOLD PRICE (24K)"; break;
        case InfoType::SPORTS:     header = "INDIA SPORTS"; break;
        case InfoType::INDIA_NEWS: header = "INDIA NEWS"; break;
        case InfoType::TECH:       header = "TECH HEADLINES"; break;
        case InfoType::CRYPTO:     header = "CRYPTO (BTC/ETH)"; break;
        case InfoType::QUOTE:      header = "DAILY QUOTE"; break;
        case InfoType::FACT:       header = "RANDOM FACT"; break;
        default: break;
    }
    
    _dm->drawCenteredText(header, 2, 1);
    d.drawLine(0, 12, SCREEN_W, 12, SSD1306_WHITE);
    
    if (_isFetching) {
        _dm->drawCenteredText("Loading...", 32, 1);
        return;
    }
    
    if (_infoText.length() == 0) {
        if (WiFi.status() != WL_CONNECTED) {
            _dm->drawCenteredText("WiFi Disconnected", 32, 1);
        } else {
            _dm->drawCenteredText("Fetch failed", 28, 1);
            _dm->drawCenteredText("Long press to retry", 40, 1);
        }
        return;
    }
    
    // Content rendering based on type
    if (_type == InfoType::GOLD || _type == InfoType::CRYPTO) {
        d.setTextSize(1);
        _dm->drawCenteredText(_infoText.c_str(), 24, 1);
        if (_authorText.length() > 0) {
            _dm->drawCenteredText(_authorText.c_str(), 42, 1);
        }
    } else {
        // Multi-line word wrap if short, scroll if long
        d.setTextSize(1);
        _dm->drawScrollingText(_infoText.c_str(), 24, _scrollX, 1);
        
        if (_authorText.length() > 0) {
            _dm->drawCenteredText(_authorText.c_str(), 44, 1);
        }
    }
    
    // Page indicators (7 dots centered)
    int total = (int)InfoType::COUNT;
    int dotSpacing = 6;
    int startX = SCREEN_W / 2 - ((total - 1) * dotSpacing) / 2;
    for (int i = 0; i < total; i++) {
        int x = startX + i * dotSpacing;
        if (i == (int)_type) {
            d.fillCircle(x, 59, 2, SSD1306_WHITE);
        } else {
            d.drawPixel(x, 59, SSD1306_WHITE);
        }
    }
}

void AppWorldInfo::_fetchData(InfoType type) {
    if (WiFi.status() != WL_CONNECTED) return;
    
    _isFetching = true;
    if (_dm) {
        _dm->beginFrame();
        _dm->drawCenteredText("Loading...", 32, 1);
        _dm->show();
    }
    
    switch (type) {
        case InfoType::GOLD:       _fetchGold(); break;
        case InfoType::SPORTS:     _fetchSports(); break;
        case InfoType::INDIA_NEWS: _fetchIndiaNews(); break;
        case InfoType::TECH:       _fetchTechNews(); break;
        case InfoType::CRYPTO:     _fetchCrypto(); break;
        case InfoType::QUOTE:      _fetchQuote(); break;
        case InfoType::FACT:       _fetchFact(); break;
        default: break;
    }
    
    _lastFetchTime[(int)type] = millis();
    _isFetching = false;
    _scrollX = 0;
}

void AppWorldInfo::_fetchGold() {
    WiFiClientSecure client;
    client.setInsecure();
    HTTPClient http;
    http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);
    http.setTimeout(7000);
    
    if (http.begin(client, API_GOLD)) {
        int code = http.GET();
        if (code == HTTP_CODE_OK) {
            String payload = http.getString();
            JsonDocument doc;
            DeserializationError err = deserializeJson(doc, payload);
            if (!err) {
                float priceOzUsd = doc["price"]; // price per oz USD
                if (priceOzUsd > 0) {
                    float price10gUsd = (priceOzUsd / 31.1035f) * 10.0f;
                    float usdInr = 83.5f;
                    int price10gInr = (int)round(price10gUsd * usdInr);
                    
                    char buf1[32], buf2[32];
                    snprintf(buf1, sizeof(buf1), "Rs %d / 10g", price10gInr);
                    snprintf(buf2, sizeof(buf2), "$%d / troy oz", (int)round(priceOzUsd));
                    
                    _infoText = buf1;
                    _authorText = buf2;
                }
            }
        }
        http.end();
    }
}

void AppWorldInfo::_fetchSports() {
    HTTPClient http;
    http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);
    http.setTimeout(7000);
    
    if (http.begin(API_NEWS_SPORTS)) {
        int code = http.GET();
        if (code == HTTP_CODE_OK) {
            String payload = http.getString();
            JsonDocument doc;
            DeserializationError err = deserializeJson(doc, payload);
            if (!err && doc["data"].is<JsonArray>() && doc["data"].size() > 0) {
                const char* title = doc["data"][0]["title"];
                if (title) {
                    _infoText = title;
                    _authorText = "India Sports";
                }
            }
        }
        http.end();
    }
}

void AppWorldInfo::_fetchIndiaNews() {
    HTTPClient http;
    http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);
    http.setTimeout(7000);
    
    if (http.begin(API_NEWS_INDIA)) {
        int code = http.GET();
        if (code == HTTP_CODE_OK) {
            String payload = http.getString();
            JsonDocument doc;
            DeserializationError err = deserializeJson(doc, payload);
            if (!err && doc["data"].is<JsonArray>() && doc["data"].size() > 0) {
                const char* title = doc["data"][0]["title"];
                if (title) {
                    _infoText = title;
                    _authorText = "India Headlines";
                }
            }
        }
        http.end();
    }
}

void AppWorldInfo::_fetchTechNews() {
    HTTPClient http;
    http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);
    http.setTimeout(7000);
    
    if (http.begin(API_NEWS_TECH)) {
        int code = http.GET();
        if (code == HTTP_CODE_OK) {
            String payload = http.getString();
            JsonDocument doc;
            DeserializationError err = deserializeJson(doc, payload);
            if (!err && doc["data"].is<JsonArray>() && doc["data"].size() > 0) {
                const char* title = doc["data"][0]["title"];
                if (title) {
                    _infoText = title;
                    _authorText = "Tech News";
                }
            }
        }
        http.end();
    }
}

void AppWorldInfo::_fetchCrypto() {
    WiFiClientSecure client;
    client.setInsecure();
    HTTPClient http;
    http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);
    http.setTimeout(7000);
    
    float btcPrice = 0, ethPrice = 0;
    
    if (http.begin(client, API_CRYPTO_BTC)) {
        int code = http.GET();
        if (code == HTTP_CODE_OK) {
            String payload = http.getString();
            JsonDocument doc;
            if (!deserializeJson(doc, payload)) {
                btcPrice = doc["price"].as<float>();
            }
        }
        http.end();
    }
    
    if (http.begin(client, API_CRYPTO_ETH)) {
        int code = http.GET();
        if (code == HTTP_CODE_OK) {
            String payload = http.getString();
            JsonDocument doc;
            if (!deserializeJson(doc, payload)) {
                ethPrice = doc["price"].as<float>();
            }
        }
        http.end();
    }
    
    if (btcPrice > 0 || ethPrice > 0) {
        char buf1[32], buf2[32];
        snprintf(buf1, sizeof(buf1), "BTC: $%d", (int)round(btcPrice));
        snprintf(buf2, sizeof(buf2), "ETH: $%d", (int)round(ethPrice));
        _infoText = buf1;
        _authorText = buf2;
    }
}

void AppWorldInfo::_fetchQuote() {
    WiFiClientSecure client;
    client.setInsecure();
    HTTPClient http;
    http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);
    http.setTimeout(7000);
    
    if (http.begin(client, API_QUOTES)) {
        int code = http.GET();
        if (code == HTTP_CODE_OK) {
            String payload = http.getString();
            JsonDocument doc;
            DeserializationError err = deserializeJson(doc, payload);
            if (!err && doc.is<JsonArray>() && doc.size() > 0) {
                const char* q = doc[0]["q"];
                const char* a = doc[0]["a"];
                if (q) _infoText = String("\"") + q + "\"";
                if (a) _authorText = String("- ") + a;
            }
        }
        http.end();
    }
}

void AppWorldInfo::_fetchFact() {
    WiFiClientSecure client;
    client.setInsecure();
    HTTPClient http;
    http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);
    http.setTimeout(7000);
    
    if (http.begin(client, API_FACTS)) {
        int code = http.GET();
        if (code == HTTP_CODE_OK) {
            String payload = http.getString();
            JsonDocument doc;
            DeserializationError err = deserializeJson(doc, payload);
            if (!err) {
                const char* text = doc["text"];
                if (text) {
                    _infoText = text;
                    _authorText = "Random Fact";
                }
            }
        }
        http.end();
    }
}
