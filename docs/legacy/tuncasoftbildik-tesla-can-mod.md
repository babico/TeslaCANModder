---
title: tuncasoftbildik-tesla-can-mod
description: A feature-rich Tesla CAN bus modification firmware for the Waveshare ESP32-C6-LCD-1.47 board. Beyond FSD activation, it 
category: legacy
folder: legacy
tags: [legacy, community, external]
author: tuncasoftbildik
repo: tesla-can-mod
---

# tuncasoftbildik-tesla-can-mod

## Overview

A feature-rich Tesla CAN bus modification firmware for the Waveshare ESP32-C6-LCD-1.47 board. Beyond FSD activation, it adds real-time battery monitoring (SoC, voltage, current, temperature), energy consumption tracking, battery preconditioning, a built-in 1.47" color LCD dashboard, and a WiFi web dashboard accessible at 192.168.4.1. Targets Model 3/Y with HW3/HW4 support.

## Architecture

```mermaid
flowchart TB
    Car["Tesla CAN (HW3/HW4)"] --> ESP["ESP32-C6<br/>(TWAI + SN65HVD230)"]
    ESP --> FSD["FSD activation"]
    ESP --> BMS["Battery monitor<br/>(SoC/V/I/T)"]
    ESP --> Pre["Battery preconditioning"]
    ESP --> LCD["1.47\" color LCD<br/>(built-in dashboard)"]
    ESP --> Web["WiFi web UI<br/>(192.168.4.1)"]
    classDef path fill:#1e3a5f,stroke:#4a7fb5,color:#fff
    class ESP,FSD,BMS,Pre,LCD,Web path
```

## Technical Details

- **Platform**: Waveshare ESP32-C6-LCD-1.47
- **Language**: C++ (PlatformIO/Arduino)
- **CAN Interface**: ESP32-C6 TWAI peripheral + SN65HVD230 transceiver
- **License**: MIT

## Architecture

PlatformIO project structured with clean separation of concerns:

- `src/main.cpp` — Entry point. Initializes CAN (TWAI driver), LCD display, handler (HW4Handler), and web server. Main loop reads CAN frames, updates handler, refreshes LCD, and serves web requests
- `include/handlers.h` — CAN message handler classes (LegacyHandler, HW3Handler, HW4Handler) with `Shared<T>` state, frame counting, and filter ID system
- `include/can_frame_types.h` — CAN frame structures
- `include/can_helpers.h` — Bit manipulation utilities
- `include/drivers/` — CAN driver abstractions (TWAIDriver)
- `include/lcd_display.h` — ST7789V LCD driver for the 172x320 display
- `include/web/` — WiFi AP and HTTP web dashboard server
- `include/log_buffer.h` — Ring buffer for event logging
- `include/shared_types.h` — Thread-safe shared state types
- `include/User_Setup.h` — TFT library configuration
- `TESLA_CAN_BATTERY_REFERENCE.md` — Comprehensive battery CAN signal reference
- `TESLA_CAN_STEERING_REFERENCE.md` — Steering CAN signal reference
- `platformio.ini` — Targets ESP32-C6 with pioarduino platform fork, ST7789 LCD libraries

## CAN Bus Integration

Handles FSD activation plus battery/energy monitoring at 500 kbps:

**FSD Control (via handlers):**

- CAN ID 1021 (AP_CONTROL): FSD enable (bit 46), V14 enable (bit 60), nag suppress (bit 19)
- CAN ID 1016 (AP_FOLLOW_DIST): Speed profile from follow-distance stalk

**Battery Monitoring:**

- CAN ID 306 (0x132, BMS_hvBusStatus): Pack voltage (bits 0-15, factor 0.01V), pack current (bits 16-30, factor -0.1A)
- CAN ID 658 (0x292, BMS_socStatus): SoC% (multiple signals — min/UI/max/avg, 10-bit each, factor 0.1)
- CAN ID 786 (0x312, BMS_thermalStatus): Battery temperature min/max
- CAN ID 826 (0x33A, UI_ratedConsumption): Energy consumption in Wh/km

**Battery Preconditioning:**

- CAN ID 130 (0x082, UI_tripPlanning): Trigger battery heating (Supercharger prep simulation)

## Relevance to Our Project

The most feature-complete single-board Tesla CAN mod, adding battery monitoring and a physical LCD dashboard on top of FSD activation. The battery CAN reference documents are especially valuable.

- **Reusability**: High
- **Key Takeaways**:
  - Battery monitoring CAN IDs and signal decoding (306, 658, 786, 826) with full bit-level documentation
  - Battery preconditioning trigger via CAN ID 130
  - LCD dashboard implementation on ESP32-C6 with ST7789V
  - WiFi AP web dashboard with live status, controls, and log viewer
  - ESP32-C6 TWAI + SN65HVD230 wiring reference (GPIO 0 TX, GPIO 1 RX)
  - `TESLA_CAN_BATTERY_REFERENCE.md` is an excellent standalone CAN signal reference including cell voltages, thermal data, and charging status
  - MIT license (more permissive than GPL-3.0 used by other FSD mods)
  - LM2596 DC-DC converter for 12V→5V power from the diagnostic port

### Recent Changes (v0.2.0, June 2026)

- **Flipper Zero companion app with UART bridge protocol** — New Flipper Zero app that connects to the ESP32-C6 via UART (GPIO 4 TX / GPIO 5 RX at 115200 baud) using a binary framing protocol with 0xAA start byte, length field, command byte, and XOR checksum. The Flipper provides a secondary display, button controls, and BLE relay for phone connectivity.
- **ISA speed multiplier** (`isaSpeedMul` 1–15 slider) — Multiplies the detected map speed limit (from 0x238 or GPS) by a configurable factor before injecting the ISA speed limit into the UI display. Range 1–15× in integer steps. Controlled via the web dashboard slider or the `isaspeedmul:N` serial command.
- **ISA dynamic speed offset** derived from CAN data — Unlike our static `offset:N` command (which applies a fixed offset), this dynamically adjusts the speed offset based on road class detection from 0x238 byte 1 (motorway vs. urban vs. rural), current vehicle speed from 0x257, and the detected speed limit sign type.
- **Python reference client** (`teslacan_client.py`) — A cross-platform Python client that connects via serial or WiFi and provides: full command-line interface, CSV logging of all CAN frames, real-time BMS graphing (matplotlib), automatic firmware version detection, and a REPL mode for interactive CAN exploration.
- **SVG battery SoC ring gauge and DNS captive portal in web dashboard** — The web dashboard now features an animated SVG ring gauge showing battery SoC with color gradient (green → yellow → red), plus a DNS captive portal that intercepts all HTTP requests on the AP for a seamless phone connection experience.
- **Connection health indicators** — The LCD and web dashboard now show per-second CAN frame rate, bus error count (TWAI error warning/bus-off states), UART bridge health (heartbeat and timeout), and WiFi client count, all with color-coded status indicators.
