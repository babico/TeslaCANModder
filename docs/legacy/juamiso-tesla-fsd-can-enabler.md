---
title: juamiso-tesla-fsd-can-enabler
description: A multi-board Tesla FSD CAN bus enabler that intercepts and modifies specific CAN messages to activate Full Self-Driving
category: legacy
folder: legacy
tags: [legacy, community, external]
author: juamiso
repo: tesla-fsd-can-enabler
---

# juamiso-tesla-fsd-can-enabler

## Overview

A multi-board Tesla FSD CAN bus enabler that intercepts and modifies specific CAN messages to activate Full Self-Driving functionality. Supports four board variants: ESP32+MCP2515, ESP32-S3 TWAI, Adafruit Feather M4 CAN, and RP2040 CAN. (An Arduino UNO+MCP2515 variant is documented in the README but the sketch is not present in the repository.) Derived from the original Starmixcraft/Tesla-OPEN-CAN-MOD project with expanded hardware support.

## Technical Details

- **Platform**: ESP32, ESP32-S3, Adafruit Feather M4 (SAMD51), RP2040, Arduino UNO
- **Language**: C++ (Arduino IDE)
- **CAN Interface**: MCP2515 (SPI), MCP25625 (SPI on Feather boards), ESP32-S3 built-in TWAI
- **License**: GPL v3 (declared in source file headers)

## Architecture

```
boards/
├── ESP32_MCP2515/ESP32_MCP2515.ino    — ESP32 + external MCP2515 via SPI
├── ESP32S3_TWAI/ESP32S3_TWAI.ino      — ESP32-S3 native TWAI CAN
├── FeatherM4CAN/FeatherM4CAN.ino      — Adafruit Feather M4 + MCP25625
└── RP2040CAN/RP2040CAN.ino            — RP2040 + MCP25625
```

(The README also lists `UNO_MCP2515_CAN/` as a planned Arduino UNO variant, but that directory is not present in the repository.)

Each sketch uses a compile-time `#define HW` to select vehicle hardware variant (LEGACY/HW3/HW4). The handler classes (`LegacyHandler`, `HW3Handler`, `HW4Handler`) implement variant-specific CAN message processing.

## CAN Bus Integration

Intercepts and re-transmits specific CAN frames at 500 kbps:

| Variant | CAN IDs | Action |
| ------- | ------- | ------ |
| LEGACY (HW3 Retrofit) | 69 (0x045), 1006 (0x3EE) | Read stalk position; set FSD enable bit 46; write speed profile; clear nag bit 19 |
| HW3 | 1016 (0x3F8), 1021 (0x3FD) | Read follow-distance; set FSD enable bit 46; speed profile; nag suppression; speed offset |
| HW4 | 1016, 1021 | Same as HW3 plus bit 60 (FSD V14), bit 47, 5-level speed profiles |

Key bit manipulations: FSD enable (bit 46), nag suppression (bit 19), speed profile in byte 6 bits 1-2, and HW4-specific FSD V14 bit 60.

## Relevance to Our Project

Directly relevant — this is a more complete, multi-board version of the FSD CAN mod pattern. Provides the most comprehensive reference for CAN IDs, bit positions, and hardware variant handling.

- **Reusability**: High
- **Key Takeaways**:
  - Complete CAN ID and bit-level mapping for FSD activation across HW3/HW4
  - Multi-board architecture pattern (ESP32, SAMD51, RP2040, UNO)
  - ESP32-S3 native TWAI implementation without external MCP2515
  - Follow-distance stalk → speed profile mapping logic
  - HW4 FSD V14 additional bits and emergency vehicle detection
