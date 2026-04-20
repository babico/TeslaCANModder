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
