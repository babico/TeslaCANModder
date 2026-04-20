---
title: iubns-tesla-fsd-can-mod
description: A fork/variant of the CanFeather Tesla FSD CAN bus enabler. Provides firmware for MCP2515-based boards (Adafruit Feather
category: legacy
folder: legacy
tags: [legacy, community, external]
author: iubns
repo: tesla-fsd-can-mod
---

# iubns-tesla-fsd-can-mod

## Overview

A fork/variant of the CanFeather Tesla FSD CAN bus enabler. Provides firmware for MCP2515-based boards (Adafruit Feather RP2040 CAN and Arduino UNO) that intercepts and modifies Tesla CAN bus messages to enable Full Self-Driving (FSD) functionality, including speed profile control and nag suppression.

## Technical Details

- **Platform**: Adafruit Feather RP2040 CAN / Arduino UNO with MCP2515
- **Language**: C++ (Arduino)
- **CAN Interface**: MCP2515 over SPI (500 kbit/s)
- **License**: GPL-3.0

## Architecture

- `RP2040CAN.ino` — Main firmware for Adafruit Feather RP2040 CAN. Uses polymorphic handler pattern (`LegacyHandler`, `HW3Handler`, `HW4Handler`) selected at compile time via `#define HW`.
- `UNO_MCP2515_CAN.ino` — Port for standard Arduino UNO + MCP2515 shield. Same logic adapted for AVR constraints without smart pointers.
- Both files share identical CAN message manipulation logic (bit-level operations on autopilot control frames).

## CAN Bus Integration

- **CAN ID 1006 (0x3EE)** — Legacy: reads FSD state from UI, sets bit 46 (FSD enable), writes speed profile to byte 6 bits 1–2, clears bit 19 (nag suppression).
- **CAN ID 1016 (0x3F8)** — HW3/HW4: reads follow-distance setting to map speed profiles.
- **CAN ID 1021 (0x3FD)** — HW3/HW4: modifies autopilot control frames — FSD enable (bit 46), nag suppression (bit 19), speed offset, and HW4-specific bits (bit 60 FSD V14, bit 59 emergency vehicle detection).
- Activation trigger: "Traffic Light and Stop Sign Control" toggle in vehicle UI.

## Relevance to Our Project

Direct predecessor/sibling of the core FSD CAN mod logic. The UNO variant is useful as a reference for MCP2515-only implementations.

- **Reusability**: High
- **Key Takeaways**:
  - Polymorphic handler pattern for HW3/HW4/Legacy variants
  - Specific CAN bit manipulation for FSD enable, speed profile, and nag suppression
  - UNO variant shows how to avoid RP2040-specific APIs
