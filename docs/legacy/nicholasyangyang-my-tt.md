---
title: nicholasyangyang-my-tt
description: A Tesla FSD (Full Self-Driving) CAN bus enabler firmware supporting multiple hardware platforms (Adafruit Feather RP2040
category: legacy
folder: legacy
tags: [legacy, community, external]
author: nicholasyangyang
repo: my-tt
---

# nicholasyangyang-my-tt

## Overview

A Tesla FSD (Full Self-Driving) CAN bus enabler firmware supporting multiple hardware platforms (Adafruit Feather RP2040 CAN, Feather M4 CAN Express, ODrive v3/STM32F405). It intercepts and modifies specific CAN bus messages to enable FSD functionality, suppress nag warnings, and map speed profiles via the follow-distance stalk. Supports HW3, HW4, and Legacy (pre-Palladium) Tesla vehicles.

## Technical Details

- **Platform**: RP2040 (Adafruit Feather CAN), ATSAME51 (Feather M4 CAN), STM32F405 (ODrive v3), ESP32 (TWAI)
- **Language**: C++ (Arduino framework + PlatformIO)
- **CAN Interface**: MCP2515 (SPI), native ATSAME51 MCAN, STM32 bxCAN, ESP32 TWAI — all at 500 kbps
- **License**: GPL-3.0

## Architecture

- `RP2040CAN.ino` — Arduino entry point with board/vehicle selection via `#define` directives.
- `src/main.cpp` — PlatformIO equivalent entry point.
- `include/app.h` — Application logic: initializes CAN driver, runs main loop reading frames and dispatching to handlers.
- `include/handlers.h` — Vehicle-specific CAN message handlers (`LegacyHandler`, `HW3Handler`, `HW4Handler`), each implementing FSD enable, speed profile mapping, and nag suppression.
- `include/can_driver.h` / `include/drivers/` — Abstracted CAN driver interface with platform-specific implementations (MCP2515, SAME51, STM32 bxCAN via `stm32_bxcan_driver.h`, ESP32 TWAI via `twai_driver.h`).
- `include/can_helpers.h` — Bit manipulation and CAN frame utility functions.
- `include/can_frame_types.h` — Common CAN frame data structures.
- `platformio.ini` — Multi-environment build configs for all supported boards.
- `my_monitor.py` — Serial monitor script for STM32/ODrive.

## CAN Bus Integration

Intercepts and re-transmits modified CAN frames at 500 kbps:

**Legacy Handler** (pre-Palladium Model S/X with HW3):

- CAN ID 69 (0x045, STW_ACTN_RQ): Reads follow-distance stalk position for speed profile mapping
- CAN ID 1006: Reads mux index, sets FSD enable bit (bit 46), speed profile, clears nag bit (bit 19)

**HW3 Handler**:

- CAN ID 1016: Reads follow distance from byte 5 bits 7:5 for speed profile
- CAN ID 1021: FSD enable (bit 46), nag suppression (bit 19), speed offset in mux index 2

**HW4 Handler**:

- CAN ID 921 (0x399): ISA speed chime suppression with checksum recalculation
- CAN ID 1016: 5-level speed profile from follow distance
- CAN ID 1021: FSD enable (bit 46), emergency vehicle detection (bit 59), nag suppression (bit 19, bit 47), speed profile in mux index 2 (byte 7 bits 6:4)

## Relevance to Our Project

Directly relevant as a Tesla FSD CAN enabler. Provides a clean, well-structured multi-platform codebase with abstracted CAN driver interfaces and vehicle-variant handlers.

- **Reusability**: High
- **Key Takeaways**:
  - Clean driver abstraction pattern supporting MCP2515, SAME51, STM32 bxCAN, and ESP32 TWAI
  - Detailed CAN ID documentation for FSD enable, nag suppression, and speed profile across HW3/HW4/Legacy
  - Handler pattern per vehicle variant with polymorphic dispatch
  - PlatformIO multi-environment build configuration
  - Checksum recalculation for CAN ID 921
