---
title: tesla-open-can-mod-main
description: The most mature and actively developed open-source Tesla CAN bus modification tool. Evolved from the single-file FSD ena
category: legacy
folder: legacy
tags: [legacy, community, external]
author: tesla
repo: open-can-mod-main
---

# tesla-open-can-mod-main

## Overview

The most mature and actively developed open-source Tesla CAN bus modification tool. Evolved from the single-file FSD enabler into a well-structured PlatformIO project supporting multiple CAN drivers (MCP2515, ATSAME51, ESP32 TWAI), multiple hardware variants (Legacy/HW3/HW4), and comprehensive test infrastructure. Includes a documentation site, CI/CD pipeline, and native test environment.

## Technical Details

- **Platform**: Adafruit Feather RP2040 CAN, Feather M4 CAN Express, ESP32 (various), M5Stack Atomic CAN Base
- **Language**: C++ (Arduino/PlatformIO)
- **CAN Interface**: MCP2515 over SPI, ATSAME51 native MCAN, ESP32 TWAI — selected at build time via driver defines
- **License**: GPL-3.0

## Architecture

Well-structured PlatformIO project with driver abstraction:

- `src/main.cpp` — Entry point that selects CAN driver at compile time (`DRIVER_MCP2515`, `DRIVER_SAME51`, `DRIVER_TWAI`) and delegates to `appSetup()`/`appLoop()` templates
- `include/app.h` — Core application logic with templated setup/loop functions
- `include/handlers.h` — Handler classes: `LegacyHandler` (CAN IDs 69, 1006), `HW3Handler` (1016, 1021), `HW4Handler` (1016, 1021 with V14 extensions). Each handler defines `filterIds()` for efficient frame filtering
- `include/drivers/` — Abstracted CAN driver implementations (mcp2515_driver.h, same51_driver.h, twai_driver.h)
- `include/can_helpers.h` — Shared bit manipulation utilities
- `include/shared_types.h` — Thread-safe shared state types
- `include/log_buffer.h` — Ring buffer for debug logging
- `include/web/` — Web server for ESP32 variants
- `RP2040CAN/` — Arduino IDE entry point and `sketch_config.h` for hardware/feature defines
- `platformio.ini` — Build environments for all supported boards plus native test environment
- `test/` — Test suite including native tests
- `scripts/` — Build helper scripts (e.g., `platformio_sync_ino_defines.py`)
- `docs-site/` — Documentation website source
- `.gitlab-ci.yml` — CI/CD pipeline

## CAN Bus Integration

Comprehensive CAN message handling at 500 kbps with detailed per-variant tables:

**Legacy (HW3 Retrofit):**

- CAN ID 69 (STW_ACTN_RQ): Read follow-distance stalk position for profile mapping
- CAN ID 1006: Mux 0 — FSD enable (bit 46), speed profile (bits 49-50); Mux 1 — nag suppress (bit 19)

**HW3:**

- CAN ID 1016 (UI_driverAssistControl): Read follow-distance setting (bits 45-47)
- CAN ID 1021 (UI_autopilotControl): Mux 0 — FSD enable (bit 46), FSD stops control (bit 38), offset (bits 25-30); Mux 1 — nag suppress (bit 19, UI_applyEceR79); Mux 2 — inject offset (bits 6-7, 8-13)

**HW4:**

- CAN ID 921 (DAS_status): Suppress speed chime (bit 13), checksum update (bits 56-63)
- CAN ID 1016: Same as HW3
- CAN ID 1021: Same as HW3 plus: V14 enable (bit 60), emergency vehicle detection (bit 59), summon enable (mux 1 bit 47, UI_hardCoreSummon), 5-level speed profile (mux 2 bits 60-62)

Signal names sourced from tesla-can-explorer by @mikegapinski.

## Relevance to Our Project

This is the primary upstream reference for our project — the most feature-complete and well-architected FSD CAN mod available.

- **Reusability**: High
- **Key Takeaways**:
  - Clean driver abstraction pattern (CAN driver interface with MCP2515/SAME51/TWAI implementations)
  - Handler filter ID system for efficient frame processing
  - Native test environment (`env:native`) enables testing CAN logic without hardware
  - `Shared<T>` type for thread-safe state management
  - LogRingBuffer for debug output
  - HW4 checksum recalculation on DAS_status (CAN ID 921)
  - CI/CD pipeline and documentation site
  - Warning about firmware 2026.2.9.x & 2026.8.6 incompatibility on HW4
