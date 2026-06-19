# RaceChrono Monitor

An **AI Assisted** ESP32-based BLE monitor for RaceChrono, built using PlatformIO.

## Overview

This project acts as an external monitor for RaceChrono, fetching and displaying real-time telemetry like Delta Lap Time, Delta Speed, and other metrics over a Bluetooth Low Energy (BLE) connection. 

It features a TFT display, a rotary encoder interface for navigating menus and configuring limits, and relies on a clean, decoupled architecture.

## AI Assisted Project

> [!NOTE]  
> This project was developed with **AI Assistance**. While the core architecture and testing strategies were driven by the developer, AI tools were utilized to help write code, debug issues, refine test configurations, and assist with code review.

## Features

- **BLE Connectivity**: Connects to the RaceChrono app as an external BLE monitor.
- **Multiple Screens**: Dedicated dynamic views for Time, Speed, and Configuration modes.
- **Hardware Integration**: Designed for ESP32 with TFT display, rotary encoder, and battery monitoring.
- **Clean Architecture**: The core logic (`Controller`, `Model`, `View`) is fully decoupled from the hardware using template-based Policy Injection.
- **Native Unit Testing**: Over 100% native unit testing support using Unity and Mock policies.

## Build and Run

1. Install [PlatformIO](https://platformio.org/).
2. Build the firmware:
   ```bash
   pio run
   ```
3. Upload to the board:
   ```bash
   pio run -t upload
   ```

## Running Tests

To run native tests on your host machine (requires GCC/MinGW on Windows):

```bash
pio test -e native
```
