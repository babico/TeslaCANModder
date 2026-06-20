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

- **Platform**: Flipper Zero (primary), ESP32 + MCP2515 / TWAI / TTGO T-Display (alternative ports)
- **Language**: C (Flipper Zero FAP), C++ (ESP32 port)
- **CAN Interface**: MCP2515 via SPI (Electronic Cats CAN Bus Add-On for Flipper, generic MCP2515 for ESP32), ESP32 TWAI (native), TTGO T-Display with SPI remapped MCP2515
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

- PlatformIO project targeting M5Stack ATOM Lite + ATOMIC CAN Base, LILYGO T-2CAN ESP32-S3, LILYGO T-CAN485, LILYGO TTGO T-Display + MCP2515, and generic ESP32
- Adds WiFi AP web dashboard with WebSocket real-time updates
- Tesla dark theme UI with BMS ring gauge, CAN stats, and control toggles
- REST API (`/api/status`) for external integration
- TTGO T-Display variant: 1.14" ST7789 LCD with on-device status display, SPI-remapped MCP2515 pins (MISO=32, SCK=33, MOSI=25, CS=26), display sleep/wake via button, brightness control, auto-timeout
- Abstract `CanDriver` interface with `begin(listen_only)`, `send()`, `receive()`, `errorCount()`, `txCount()`, `setListenOnly()` — compile-time driver selection
- Listen-only mode as safe default on boot; runtime toggle between normal and listen-only
- OTA detection hardening: consecutive-frame confirmation (3 assert, 6 clear) to avoid false positives from firmware versions with non-zero idle states
- China mode: bypass FSD UI selection check for Chinese market vehicles
- Emergency vehicle detect: sets bit59 in mux0 (HW4)

**Supplementary:**

- `enhauto-re/` — Reverse engineering notes and tools
- `tools/` — Development utilities
- `HARDWARE.md` — Comprehensive hardware comparison (8+ boards), X179 20/26-pin variants, X052 connector (2019 Model 3), DoIP warning for 2024+ Juniper, termination resistor guide
- `SECURITY.md` — Documents Tesla VIN-level ban risk (April 2026)
- `ROADMAP.md` — Feature roadmap
- `esp32/platformio.ini` — Multi-board build environments including TTGO T-Display, LILYGO T-2CAN, LILYGO T-CAN485

## CAN Bus Integration

Extensive CAN bus integration with the most comprehensive CAN ID coverage seen in legacy repos:

**FSD Control:**

- `0x3FD (1021)` — `UI_autopilotControl` (HW3/HW4): FSD enable (bit46, bit60), nag (bit19), speed profile
- `0x3EE (1006)` — `UI_autopilotControl` (Legacy): FSD enable, speed profile
- `0x3F8 (1016)` — Follow-distance stalk for speed profile mapping
- `0x331 (817)` — `DAS_autopilotConfig`: TLSSC Restore — read-modify-retransmit at ~1 Hz, sets byte[0] lower 6 bits to 0x1B (SELF_DRIVING)

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
  - Abstract `CanDriver` interface with listen-only as safe default — runtime toggle between modes
  - OTA guard: hardened detection via consecutive-frame confirmation (3 assert, 6 clear) to avoid false positives
  - VIN-level ban documentation (SECURITY.md) — critical operational risk information
  - BMS dashboard pattern with live voltage/current/SOC/temp display
  - Battery preconditioning trick (`0x082 byte[0] = 0x05`)
  - CRC/checksum recalculation after frame modification
  - ESP32 web dashboard with WebSocket real-time push and REST API
  - TTGO T-Display on-device LCD: 1.14" ST7789 with SPI-remapped MCP2515, display sleep/wake, brightness, auto-timeout
  - Clean separation of CAN logic (`fsd_handler`) from platform-specific code (Flipper scenes, ESP32 WiFi, display)
  - DAS-aware nag killer: gates echo on `0x39B` DAS_autopilotHandsOnState to reduce spurious bus traffic; uses organic torque variation to evade telemetry-based VIN bans
  - TLSSC Restore (`0x331`): recovers Traffic Light and Stop Sign Control on banned vehicles via DAS config spoofing (v2.10+)
  - Ban Shield (`0x7FF`): captures healthy GTW_carConfig baseline and retransmits to block server-side ban modifications in real time (v2.9+)
  - AP-First mode: delays `0x3FD` injection until AP is engaged (reads `DAS_autopilotState` from `0x39B`) — required for Tesla firmware 2026.14.x (v2.14+)
  - China mode: bypass FSD UI selection for Chinese market vehicles
  - Emergency vehicle detect: sets bit59 in mux0 (HW4)
  - HARDWARE.md expanded: X052 connector pinout (2019 Model 3), X179 20/26-pin variants (pre/post April 2024), DoIP warning for Juniper/Highland, 8+ board comparison

### Recent Changes (v2.16-beta.5–beta.6, June 2026)

**New features:**

- **AP-First gate with 1-second stability debounce** — Withholds 0x3FD/0x3EE/0x370 injection until AP is engaged and stable for 1s, preventing AP engagement failure on 2026.14.x firmware. Uses a stability counter that resets on any AP state change within the debounce window.
- **DAS escalation-edge grip pulse re-arm** — Detects rising edge on `das_hands_on_state` to re-arm nag suppression on HW4 Juniper trims where EPAS byte4 is frozen. Watches the full handshake sequence (states 0→2→7→2→0) and triggers a grip-simulating EPAS pulse on each escalation transition.
- **0x399 hands-on fallback for HW4 trims without 0x39B** — Reads `DAS_handsOnState` from 0x399 byte5[5:2] when 0x39B is absent from the bus. Handles HW4 Juniper trims that route hands-on state through the DAS_status frame instead of the separate DAS_status2 frame.
- **Scroll-press AP engage via 0x3C2** — State machine for HW4 AP engagement without touching 0x3FD. Monitors scroll-wheel presses on 0x3C2 (VCLEFT_switchStatus) and translates long-press patterns into AP engage signals, avoiding the 0x3FD rejection on 2026.14.x.
- **Continuous AP auto-re-engage (HW3/Legacy)** — Watches disengage conditions (brake pedal via 0x145, stalk position via 0x229, AP state via 0x39B) and automatically re-engages AP via stalk emulation when safe. Includes a configurable cooldown period to prevent rapid cycling.
- **0x229 CRC table fully cracked** — 16 neutral CRC values by counter with position XOR deltas for gear lever/stalk position detection. The crack table maps each mux counter nibble (0x0–0xF) to its valid CRC nibble, enabling CRC validation and spoofing of SCCM stalk position frames.
- **All-zero 0x398 stub filtering** — Prevents HW4 Juniper cars from being mislabelled Legacy. Some Juniper gateways emit zero-filled 0x398 frames during boot that would otherwise trigger the HW2.5/Legacy detection path.
- **Host-native test suite with 212 assertions** — Comprehensive native test suite covering FSD state machine, nag killer timing, BMS decoding, CRC validation, AP gate logic, and HW detection fallback paths.
- **CRC cracking tools** (`crack_0x229.py`, `tesla_crc_cracker.py`) — Python tools for brute-force CRC-8/OPENSAFETY analysis with per-ID XOR magic table recovery.
- **HTTP CAN stream server** (candump-compatible) — Streams raw CAN frames over HTTP in candump format for integration with external analysis tools.

**New CAN signals documented:**

- **0x238** `UI_driverAssistMapData` — Map-derived speed limit (byte 0 / 2 = km/h, byte 1 speed limit sign type)
- **0x331** `DAS_autopilotConfig` — TLSSC restore fields (byte 0 lower 6 bits control FSD tier, byte 1 bits 7:6 control NOA/RED_LIGHT/WARNING)
- **0x145** `ESP_driverBrakeApply` — Brake pedal position tracking (byte 2 raw to 0–100% scaled, byte 3 brake pressure)
- **0x389** `DAS_status2` — ACC set speed (byte 1 × 0.4 km/h), ACC speed limit (byte 2), DAS path prediction
- **0x2B9** `DAS_control` — Set speed target (byte 1 × 0.4 km/h), ACC resume/cancel/coast/accel buttons
- **0x293** `DAS_settings` — Autosteer readback (byte 2 bit 5), TACC readback (byte 2 bit 4), autopilot settings confirmation

**Adopted by us:**

- **0x398 all-zero stub filter** → `firmware/lib/vehicle/can/handler/bus/vehicle.h` and transport copy
- **ISA chime HW3 guard** → `firmware/lib/vehicle/can/handler/bus/chassis.h` and transport copy
- **0x39B comment fix** → `firmware/lib/vehicle/can/handler/frame_readers.h` and transport copy

## Upstream (2026-06-20)

12 new commits on `main` (v2.16-beta.6 → beta.8):

- TWAI auto-recover from bus-off so RX does not silently die (`515e25a`).
- EPAS-faithful nag rewritten as a **demand-state (Mode-C) machine** (`2bc2f33`, `b9bb6c2`).
- HW4 Highland `0x39B` byte0 `DAS_autopilotState` auto-fallback (`b78acd2`).
- `tools/feifan_0x370.py` decode tool for captured `0x370` traffic (`cf631e5`).
- Web-stream CAN capture enabled during **Active** mode (`e9dbef7`).
- HARDWARE.md / README: tap Party CAN (2/3) for nag killer; relabel X179 13/14 as Chassis CAN (`8b9e286`, `c1f2c36`, `5c173ec`).
- Changelog headers for 2.16-beta.7 / beta.8 (`dedb996`, `2e3c4c8`).
- Contributor credits refresh across EN / zh-CN / zh-TW (`6651805`).

For TeslaCANModder: Mode-C is the same organic-state-machine idea our `nag:mode:organic` already implements but driven by DAS rather than a fixed schedule — worth a future cross-read of `fsd_handler.c`. The HW4 Highland `0x39B` byte0 fallback and the TWAI bus-off recovery pattern are also relevant. Full commit table: `docs/legacy/upstream-review-2026-06-20.md`.
