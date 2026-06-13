---
title: bobmorane83-TeslaCam
description: A wireless auxiliary dashboard system for Tesla Model 3, composed of three independent ESP32-S3 modules - a CAN bridge (L
category: legacy
folder: legacy
tags: [legacy, community, external]
author: bobmorane83
repo: TeslaCam
---

# bobmorane83-TeslaCam

## Overview

A wireless auxiliary dashboard system for Tesla Model 3, composed of three independent ESP32-S3 modules: a CAN bridge (LilyGo T-2CAN), a round LVGL display (JC3636W518C, 360×360), and a front camera (Freenove ESP32-S3 CAM). The CAN bridge reads Tesla CAN bus data and transmits it wirelessly via ESP-NOW to the display, which shows speed, battery SOC, gear, power, temperatures, turn signals, blind spot alerts, and a live front camera feed. Total hardware cost ~75 €.

## Technical Details

- **Platform**: ESP32-S3 (three boards: LilyGo T-2CAN, JC3636W518C display, Freenove CAM)
- **Language**: C++ (PlatformIO/Arduino framework)
- **CAN Interface**: MCP2515 (SPI) + ESP32-S3 TWAI (built-in CAN) on LilyGo T-2CAN, dual-bus
- **License**: None

## Architecture

- `Bridge/src/main.cpp` — CAN bridge firmware for LilyGo T-2CAN: reads from both MCP2515 (VehicleBus) and TWAI (ChassisBus) at 500 kbit/s, filters frames via whitelist (`valid_can_ids.h`), broadcasts via ESP-NOW
- `Ecran/firmware/src/main.cpp` — Display firmware for JC3636W518C: LVGL dashboard on 360×360 round screen, ESP-NOW receiver for CAN data, UDP receiver for JPEG camera stream, touch-to-toggle camera/dashboard mode
- `Camera/` — Camera firmware for Freenove ESP32-S3 WROOM CAM: captures JPEG frames, streams via WiFi UDP
- `Can/` — Tesla Model 3 DBC files (`tesla_can.dbc`, `Model3CAN.dbc`, `tesla_can2.dbc`) plus exhaustive signal synthesis document
- `Can/SIGNAL_SYNTHESIS.md` — Comprehensive analysis of ~170 CAN messages and ~2900 signals across VehicleBus and ChassisBus
- `auto_port.py` — PlatformIO script for auto-detecting ESP32 upload port by MAC address

## CAN Bus Integration

Comprehensive Tesla Model 3 CAN integration via dual-bus setup:

- **VehicleBus** (MCP2515 SPI): Standard vehicle telemetry
- **ChassisBus** (TWAI native): Chassis dynamics, steering, braking
- Filters CAN IDs via a compiled whitelist
- Decodes: speed, battery SOC, range, gear (P/R/N/D), front+rear motor power (kW), battery/cabin/exterior temperatures, turn signals, blind spot warnings, brake status, time, destination SOC
- DBC files contain ~159 messages with ~2752 signals
- Uses ESP-NOW broadcast for wireless CAN data relay (no wiring in cabin)

## Relevance to Our Project

Highly relevant — a complete, well-architected Tesla CAN bus project with dual-bus support, comprehensive signal documentation, and wireless architecture. The DBC files and signal synthesis document are particularly valuable references.

- **Reusability**: High
- **Key Takeaways**:
  - Dual CAN bus architecture (MCP2515 + TWAI) on a single ESP32-S3
  - ESP-NOW for low-latency wireless CAN data relay
  - Comprehensive Tesla Model 3 DBC files with ~2752 signals
  - SIGNAL_SYNTHESIS.md is an exhaustive CAN signal reference document
  - LilyGo T-2CAN hardware platform for dual-bus CAN bridge
  - Valid CAN ID whitelist approach for filtering
  - Auto-port detection by MAC address prevents cross-flashing
  - LVGL-based round display dashboard pattern
  - Brake light anti-flicker: timestamp-only hold timer (600ms), refresh on ON only
  - Speed limit tolerance arc sizing and positioning aligned with firmware constants
  - Mockup/firmware color alignment: teal accent (#00e5c8), gear display bottom-right, status dots, clock/temp area
