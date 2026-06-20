---
title: linesoft2-tesla-fsd-can-mod-fork
description: A Chinese-localized fork of the Starmixcraft Tesla FSD CAN Mod, adapted for Arduino Nano + MCP2515 hardware instead of t
category: legacy
folder: legacy
tags: [legacy, community, external]
author: linesoft2
repo: tesla-fsd-can-mod-fork
---

# linesoft2-tesla-fsd-can-mod-fork

## Overview

A Chinese-localized fork of the Starmixcraft Tesla FSD CAN Mod, adapted for Arduino Nano + MCP2515 hardware instead of the original Adafruit Feather. The firmware intercepts and modifies CAN bus messages to enable Full Self-Driving (FSD) functionality on vehicles with a valid FSD entitlement. The README notes that as of 2026.3.31, Chinese vehicles have had FSD disabled at the gateway level, rendering this project ineffective for those vehicles.

## Architecture

```mermaid
flowchart LR
    Car["Tesla CAN"] --> Nano["Arduino Nano<br/>(MCP2515 SPI)"]
    Nano --> FSD["FSD enable<br/>(intercept + modify)"]
    classDef path fill:#1e3a5f,stroke:#4a7fb5,color:#fff
    class Nano,FSD path
```

## Technical Details

- **Platform**: Arduino Nano
- **Language**: C++ (Arduino)
- **CAN Interface**: MCP2515 via SPI (CS=D10, MISO=D12, MOSI=D11, SCK=D13)
- **License**: GPL-3.0 (inherited from original Starmixcraft project)

## Architecture

Single-file Arduino sketch (`CanFeather.ino`) containing all logic:

- `CarManagerBase` — base struct with virtual `handelMessage` method
- `LegacyHandler` — handles HW3 retrofit vehicles (CAN ID 1006)
- `HW3Handler` — handles HW3 vehicles (CAN IDs 1016, 1021)
- `HW4Handler` — handles HW4 vehicles (CAN IDs 1016, 1021, FSD v14)
- Hardware variant selected via `#define HW` at compile time

## CAN Bus Integration

- **CAN IDs monitored**: 1006 (Legacy), 1016 (UI driver assist control), 1021 (UI autopilot control), 69 (steering wheel action request)
- **FSD enable bit**: Bit 46 in the autopilot control frame
- **Nag suppression**: Bit 19 cleared in mux index 1
- **Speed profile**: Mapped from follow-distance stalk (3 levels) via bits in byte 6
- Reads mux ID from `frame.data[0] & 0x07`
- Checks FSD UI selection via `(frame.data[4] >> 6) & 0x01`

## Relevance to Our Project

Direct fork of the same Starmixcraft codebase that our project builds upon. The pin mapping and wiring documentation for Arduino Nano + MCP2515 is useful as a reference for alternative hardware configurations.

- **Reusability**: Medium
- **Key Takeaways**:
  - Arduino Nano + MCP2515 wiring reference (SPI pins, X179 connector pins 13/14)
  - Chinese vehicle gateway lockout information (2026.3.31 event)
  - Same core CAN bit manipulation logic as our handlers
