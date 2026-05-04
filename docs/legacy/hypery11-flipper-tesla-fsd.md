---
title: hypery11 / flipper-tesla-fsd
description: A comprehensive Tesla FSD region-gate bypass application for Flipper Zero (and ESP32) that enables the FSD UI toggle for
category: legacy
folder: legacy
tags: [legacy, community, external]
author: hypery11
repo: flipper-tesla-fsd
---

# hypery11 / flipper-tesla-fsd

## Overview

A comprehensive Tesla FSD region-gate bypass application for Flipper Zero (and ESP32) that enables the FSD UI toggle for users with active FSD subscriptions in regions where it's not exposed. Beyond FSD, it includes a nag killer (DAS-aware, with organic torque variation for anti-detection), ISA speed chime suppression, OTA guard, battery preconditioning trigger, a live BMS dashboard, TLSSC Restore (v2.10+, recovers stop-sign/traffic-light control on VIN-banned vehicles via `0x331` DAS config spoofing), Ban Shield (v2.9+, freezes `0x7FF` GTW_carConfig to block server-side bans), and AP-First mode (v2.14+, required for Tesla firmware 2026.14.x — delays `0x3FD` injection until Autopilot is engaged). This is one of the most feature-complete and well-documented Tesla CAN modification projects in the community.

## Technical Details

- **Platform**: Flipper Zero (primary), ESP32 + MCP2515 (alternative port)
- **Language**: C (Flipper Zero FAP), C++ (ESP32 port)
- **CAN Interface**: MCP2515 via SPI (Electronic Cats CAN Bus Add-On for Flipper, generic MCP2515 for ESP32)
- **License**: GPL-3.0

## Architecture

**Flipper Zero App (main):**

- `tesla_fsd_app.c` / `tesla_fsd_app.h` — Application entry point, Flipper GUI/scene framework integration
- `fsd_logic/fsd_handler.c` / `fsd_handler.h` — Core CAN protocol logic (hardware-agnostic)
  - Defines all CAN IDs as constants (0x3FD, 0x3EE, 0x3F8, 0x370, 0x399, 0x398, etc.)
  - FSD state machine, BMS data parsing, OTA detection, nag killer
- `libraries/mcp_can_2515.h` — MCP2515 driver adapted for Flipper Zero SPI
- `scenes/` / `scenes_config/` — Flipper UI scenes (main menu, settings, running, HW detect, about)
- `assets/` — Flipper screen graphics (128x64 monochrome)
- `application.fam` — Flipper app manifest

**ESP32 Port (`esp32/`):**

- PlatformIO project targeting M5Stack ATOM Lite + ATOMIC CAN Base
- Adds WiFi AP web dashboard with WebSocket real-time updates
- Tesla dark theme UI with BMS ring gauge, CAN stats, and control toggles
- REST API (`/api/status`) for external integration

**Supplementary:**

- `enhauto-re/` — Reverse engineering notes and tools
- `tools/` — Development utilities
- `HARDWARE.md` — Detailed hardware comparison, wiring, termination resistor guide
- `SECURITY.md` — Documents Tesla VIN-level ban risk (April 2026)
- `ROADMAP.md` — Feature roadmap

## CAN Bus Integration

Extensive CAN bus integration with the most comprehensive CAN ID coverage seen in legacy repos:

**FSD Control:**

- `0x3FD (1021)` — `UI_autopilotControl` (HW3/HW4): FSD enable (bit46, bit60), nag (bit19), speed profile
- `0x3EE (1006)` — `UI_autopilotControl` (Legacy): FSD enable, speed profile
- `0x3F8 (1016)` — Follow-distance stalk for speed profile mapping

**Nag / Safety:**

- `0x370 (880)` — EPAS3P_sysStatus: nag killer via counter+1 echo (with organic torque variation [1.00–2.40 Nm] to avoid flat-signal telemetry detection)
- `0x399 (921)` — ISA speed chime suppression (HW4 only)
- `0x39B (923)` — `DAS_status`: read `DAS_autopilotHandsOnState` (v2.8+) to gate nag echo only when DAS actively demands hands-on (states 2–7, 9–10); also read `DAS_autopilotState` for AP-First mode trigger (v2.14+)

**BMS / Diagnostics:**

- `0x132 (306)` — BMS_hvBusStatus: pack voltage, current
- `0x292 (658)` — BMS_socStatus: state of charge
- `0x312 (786)` — BMS_thermalStatus: battery temperature

**Vehicle State:**

- `0x398 (920)` — GTW_carConfig: HW version auto-detection
- `0x7FF (2047)` — GTW_carConfig on Ethernet bus; also used by Ban Shield (v2.9+) — captures healthy mux frames as baseline and re-transmits any changed frame to block server-side VIN bans
- `0x318 (792)` — GTW_carState: OTA update detection (pauses TX)
- `0x082 (130)` — UI_tripPlanning: battery precondition trigger
- `0x331 (817)` — DAS_autopilotConfig: TLSSC Restore (v2.10+) — read-modify-retransmit at ~1 Hz, sets byte[0] lower 6 bits to 0x1B to restore TLSSC toggle on VIN-banned vehicles (confirmed on Palladium, HW4 Highland, Intel HW3)

**Extras (40+ additional CAN IDs defined):**

- Steering, braking, lighting, turn signals, gear, door state, wheel speeds, and more

Operates at 500 kbps. Supports three operation modes: Active (TX+RX), Listen-Only (RX only, no TX), Service (unrestricted).

## Relevance to Our Project

Very high relevance — this is the most feature-complete CAN modification project in the legacy collection. The `fsd_handler.c` logic is the definitive reference for CAN bit manipulation across all Tesla HW versions, and the ESP32 port adds a web dashboard pattern.

- **Reusability**: High
- **Key Takeaways**:
  - Most comprehensive CAN ID and signal definition set (40+ IDs with full bit/byte specs)
  - Three-mode operation (Active/Listen-Only/Service) — safe first-boot default pattern
  - OTA guard: detects Tesla OTA updates via `GTW_carState` and suspends TX to avoid bricking
  - VIN-level ban documentation (SECURITY.md) — critical operational risk information
  - BMS dashboard pattern with live voltage/current/SOC/temp display
  - Battery preconditioning trick (`0x082 byte[0] = 0x05`)
  - CRC/checksum recalculation after frame modification
  - ESP32 web dashboard with WebSocket real-time push and REST API
  - Clean separation of CAN logic (`fsd_handler`) from platform-specific code (Flipper scenes, ESP32 WiFi)
  - DAS-aware nag killer: gates echo on `0x39B` DAS_autopilotHandsOnState to reduce spurious bus traffic; uses organic torque variation to evade telemetry-based VIN bans
  - TLSSC Restore (`0x331`): recovers Traffic Light and Stop Sign Control on banned vehicles via DAS config spoofing (v2.10+)
  - Ban Shield (`0x7FF`): captures healthy GTW_carConfig baseline and retransmits to block server-side ban modifications in real time (v2.9+)
  - AP-First mode: delays `0x3FD` injection until AP is engaged (reads `DAS_autopilotState` from `0x39B`) — required for Tesla firmware 2026.14.x (v2.14+)
