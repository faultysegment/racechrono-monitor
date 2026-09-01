# RaceChrono Monitor

An **AI-Assisted**, high-performance, modular telemetry monitor and multi-screen display for [RaceChrono](https://racechrono.com/) over Bluetooth Low Energy (BLE), powered by ESP32-S3 and PlatformIO.

---

## Overview

**RaceChrono Monitor** acts as a standalone digital dashboard and lap timer display for motorsport track sessions. It connects directly to the RaceChrono app over BLE as a custom DIY device, receiving real-time live telemetry (such as Delta Lap Time, Delta Speed, Trap Speeds, or custom-calculated channels) and rendering them with sub-frame latency on high-visibility displays.

---

## Key Features

- **⚡ Real-Time Low-Latency BLE Telemetry**  
  Communicates over Bluetooth Low Energy with custom RaceChrono notification channels. Automatic reconnect, session-state synchronization, and keepalive management.

- **📐 Flexible RaceChrono Formulas & Multi-Channel Support**  
  Define arbitrary metrics using RaceChrono formula expressions (e.g., `channel(device(lap), delta_lap_time)*100.0` or `channel(device(calc), delta_speed)*100`), customizable multipliers, decimal precision, and polarity («positive is good/bad»).

- **🖥️ Multiple Screen Layouts & Form Factors**  
  - **Single Monitor Screen**: High-contrast, large numerical readout with a dynamic, color-coded proportional delta bar.
  - **Dual Monitor Screen**: Split screen presenting two metrics simultaneously (e.g., Lap Delta on top, Speed Delta on bottom).
  - **Circular AMOLED UI**: Dedicated radial sweep gauge and ring visualization designed for circular displays.
  - **HUD Mode (Head-Up Display)**: Built-in horizontal and vertical mirroring/inversion for windshield projection reflection.

- **🌐 Embedded Web UI Configurator**  
  Built-in Wi-Fi Access Point (`RaceChrono-Monitor`) hosting a lightweight Single Page Application (SPA). Configure monitor formulas, screen assignments, color palettes, and limits on the fly from any smartphone or browser without flashing.

- **💾 Persistent Flash Storage**  
  Configurations and screen preferences are persisted to non-volatile Flash storage (NVS / JSON schema) and reloaded automatically on startup.

- **🧪 100% Host-Native Unit Testing**  
  Ultra-fast desktop test execution with Unity and mock hardware policies—verify core math, state transitions, and screen rendering in seconds.

---

## Supported Hardware Targets

| Target Environment | Device | Display | Input & Peripherals |
| :--- | :--- | :--- | :--- |
| `T_Embed_CC1101` | **LilyGO T-Embed CC1101** (ESP32-S3) | 1.9" IPS TFT (170x320, ST7789 via `TFT_eSPI`) | Rotary Encoder, Action Button, AXP2101 PMU / Battery Voltage |
| `T_Display_S3_AMOLED` | **LilyGO T-Display-S3 AMOLED** (ESP32-S3) | 1.75" Circular AMOLED (466x466, SH8601/CO5300 QSPI via `Arduino_GFX`) | Hardware Buttons, Touch, SD Card over HSPI |

---

## Architecture & Design

The project is built on a strictly decoupled **Event-Driven Actor Architecture** using FreeRTOS tasks and C++ template policy injection:

```
                      +-------------------+
                      |   RaceChrono BLE  |
                      +---------+---------+
                                |
                                v
+-------------+       +-------------------+       +-------------+
| Input Task  | ----> |     EventBus      | <---> | Logic Task  |
+-------------+       +---------+---------+       +------+------+
                                |                        |
                                v                        v
+-------------+       +-------------------+       +-------------+
|  Web Task   | ----> |     UI Task       |       |  AppState   |
+-------------+       | (View & Screens)  |       +-------------+
                      +-------------------+
```

- **`AppState` (`src/AppState.h`)**: Pure UI and telemetry state container. Strictly managed by the event processing thread.
- **`EventBus` (`src/EventBus.h`)**: Thread-safe queue (`std::queue`, `std::mutex`, `std::condition_variable`) distributing hardware, BLE, and web configuration events.
- **`AppLogic` (`src/AppLogic.h`)**: Device-agnostic business logic processing BLE packet streams, managing configuration state, and handling user inputs.
- **`View` & Screens (`src/View.h`, `src/Screens/`)**: Rendering layer using injected `DisplayPolicy` and polymorphic `IScreen` implementations.
- **FreeRTOS Task Distribution**:
  - **Core 1 (App Core)**: `uiTask` dedicated to SPI/QSPI frame rendering for maximum smoothness.
  - **Core 0 (Pro Core)**: `logicTask`, `inputTask`, and `webTask` (handling BLE, Wi-Fi AP, and HTTP server).

---

## Configuration & Web UI

### Web UI Access
1. Power on the device.
2. If enabled or in configuration mode, connect to the Wi-Fi network:
   - **SSID:** `RaceChrono-Monitor` (or configured SSID)
   - **Password:** *(None by default)*
3. Navigate to `http://192.168.4.1` in your mobile or desktop browser.
4. Customize monitors, screen layouts, colors, and HUD settings live, then click **Save**.

### Configuration Schema (`config.json`)
```json
{
  "isHud": false,
  "webui": {
    "enabled": false,
    "ssid": "RaceChrono-Monitor",
    "password": ""
  },
  "monitors": [
    {
      "id": "lap_delta",
      "title": "TIME",
      "formula": "channel(device(lap), delta_lap_time)*100.0",
      "multiplier": 0.01,
      "decimals": 2,
      "limit": 0.3
    },
    {
      "id": "speed_delta",
      "title": "SPEED",
      "formula": "channel(device(calc), delta_speed)*100",
      "multiplier": 0.036,
      "decimals": 1,
      "limit": 1.0
    }
  ],
  "screens": [
    {
      "type": "single",
      "monitor": "lap_delta",
      "positive_color": "#FF0000",
      "negative_color": "#00FF00",
      "title_color": "#0000FF",
      "value_color": "#0000FF"
    },
    {
      "type": "dual",
      "top": {
        "monitor": "lap_delta",
        "positive_color": "#FF0000",
        "negative_color": "#00FF00"
      },
      "bottom": {
        "monitor": "speed_delta",
        "positive_color": "#00FFFF",
        "negative_color": "#FFA500"
      }
    }
  ]
}
```

---

## How to Build and Flash

### Prerequisites
- [PlatformIO Core](https://platformio.org/install/cli) or [PlatformIO IDE for VSCode](https://platformio.org/platformio-ide).

### 1. LilyGO T-Embed CC1101
```bash
# Build firmware
pio run -e T_Embed_CC1101

# Build and upload
pio run -e T_Embed_CC1101 -t upload

# Serial monitor
pio device monitor -e T_Embed_CC1101
```

### 2. LilyGO T-Display-S3 AMOLED (Round 1.75")
```bash
# Build firmware
pio run -e T_Display_S3_AMOLED

# Build and upload
pio run -e T_Display_S3_AMOLED -t upload

# Serial monitor
pio device monitor -e T_Display_S3_AMOLED
```

---

## Running Unit Tests (`unit_tests`)

Run the full suite of native host unit tests (validating `AppState`, `AppLogic`, JSON configuration parser, screen composition, and color math without physical hardware):

```bash
pio test -e unit_tests
```

All tests execute natively on your host machine via Unity in a matter of seconds.

---

## AI-Assisted Project

> [!NOTE]  
> This project is developed with **AI Assistance**. While architectural design, hardware validation, and core performance optimizations are guided by the developer, AI pair programming was utilized for iterative implementation, test suite expansion, and code refactoring.

