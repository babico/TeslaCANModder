---
title: ylovex75-tesla-open-can-mod-release
description: A comprehensive fork of Tesla-OPEN-CAN-MOD — an ESP32/ESP32-S3 CAN bus modification firmware for Tesla vehicles. It inte
category: legacy
folder: legacy
tags: [legacy, community, external, archived]
author: ylovex75
repo: tesla-open-can-mod-release
status: deleted
---

# ylovex75-tesla-open-can-mod-release

> **Repository deleted.** `https://github.com/ylovex75/tesla-open-can-mod-release` returned 404 as of May 2026. Content below is preserved from the last recorded state.

## Overview

A comprehensive fork of Tesla-OPEN-CAN-MOD — an ESP32/ESP32-S3 CAN bus modification firmware for Tesla vehicles. It intercepts, modifies, and injects CAN frames in real time to enable FSD activation, nag suppression, speed profile management, ISA chime mute, battery preheating, and more. Includes a full-featured WiFi web dashboard for runtime control and monitoring.

## Technical Details

- **Platform**: ESP32 / ESP32-S3 (PlatformIO + Arduino framework)
- **Language**: C++ (C++17)
- **CAN Interface**: ESP32 native TWAI peripheral with external transceivers (SIT1050T, SN65HVD230, CA-IS3050G)
- **License**: GNU General Public License v3 (GPLv3)

## Architecture

- `src/main.cpp` — PlatformIO entry point. Calls `appSetup<TWAIDriver>()` and `appLoop<TWAIDriver>()` with configurable TX/RX pins.
- `include/app.h` — Core application logic templates for setup and main loop.
- `include/handlers.h` — CAN frame handlers per hardware variant (Legacy / HW3 / HW4). Modifies frames for 0x3FD, 0x3EE, 0x313, 0x370, 0x399, and injects 0x082.
- `include/can_helpers.h` — CRC/checksum recalculation, bitmask filter utilities.
- `include/can_live.h` / `can_monitor.h` — Real-time CAN signal decoding for the dashboard (O(1) lookup via 1024-element direct-mapped table).
- `include/drivers/twai_driver.h` — TWAI peripheral abstraction.
- `include/web/` — Web server with 29 endpoints, single-page dashboard (settings, system, live charts).
- `include/log_buffer.h` — Ring-buffer based logging (256 entries).
- `include/shared_types.h` / `can_frame_types.h` — Shared data structures.
- `platformio.ini` — Build environments for ESP32, ESP32-S3 (Waveshare, M5Stack, LilyGo), and native test targets.
- `test/` — 114+ native unit tests.
- `firmware-esp32-v1.7.0.bin` / `firmware-esp32s3-v1.7.0.bin` — Pre-built firmware binaries (v1.7.0).
- `scripts/` — Build and deployment automation.

## CAN Bus Integration

Extensive CAN bus integration — this is the core purpose of the firmware:

**Modified frames** (intercepted, bits altered, re-transmitted):

- `0x313` (TrackModeRequest) — Track mode bit for HW3
- `0x399` (DAS_status) — ISA speed chime suppression
- `0x3EE` (UI_autopilotControl) — FSD enable, nag clear, speed profile (Legacy)
- `0x3FD` (UI_autopilotControl) — FSD enable, speed offset, emergency vehicle detection, nag clear, enhanced AP (HW3/HW4)

**Injected frames** (firmware-generated):

- `0x082` (UI_tripPlanning) — Battery preheat trigger (500 ms interval)
- `0x370` (EPAS3P_sysStatus) — Spoofed torque for nag killer

**Monitored frames** (passive decode for dashboard):

- 0x118, 0x132, 0x185, 0x238, 0x257, 0x292, 0x2B9, 0x312, 0x318, 0x334, 0x370, 0x389, 0x399, 0x3D9, 0x3F8, 0x3FD

Uses O(1) bitmask filter (32-element array covering IDs 0–1023) and single-shot TX (TWAI_MSG_FLAG_SS) for injected frames.

## Relevance to Our Project

This is essentially the upstream project that our Tesla-CAN-Mod firmware is based on. It represents the most complete open-source Tesla CAN modification firmware available, and our project extends/modifies it.

- **Reusability**: High
- **Key Takeaways**:
  - Complete CAN frame handler architecture for Legacy / HW3 / HW4 variants
  - O(1) filtering and signal lookup patterns for real-time CAN processing
  - CRC/checksum recalculation for all modified frames
  - Web dashboard with OTA, live charts, i18n, and captive portal
  - Safety features: OTA detection pause, NVS corruption recovery, rate limiting
  - Comprehensive native test suite (114+ tests)
  - Pre-built binaries and multi-board PlatformIO configuration
  - Battery preheat injection with auto-stop safeguards
