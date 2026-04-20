---
title: ev-open-can-tools / ev-open-can-tools
description: This is the **upstream repository** that our Tesla-CAN-Mod project is forked from / closely tracks. It is a general-purp
category: legacy
folder: legacy
tags: [legacy, community, external]
author: ev
repo: open-can-tools-ev-open-can-tools
---

# ev-open-can-tools / ev-open-can-tools

## Overview

This is the **upstream repository** that our Tesla-CAN-Mod project is forked from / closely tracks. It is a general-purpose, open-source CAN bus modification tool for Tesla vehicles that intercepts, modifies, and re-transmits CAN frames in real time to enable features like FSD region-gate bypass, nag suppression, speed profiles, ISA chime suppression, and emergency vehicle detection. It supports multiple hardware platforms and includes a WiFi web dashboard on ESP32 boards.

## Technical Details

- **Platform**: Adafruit Feather RP2040 CAN, Feather M4 CAN Express, ESP32 (multiple variants), M5Stack Atomic CAN Base
- **Language**: C++ (Arduino framework via PlatformIO)
- **CAN Interface**: MCP2515 (SPI), Adafruit SAME51 built-in CAN, ESP32 TWAI (built-in)
- **License**: GPL-3.0

## Architecture

- `src/main.cpp` — PlatformIO entry point; selects driver at compile time via `#ifdef` flags
- `include/app.h` — Core application logic (setup/loop templates)
- `include/handlers.h` — CAN frame handlers for Legacy/HW3/HW4 vehicle variants
- `include/drivers/` — Hardware abstraction: `mcp2515_driver.h`, `esp32_mcp2515_driver.h`, `same51_driver.h`, `twai_driver.h`
- `include/can_frame_types.h`, `can_helpers.h` — CAN frame type definitions and bit manipulation helpers
- `include/plugin_engine.h` — JSON-based runtime plugin system for custom CAN modifications
- `include/web/` — WiFi web dashboard (ESP32 only) for real-time monitoring and OTA updates
- `platformio_profile.h` — Compile-time feature selection (hardware variant, feature toggles)
- `platformio.ini` — Build environments for each supported board

## CAN Bus Integration

Directly intercepts and modifies CAN frames on the Tesla vehicle bus:

- **CAN ID 0x3FD (1021)** — `UI_autopilotControl`: FSD enable bit (bit46, bit60), nag suppression (bit19), speed profile
- **CAN ID 0x3EE (1006)** — Legacy autopilot control
- **CAN ID 0x3F8 (1016)** — Follow-distance stalk reading for speed profile mapping
- **CAN ID 0x399 (921)** — ISA speed chime suppression (HW4)
- **CAN ID 0x370 (880)** — EPAS nag killer (counter+1 echo)
- Operates at 500 kbps CAN bus speed

## Relevance to Our Project

This **is** the canonical upstream for our Tesla-CAN-Mod firmware. The `firmware/` directory in our monorepo is essentially a structured version of this codebase.

- **Reusability**: High — this is our primary firmware source
- **Key Takeaways**:
  - Multi-driver architecture pattern (MCP2515, TWAI, SAME51) via compile-time selection
  - Plugin engine for extensible CAN modifications without recompilation
  - WiFi dashboard with OTA update capability
  - Separation of CAN logic from hardware abstraction
  - HW3/HW4/Legacy handler pattern for different Tesla hardware generations
