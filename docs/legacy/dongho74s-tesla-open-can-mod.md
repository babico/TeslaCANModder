---
title: dongho74s-tesla-open-can-mod
description: An open-source firmware for enabling Tesla FSD (Full Self-Driving) functionality at the CAN bus level. Runs on Adafruit 
category: legacy
folder: legacy
tags: [legacy, community, external]
author: dongho74s
repo: tesla-open-can-mod
---

# dongho74s-tesla-open-can-mod

## Overview

An open-source firmware for enabling Tesla FSD (Full Self-Driving) functionality at the CAN bus level. Runs on Adafruit Feather RP2040 CAN (MCP2515), Feather M4 CAN Express (ATSAME51), or ESP32 boards with TWAI. It intercepts and modifies specific CAN messages to activate FSD, suppress nag warnings, and control speed profiles based on follow-distance stalk settings. Supports HW3 (Legacy and Palladium) and HW4 vehicles.

## Technical Details

- **Platform**: Adafruit Feather RP2040 CAN, Feather M4 CAN Express, ESP32, M5Stack Atomic CAN Base
- **Language**: C++ (Arduino framework via PlatformIO)
- **CAN Interface**: MCP2515 (SPI), ATSAME51 native MCAN, ESP32 TWAI — all at 500 kbit/s
- **License**: GPL v3

## Architecture

- `src/main.cpp` / `RP2040CAN.ino` — Entry points (PlatformIO and Arduino IDE respectively)
- `include/app.h` — Template-based setup/loop dispatching to the correct CAN driver and handler
- `include/handlers.h` — Three handler classes (`LegacyHandler`, `HW3Handler`, `HW4Handler`) implementing CAN message interception and modification logic per hardware variant
- `include/can_helpers.h` — Bit manipulation utilities for CAN frames (setBit, readMuxID, isFSDSelectedInUI, setSpeedProfile)
- `include/can_frame_types.h` — Common CAN frame structure
- `include/drivers/can_driver.h` — Abstract CAN driver interface
- `include/drivers/mcp2515_driver.h` — MCP2515 driver (SPI)
- `include/drivers/same51_driver.h` — ATSAME51 native CAN driver
- `include/drivers/twai_driver.h` — ESP32 TWAI driver
- `include/drivers/mock_driver.h` — Mock driver for native unit tests
- `platformio.ini` — Build configurations for all supported boards + native test env
- `test/` — Unit tests
- `guides/` — Installation/wiring guides

## CAN Bus Integration

Deep CAN integration — this is a CAN frame interceptor/modifier:

**Legacy (HW3 Retrofit):**

- Reads CAN ID 69 (0x045, `STW_ACTN_RQ`) for follow-distance stalk position → speed profile mapping
- Reads + modifies CAN ID 1006 (0x3EE): mux 0 sets bit 46 (FSD enable), writes speed profile to byte 6 bits 1-2; mux 1 clears bit 19 (nag suppression)

**HW3:**

- Reads CAN ID 1016 (0x3F8, `UI_driverAssistControl`) for follow-distance setting
- Reads + modifies CAN ID 1021 (0x3FD, `UI_autopilotControl`): mux 0 sets bit 46 (FSD enable), speed profile; mux 1 clears bit 19 (nag); mux 2 writes speed offset

**HW4:**

- Reads CAN ID 1016 (0x3F8) for follow-distance (5 levels)
- Reads + modifies CAN ID 1021 (0x3FD): mux 0 sets bit 46 (FSD enable), bit 60 (FSD V14), bit 59 (emergency vehicle detection); mux 1 clears bit 19 (nag), sets bit 47; mux 2 writes speed profile to byte 7 bits 4-6
- Optionally reads + modifies CAN ID 921 (0x399, `DAS_status`) for ISA speed chime suppression

## Relevance to Our Project

Extremely relevant — this is a well-architected FSD CAN enabler firmware that is essentially a sister project to Tesla-CAN-Mod.

- **Reusability**: High
- **Key Takeaways**:
  - Clean driver abstraction supporting MCP2515, SAME51, and ESP32 TWAI from a single codebase
  - Template-based dispatch pattern for hardware-specific handlers (Legacy/HW3/HW4)
  - Detailed CAN message bit manipulation for FSD enable, nag suppression, and speed profile
  - PlatformIO multi-environment build system with native test target
  - Complete wiring guide for Model 3 Highland with Enhance Auto Gen 2 cable and X179 connector pinout
  - Important note: TERM jumper must be cut on Feather CAN boards to avoid double-termination
  - FSD V14 support for HW4 (bit 60) and emergency vehicle detection (bit 59)
  - Hardware variant detection guide (Legacy vs HW3 vs HW4)
