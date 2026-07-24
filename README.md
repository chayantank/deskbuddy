# DeskBuddy — Open Source ESP32-C3 Desk Companion (v1.2.0)

DeskBuddy is a fully open-source, ultra-fluid customizable firmware for the ESP32-C3 Super Mini paired with a 0.96" SSD1306 OLED display and a TTP223 capacitive touch sensor. It serves as a dynamic, interactive desktop companion competing with commercial companions (Vector/Emo/Desktop Buddy), featuring 19 procedurally animated expressions, eye style customizations (Classic Oval, Vector Square, Emo Neon, Cyber Dot), a captive portal for easy WiFi setup with city geocoding, screen-wide jumbo clock faces, live weather data, India-specific world APIs (24K Gold Price, India Sports, Headlines, Crypto), and touch mini-games.

This project was built as a zero-latency, open-source alternative to closed-source desktop companions, providing superior animation fluidity, a robust UI, and zero reliance on paid API keys.

---

## ✨ What's New in v1.2.0

- 🤖 **Competition-Grade Pet Expressions & Custom Eye Styles**:
  - 19 procedurally animated expressions including `PARTY`, `CHILL`, `LOVE`, `ANNOYED`, `SKEPTICAL`, and `SURPRISED`.
  - **4 Selectable Eye Styles**: `Classic Oval`, `Vector Square` (Anki Vector style), `Neon Outline` (Emo style), and `Cyber Dot` (Matrix style), switchable live in Settings.
  - Micro-animations: Soft breathing idle pulsation and dynamic eyebrow physics.
- 🕒 **Screen-Wide Jumbo Digital Clock Mode**:
  - Full-width jumbo time display using `ClockFont24pt7b` font with animated blinking colon, live day/date header, and smooth seconds progress track.
  - NVS storage persistence for preferred clock face.
  - **Pomodoro Timer (25 min)** & 7 timer presets (1m, 3m, 5m, 10m, 15m, 25m, 60m).
- 📰 **India-Specific & Global Live Data Feeds**:
  - **24K Gold Price**: Live rate in **₹/10g** (INR) & **$/oz** (USD).
  - **India Sports News**: Live Cricket & Sports headlines with smooth marquee scrolling.
  - **India National News**: Top headlines from India.
  - **Tech News**: Technology stories.
  - **Crypto Live Rates**: Bitcoin (BTC) & Ethereum (ETH) prices.
  - **Quotes & Facts**: Inspirational quotes & random facts.
- 📍 **City Location Geocoding**: Enter any city name in the WiFi setup portal (e.g. *Delhi, Mumbai, Bangalore, London, NYC, Tokyo*) to automatically geocode weather coordinates.

---

## Features

- **Procedural Face Engine:** 19 fluidly animated expressions. No bitmap bloat, resulting in tiny flash footprint and smooth 30+ FPS animation.
- **Single-Touch Navigation:** Full menu and OS navigation using a single TTP223 capacitive touch sensor (Tap, Double-Tap, Long-Press).
- **Captive Portal Setup:** Broadcasts `DeskBuddy-Setup` AP on first boot for browser configuration of WiFi credentials and city location.
- **5 Clock Modes:** Screen-Wide Digital, Analog Watch, Binary BCD, Millisecond Stopwatch, and Countdown/Pomodoro Timer.
- **Live Weather:** Free integration with Open-Meteo for current conditions and 3-day forecasting.
- **World Info App:** 7 interactive categories with horizontal auto-scrolling news headlines and page indicators.
- **4 Mini-Games:** Dino Runner, Pong, Simon Says, and Reaction Time with NVS flash high scores.
- **Persistent Settings:** Adjust brightness, 12/24h format, screen flipping, eye styles, and sleep timeouts.

---

## Hardware BOM

1. **ESP32-C3 Super Mini:** (Or equivalent ESP32-C3 board)
2. **0.96" I2C OLED Display:** SSD1306 (128x64 resolution, 0x3C address)
3. **TTP223 Capacitive Touch Sensor:** Single digital pin touch input
4. **Enclosure:** Any 3D printed desktop robot case

### Wiring

| ESP32-C3 Pin | Component Pin | Function |
|---|---|---|
| 3.3V | VCC (OLED & Touch) | 3.3V Power |
| GND | GND (OLED & Touch) | Ground |
| GPIO 8 | SDA (OLED) | I2C Data |
| GPIO 9 | SCL (OLED) | I2C Clock |
| GPIO 7 | SIG/OUT (Touch) | Digital Input |

---

## Building & Flashing

This project is built using **PlatformIO**.

1. Clone this repository:
   ```bash
   git clone https://github.com/chayantank/deskbuddy.git
   cd deskbuddy
   ```
2. Open in VS Code with PlatformIO extension installed.
3. Build & Upload:
   ```bash
   pio run -t upload
   ```

---

## Usage & Touch Gestures

- **Tap (Short Touch):** Cycle items, switch categories, or jump in games.
- **Double-Tap:** Back button. Instantly returns to the main pet face from any app.
- **Long Press (~1 sec):** Select / Action (e.g. force refresh API data or start timer).
- **Deep Hold (>2 sec):** Advanced menu actions or Wi-Fi reset in Settings.

---

## License

Distributed under the MIT License. See [LICENSE](LICENSE) for details.
