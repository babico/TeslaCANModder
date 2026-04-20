---
title: sydneyg007-Tesla-Model-3-Front-DI-emulator
description: ESP32-based CAN bus emulators that replicate the messages sent by the Tesla Model 3 Front Drive Inverter (DI) on both th
category: legacy
folder: legacy
tags: [legacy, community, external]
author: sydneyg007
repo: Tesla-Model-3-Front-DI-emulator
---

# sydneyg007-Tesla-Model-3-Front-DI-emulator

## Overview

ESP32-based CAN bus emulators that replicate the messages sent by the Tesla Model 3 Front Drive Inverter (DI) on both the Party CAN bus and Vehicle CAN bus. Designed for a 2019 Tesla Model 3 Performance, these emulators prevent error messages when the front drive inverter logic board is disconnected.

## Technical Details

- **Platform**: ESP32
- **Language**: C++ (Arduino)
- **CAN Interface**: ESP32 built-in CAN (TWAI) via `esp32_can.h` library, TX: GPIO 22, RX: GPIO 23, 500 kbps
- **License**: None

## Architecture

Two separate Arduino sketches — one per CAN bus, with multiple versions:

### ESP32CanbusEmulatorPartyCanFrontDiV2

Emulates Front DI messages on the **Party CAN bus** (10ms cycle only):

- `0x186` (DIS_frontTorque) — 8 bytes, 10ms, checksum = counter + 0x07, counter 0x00–0x0F
- `0x187` — 8 bytes, 10ms, checksum = counter - 0x44, counter 0x60–0x6F
- `0x2D5` — 8 bytes, 10ms, counter 0x40–0x4F, checksum 0x8E–0x9D

### ESP32CanbusEmulatorVehicleCanFrontDiV4

Emulates Front DI messages on the **Vehicle CAN bus** (10ms, 100ms cycles):

- `0x1D5` — 8 bytes, 10ms, counter increments by 0x20 (0x00–0xE0), checksum increments by 0x20
- `0x2E5` — 8 bytes, 10ms, static data
- `0x186` (DIS_frontTorque) — 8 bytes, 100ms, checksum = counter + 0x07, counter 0x00–0x0F
- `0x195` — 8 bytes, 100ms, static data
- `0x1A5` — 8 bytes, 100ms, static data
- `0x27A` — 8 bytes, 100ms, counter 0xC0–0xCF, checksum 0x38–0x47
- `0x396` (DI_frontOilPump) — 8 bytes, 100ms, static data
- `0x757` — 8 bytes, 100ms, with periodic variant frame every 10th cycle

Multiple .ino files suggest iterative development (V2, V3, V4, V7 versions).

## CAN Bus Integration

Extensive direct CAN integration. Sends precisely timed Front DI emulation frames:

| CAN ID | Bus | Cycle | Purpose |
| --- | --- | --- | --- |
| 0x186 | Party + Vehicle | 10ms/100ms | DIS_frontTorque (front drive torque) |
| 0x187 | Party only | 10ms | Front DI status |
| 0x2D5 | Party only | 10ms | Front DI data |
| 0x1D5 | Vehicle only | 10ms | Front DI control |
| 0x2E5 | Vehicle only | 10ms | Front DI static |
| 0x195 | Vehicle only | 100ms | Front DI status |
| 0x1A5 | Vehicle only | 100ms | Front DI data |
| 0x27A | Vehicle only | 100ms | Front DI counter frame |
| 0x396 | Vehicle only | 100ms | DI_frontOilPump |
| 0x757 | Vehicle only | 100ms | Front DI info (with periodic variant) |

Key patterns: Various counter/checksum relationships (additive, subtractive), non-uniform counter step sizes (0x20 increments for 0x1D5), periodic variant frames (0x757 every 10th cycle).

## Relevance to Our Project

Valuable reference for Tesla Front Drive Inverter CAN messages and timing patterns. Demonstrates Party CAN vs Vehicle CAN bus separation and ECU emulation for component removal.

- **Reusability**: Medium
- **Key Takeaways**:
  - Front Drive Inverter CAN message IDs and frame structures
  - Party CAN vs Vehicle CAN bus message separation
  - Counter/checksum patterns: additive (+0x07), subtractive (-0x44), large step (+0x20)
  - Periodic variant frame pattern (0x757 alternates every 10 cycles)
  - DIS_frontTorque (0x186) appears on both buses at different rates
  - DI_frontOilPump (0x396) signal definitions
  - ESP32 TWAI CAN initialization pattern
