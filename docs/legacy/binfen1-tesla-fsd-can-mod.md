---
title: binfen1-tesla-fsd-can-mod
description: Arduino firmware for enabling Tesla Full Self-Driving (FSD) via CAN bus message interception. Runs on an Adafruit Feathe
category: legacy
folder: legacy
tags: [legacy, community, external]
author: binfen1
repo: tesla-fsd-can-mod
---

# binfen1-tesla-fsd-can-mod

## Overview

Arduino firmware for enabling Tesla Full Self-Driving (FSD) via CAN bus message interception. Runs on an Adafruit Feather or ESP32-C3 with an MCP2515 CAN controller. It listens for Autopilot-related CAN frames, sets FSD enable bits, maps follow-distance to speed profiles, and suppresses the hands-on-wheel nag.

## Technical Details

- **Platform**: ESP32-C3 Super Mini / Adafruit Feather M4 CAN (MCP2515-based)
- **Language**: C++ (Arduino)
- **CAN Interface**: MCP2515 via SPI at 500 kbit/s
- **License**: GPL-3.0

## Architecture

Single-file firmware (`CanFeather.ino`):

- `CarManagerBase` — Base struct with virtual `handelMessage()` for CAN frame processing
- `HW4Handler` — Handles CAN IDs 1016 (follow-distance/speed profile) and 1021 (FSD enable bits, nag suppression, speed profile injection). Extends `CarManagerBase`
- Hardware variant selection via `#define HW` preprocessor directive (LEGACY, HW3, HW4)
- SPI pin configuration for ESP32-C3 Super Mini (SCK=4, MISO=5, MOSI=6, CS=7)
- Debug output over Serial at 115200 baud

## CAN Bus Integration

Directly intercepts and modifies specific Tesla CAN frames at 500 kbit/s:

- **CAN ID 1016**: Reads follow-distance stalk setting (bits [7:5] of byte 5), maps to 5-level speed profile
- **CAN ID 1021**: Multiplexed frame; uses mux index from bits [2:0] of byte 0:
  - Mux 0: Checks if "Traffic Light and Stop Sign Control" is enabled (bit 6 of byte 4), sets FSD enable bits at positions 46 and 60
  - Mux 1: Clears nag bit at position 19, sets bit 47
  - Mux 2: Injects speed profile into bits [6:4] of byte 7
- Re-transmits modified frames via `mcp->sendMessage()`

## Relevance to Our Project

Directly relevant — this is a Tesla FSD CAN mod with the same core goal. The CAN ID handling, bit manipulation patterns, and hardware variant support are immediately applicable reference material.

- **Reusability**: High
- **Key Takeaways**:
  - Concrete CAN ID 1016/1021 bit-level protocol for FSD activation on HW3/HW4
  - MCP2515 polling-mode SPI integration on ESP32-C3
  - Multiplexed frame handling pattern (mux index from low bits of byte 0)
  - Speed profile mapping from follow-distance stalk
  - Nag suppression bit location (position 19 in mux 1 of ID 1021)
  - Hardware variant abstraction via compile-time `#define`
