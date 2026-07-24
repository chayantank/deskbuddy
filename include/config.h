#pragma once
// ============================================================================
// DeskBuddy — Hardware & Software Configuration
// ============================================================================

// --- Hardware Pins ---
#define PIN_SDA         8       // OLED I2C Data
#define PIN_SCL         9       // OLED I2C Clock
#define PIN_TOUCH       7       // TTP223 Capacitive Touch Sensor (digital)

// --- Display ---
#define SCREEN_W        128
#define SCREEN_H        64
#define OLED_ADDR       0x3C
#define OLED_RESET      -1      // No reset pin
#define TARGET_FPS      30
#define FRAME_TIME_MS   (1000 / TARGET_FPS)

// --- Touch Timing (ms) ---
#define TOUCH_DEBOUNCE_MS       30
#define TOUCH_TAP_MAX_MS        250
#define TOUCH_DOUBLE_GAP_MS     250
#define TOUCH_LONG_PRESS_MS     650
#define TOUCH_HOLD_MS           1800
#define TOUCH_DEEP_HOLD_MS      4500

// --- WiFi AP (Captive Portal) ---
#define AP_SSID         "DeskBuddy-Setup"
#define AP_PASS         "buddy1234"
#define AP_CHANNEL      1

// --- NTP ---
#define NTP_SERVER_1    "pool.ntp.org"
#define NTP_SERVER_2    "time.nist.gov"
#define NTP_SYNC_INTERVAL_MS    3600000UL   // 1 hour

// --- APIs (all free, no key required) ---
#define API_WEATHER         "https://api.open-meteo.com/v1/forecast"
#define API_QUOTES          "https://zenquotes.io/api/random"
#define API_FACTS           "https://uselessfacts.jsph.pl/api/v2/facts/random?language=en"
#define API_GOLD            "https://api.binance.com/api/v3/ticker/price?symbol=PAXGUSDT"
#define API_NEWS_SPORTS     "http://inshortsapi.vercel.app/news?category=sports"
#define API_NEWS_INDIA      "http://inshortsapi.vercel.app/news?category=national"
#define API_NEWS_TECH       "http://inshortsapi.vercel.app/news?category=technology"
#define API_CRYPTO_BTC      "https://api.binance.com/api/v3/ticker/price?symbol=BTCUSDT"
#define API_CRYPTO_ETH      "https://api.binance.com/api/v3/ticker/price?symbol=ETHUSDT"

// --- API Refresh Intervals (ms) ---
#define WEATHER_REFRESH_MS      900000UL    // 15 min
#define INFO_REFRESH_MS         300000UL    // 5 min

// --- Face Animation ---
#define BLINK_MIN_INTERVAL_MS   2000
#define BLINK_MAX_INTERVAL_MS   6000
#define BLINK_CLOSE_MS          80
#define BLINK_OPEN_MS           150
#define EXPRESSION_TRANSITION_MS 300
#define IDLE_LOOK_INTERVAL_MS   3000
#define SLEEPY_TIMEOUT_MS       120000UL    // 2 min
#define SLEEP_TIMEOUT_MS        300000UL    // 5 min

#if __has_include("local_config.h")
#include "local_config.h"
#endif

// --- Pi Server Monitor (Local Network Companion) ---
#ifndef SERVER_MONITOR_URL
#define SERVER_MONITOR_URL      "http://192.168.x.x:34001/api/v1.0/get-history?n=1"
#endif

#ifndef SERVER_DISPLAY_NAME
#define SERVER_DISPLAY_NAME     "PI SERVER"
#endif

// --- Menu ---
#define MENU_ITEM_COUNT         7
#define MENU_ICON_SIZE          24
#define MENU_SCROLL_SPEED       3

// --- NVS Keys ---
#define NVS_NAMESPACE           "deskbuddy"
#define NVS_KEY_WIFI_SSID       "wifi_ssid"
#define NVS_KEY_WIFI_PASS       "wifi_pass"
#define NVS_KEY_TIMEZONE        "timezone"
#define NVS_KEY_LATITUDE        "latitude"
#define NVS_KEY_LONGITUDE       "longitude"
#define NVS_KEY_CITY_NAME       "city_name"
#define NVS_KEY_BRIGHTNESS      "brightness"
#define NVS_KEY_TIME_FMT        "time_fmt"
#define NVS_KEY_DISPLAY_FLIP    "disp_flip"
#define NVS_KEY_SLEEP_TIMEOUT   "sleep_to"
#define NVS_KEY_CLOCK_MODE      "clock_mode"
#define NVS_KEY_EYE_STYLE       "eye_style"
#define NVS_KEY_HAPPINESS       "happiness"
#define NVS_KEY_DINO_HISCORE    "dino_hi"
#define NVS_KEY_FLAPPY_HISCORE  "flappy_hi"
#define NVS_KEY_STACK_HISCORE   "stack_hi"
#define NVS_KEY_REACT_BEST      "react_best"
#define NVS_KEY_CONFIGURED      "configured"

// --- Firmware Info ---
#define FW_NAME         "DeskBuddy"
#define FW_VERSION      "1.2.0"
#define FW_AUTHOR       "Open Source"
