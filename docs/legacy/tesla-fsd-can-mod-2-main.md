---
title: tesla-fsd-can-mod-2-main
description: A Tesla FSD CAN bus enabler firmware with two hardware variants - one for RP2040-based Adafruit Feather CAN boards and on
category: legacy
folder: legacy
tags: [legacy, community, external]
author: tesla
repo: fsd-can-mod-2-main
---

# tesla-fsd-can-mod-2-main

## Overview

A Tesla FSD CAN bus enabler firmware with two hardware variants: one for RP2040-based Adafruit Feather CAN boards and one for Arduino UNO with MCP2515 shields. It intercepts autopilot-related CAN frames and modifies specific bits to enable FSD functionality, including nag suppression and speed profile mapping. Includes a Korean-language README. This is an evolution of the tesla-fsd-can-mod-main repo with added UNO support and HW4/FSDV14 features.

## Technical Details

- **Platform**: Adafruit Feather RP2040 CAN / Arduino UNO
- **Language**: C++ (Arduino)
- **CAN Interface**: MCP2515 over SPI (both variants)
- **License**: GPL-3.0

## Architecture

- `RP2040CAN.ino` — Main sketch for RP2040 Feather boards using MCP2515. Contains all handler structs (LegacyHandler, HW3Handler, HW4Handler) as inline code with compile-time HW selection via `#define HW`
- `UNO_MCP2515_CAN.ino` — Arduino UNO variant with the same CAN logic but adapted for UNO pin mappings (CS=10, INT=2, LED=13). Supports configurable MCP2515 clock (8MHz/16MHz)
- `README.ko.md` — Korean-language installation and usage guide
- Polymorphic handler pattern: `CarManagerBase` base struct with virtual `handleMessage()`, each HW variant has its own derived struct

Key helpers: `readMuxID()`, `isFSDSelectedInUI()`, `setSpeedProfileV12V13()`, `setBit()` — shared bit-manipulation utilities for CAN frame modification.

## CAN Bus Integration

Listens and modifies specific CAN IDs at 500 kbps:

| Variant | CAN IDs | Actions |
| ------- | ------- | ------- |
| Legacy (HW3 Retrofit) | 1006 | Mux 0: set FSD enable bit 46, speed profile bits 49-50; Mux 1: clear nag bit 19 |
| HW3 | 1016, 1021 | 1016: read follow-distance (bits 45-47); 1021 mux 0: FSD enable + speed profile; mux 1: suppress nag; mux 2: inject offset |
| HW4 | 1016, 1021 | Same as HW3 plus: bit 59 (emergency vehicle detection), bit 60 (V14 enable), mux 1 bit 47 (summon), mux 2 bits 60-62 (5-level speed profile) |

FSD activation is triggered by detecting "Traffic Light and Stop Sign Control" enabled in UI (bit check on CAN frame). Speed profile is derived from follow-distance stalk setting.

## Relevance to Our Project

Core FSD enabler logic with multi-platform support — directly relevant as this represents an earlier version of the same concept our project builds on.

- **Reusability**: High
- **Key Takeaways**:
  - UNO variant shows the logic works on very low-cost 8-bit hardware with external MCP2515
  - Clean polymorphic handler pattern for multi-HW targets
  - HW4 V14 additions (emergency vehicle detection, summon enable) are documented inline
  - Korean README shows international community interest
  - Configurable MCP2515 clock speed (8/16 MHz) in UNO variant is a useful detail
