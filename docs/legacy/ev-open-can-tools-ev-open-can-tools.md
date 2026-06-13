---
title: ev-open-can-tools / ev-open-can-tools
description: Upstream open-source CAN bus modification firmware for Tesla vehicles. ESP32/RP2040/M4 hardware, multi-handler architecture, AP Injection Gate (v2.5.x), speed profiles, nag suppression, ISA chime, EVD, Smart Summon.
category: legacy
folder: legacy
tags: [legacy, community, external]
author: ev-open-can-tools
repo: ev-open-can-tools
---

# ev-open-can-tools / ev-open-can-tools

## Overview

Open-source CAN bus modification firmware for Tesla vehicles. Intercepts, modifies, and re-transmits CAN frames in real time to enable FSD region-gate bypass, nag suppression, speed profiles, ISA chime suppression, emergency vehicle detection, and Smart Summon compatibility. Current stable release is **v2.5.2** (April 2026). Supports multiple hardware platforms and includes a WiFi web dashboard on ESP32 boards.

Submodule tracked on `main` branch.

## Technical Details

- **Platform**: Adafruit Feather RP2040 CAN, Feather M4 CAN Express, ESP32 (TWAI, MCP2515 variants), M5Stack Atomic CAN Base, Waveshare ESP32-S3 RS485/CAN
- **Language**: C++ (Arduino framework via PlatformIO)
- **CAN Interface**: MCP2515 (SPI), ATSAME51 native CAN, ESP32 TWAI
- **License**: GPL-3.0

## Architecture

```
include/
  app.h              — setup/loop templates, driver selection
  handlers.h         — LegacyHandler, HW3Handler, NagHandler, HW4Handler structs
  can_frame_types.h  — portable CanFrame type
  can_helpers.h      — bit manipulation, checksum, mux helpers
  shared_types.h     — Shared<T> atomic wrapper, runtime flags
  log_buffer.h       — ring buffer for serial/dashboard log
  drivers/           — mcp2515_driver.h, twai_driver.h, same51_driver.h, mock_driver.h
  web/               — ESP32 WiFi dashboard (HTML/JS served over AP)
src/main.cpp         — PlatformIO entry point
platformio_profile.h — compile-time feature selection
platformio.ini       — build environments
```

## Handler Architecture

All handlers inherit `CarManagerBase` which holds shared state: `speedProfile`, `ADEnabled`, `APActive`, `Parked`, `Summoning`, `gatewayAutopilot`, `speedOffset`.

### AP Injection Gate (v2.5.x — required for Tesla FW 2026.14.3+)

Tesla firmware 2026.14.3+ rejects always-on CAN injection. The gate opens injection only when one of three conditions is true:

```cpp
bool injectionGateOpen() const {
    return (bool)APActive || (bool)Parked || (bool)Summoning;
}
```

- **APActive** — set from `DAS_status` (CAN 921) `DAS_autopilotStatus` active states
- **Parked** — set from `DI_systemStatus` (CAN 280) `DI_gear` and `DIF_torque` (CAN 390). Defaults `true` at boot so the gate is open when the DI is asleep (Sentry mode). Flips false on first R/N/D gear frame.
- **Summoning** — requires both `DI_autonomyControlActive` (CAN 280 byte 6 bit 2) AND a `UI_selfParkRequest` non-zero command (CAN 1016 byte 3 bits 4-7) observed in the current autonomy episode. ACA falling edge clears the spr-seen flag so plain TACC re-engagement does not re-latch the gate.

Gate signals consumed by all three handlers (Legacy, HW3, HW4):
- CAN 280 `DI_systemStatus` — `DI_gear` (byte 2 bits 0-2), `DI_autonomyControlActive` (byte 6 bit 2)
- CAN 390 `DIF_torque` / `DIR_torque` — `DI_gear` fallback
- CAN 921 `DAS_status` — `DAS_autopilotStatus`
- CAN 1016 `UI_driverAssistControl` — `UI_selfParkRequest` (byte 3 bits 4-7)

### LegacyHandler (HW2.5 / pre-AP retrofit)

Filters: CAN IDs 69, 280, 390, 921, 1006

- **CAN 69** `STW_ACTN_RQ` — follow-distance stalk → speed profile (3 levels, byte 1 bits 7-5)
- **CAN 1006** mux 0 — FSD enable (bit 46), speed profile injection (`setSpeedProfileV12V13`)
- **CAN 1006** mux 1 — nag suppression (bit 19 clear), gated by `injectionGateOpen()` on dashboard builds

### HW3Handler

Filters: CAN IDs 280, 390, 921, 1016, 1021, 2047

- **CAN 1016** — follow-distance stalk → speed profile (3 levels, byte 5 bits 7-5); `UI_selfParkRequest` for summon gate
- **CAN 1021** mux 0 — FSD enable (bit 46), speed profile, speed offset read (byte 3 bits 6-1, scale ×5, range 0-100)
- **CAN 1021** mux 1 — nag suppression (bit 19 clear, bit 46 set), gated by `injectionGateOpen()`
- **CAN 1021** mux 2 — speed offset injection (byte 0 bits 7-6 + byte 1 bits 5-0)
- **CAN 2047** — GTW_autopilot observation and logging

### NagHandler (standalone, compile-time `-D NAG_KILLER`)

Filters: CAN ID 880 only

Echoes `EPAS3P_sysStatus` (0x370) with counter+1 and fixed 1.80 Nm torque when `handsOnLevel == 0`. This is the upstream's simple always-echo implementation — no DAS gating, no organic torque variation.

Frame modification:
- `data[2]` lower nibble = `0x08` (tRaw high byte)
- `data[3]` = `0xB6` (tRaw low byte → 1.80 Nm fixed)
- `data[4]` |= `0x40` (handsOnLevel = 1)
- `data[6]` lower nibble = counter + 1 (mod 16)
- `data[7]` = `(sum(b0..b6) + 0x73) & 0xFF`

### HW4Handler

Filters: CAN IDs 280, 390, 921, 1016, 1021, 2047

- **CAN 921** — `DAS_autopilotStatus` for APActive; ISA chime suppression (bit 5 of byte 1, checksum recalc)
- **CAN 1016** — follow-distance stalk → speed profile (5 levels); `UI_selfParkRequest` for summon gate
- **CAN 1021** mux 0 — FSD enable (bit 46), FSD v14 (bit 60), EVD (bit 59), gated by `injectionGateOpen()`
- **CAN 1021** mux 1 — nag suppression (bit 19 clear, bit 47 set), gated by `injectionGateOpen()`
- **CAN 1021** mux 2 — speed profile injection (`setSpeedProfileHW4`, byte 7 bits 6-4)
- **CAN 2047** — GTW_autopilot observation

## CAN Signal Reference

| CAN ID | Decimal | Signal | Location | Notes |
|--------|---------|--------|----------|-------|
| 0x045 | 69 | STW_ACTN_RQ follow-distance | byte 1 bits 7-5 | Legacy only |
| 0x118 | 280 | DI_gear | byte 2 bits 2-0 | 1=P, 2=R, 3=N, 4=D, 7=SNA |
| 0x118 | 280 | DI_autonomyControlActive | byte 6 bit 2 | Summon gate |
| 0x186 | 390 | DIF/DIR gear | byte 0 bits 2-0 | Fallback gear source |
| 0x370 | 880 | EPAS3P_sysStatus | full frame | Nag killer target |
| 0x399 | 921 | DAS_autopilotStatus | byte 0 bits 3-0 | AP gate |
| 0x399 | 921 | ISA chime | byte 1 bit 5 | HW4 only |
| 0x3F8 | 1016 | UI_selfParkRequest | byte 3 bits 7-4 | Summon gate |
| 0x3F8 | 1016 | follow-distance | byte 5 bits 7-5 | HW3/HW4 |
| 0x3FD | 1021 | UI_autopilotControl mux 0 | bit 46 FSD, bit 60 FSDv14, bit 59 EVD | |
| 0x3FD | 1021 | UI_autopilotControl mux 1 | bit 19 nag, bit 47 HW4 | |
| 0x3FD | 1021 | UI_autopilotControl mux 2 | speed profile / offset | |
| 0x7FF | 2047 | GTW_autopilot | mux 2 | Observation only |

## Relevance to Our Project

This is the primary upstream reference for our firmware. Key differences from our codebase:

- **AP Injection Gate** — upstream has it (v2.5.x), our codebase does not. Required for Tesla FW 2026.14.3+.
- **Speed profile auto-mapping** — upstream maps follow-distance stalk to AP aggressiveness profile automatically. Our `profile.h` exists but is not wired to the stalk.
- **Speed offset injection** — upstream HW3 reads and re-injects speed offset from mux-2. Our `offsets.h` is a stub.
- **Emergency vehicle detection** — upstream HW4 sets bit 59 on mux-0. Not in our codebase.
- **NagHandler** — upstream uses simple always-echo with fixed 1.80 Nm. Our `nag.h` is more sophisticated (safe/natural modes, Gaussian jitter) but uses a smaller torque range.
- **Nag suppression gating** — upstream gates bit-19 nag on `injectionGateOpen()`. Our nag suppress is ungated.

### Recent Changes (v3.0.0–v3.0.1, 2026)

- **Migrated from Arduino to ESP-IDF 6.0.1** (223 KB flash savings) — Complete rewrite of the ESP32 build target from the Arduino framework to ESP-IDF 6.0.1 native APIs. Saved 223 KB of flash (from 1.38 MB to 1.16 MB) by eliminating the Arduino HAL, WiFi, and BLE stack duplication. The Arduino framework is retained as a compile-time fallback for RP2040 and M4 targets. FreeRTOS task model replaces the Arduino `loop()` pattern with dedicated pinned-to-core tasks: CAN RX (core 0), CAN TX injection (core 0), web server (core 1), and watchdog (core 1).
- **New CAN driver diagnostics** (TWAI and MCP2515 per-driver JSON diagnostics) — Each CAN driver now exposes a `/api/diag/can` endpoint returning JSON with: bus state, RX/TX error counters, arbitration lost count, bus-off count, last error code, driver uptime, and per-frame-ID TX success/failure rates. Available via both the web dashboard and serial `candiag` command.
- **MCP2515 automatic recovery from TX failure / bus-off** — The MCP2515 driver now detects TX failure (MCP2515 TXBnCTRL TXERR flag) and bus-off (CANSTAT 0xE0) states and performs automatic recovery: abort pending TX, reset the MCP2515, reconfigure bit timing, and re-enter normal mode. Includes exponential backoff (1s → 2s → 4s → 8s → max 30s) for repeated failures.
- **Byte-mask matching for plugin rules** — The plugin rule system (`include/plugins/`) now supports byte-level mask matching in addition to CAN ID filtering. Each rule can specify a `byteMask[8]` and `byteValue[8]` pair; a frame matches only when `(frame.data[i] & byteMask[i]) == byteValue[i]` for all bytes with a non-zero mask. This enables signal-level filtering (e.g., match only mux-0 frames on 0x3FD) without decoding the full frame.
- **Frame timing diagnostics for gate-critical CAN IDs** — The diagnostic system now tracks inter-arrival times (min/max/mean/stddev) for all gate-critical CAN IDs (0x280, 0x390, 0x399, 0x3F8). Alerts fire when any ID deviates >3σ from its observed mean, flagging potential bus congestion, dropped frames, or gateway firmware changes. Exposed via `canstats` serial command and the web dashboard.
- **Multi-SSID WiFi with auto-failover** — The ESP32 can now scan and store up to 4 preferred SSIDs (configured via `wifiaps` serial command or web UI). On boot, it scans for available APs and connects to the strongest matching SSID. If the connection drops, it auto-failovers to the next available SSID within 5 seconds. The AP-only fallback mode still activates when no saved SSIDs are found.
- **PlatformIO board targets for AtomS3 Mini CAN Base and Waveshare ESP32-S3** — New `[env:atoms3-mini-can]` and `[env:waveshare-esp32-s3-can]` build environments with matching M5Stack AtomS3 Lite + ATOMIC CAN Base pinout (MCP2515 on SPI2_HOST, CS=GPIO 5, INT=GPIO 6) and Waveshare ESP32-S3 RS485/CAN pinout (TWAI on GPIO 4/5 or MCP2515 on standard SPI).
