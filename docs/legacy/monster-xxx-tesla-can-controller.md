---
title: monster-xxx-tesla-can-controller
description: A multi-device ESP32-based Tesla Model Y control system that uses encrypted BLE to coordinate between three devices - an 
category: legacy
folder: legacy
tags: [legacy, community, external]
author: monster
repo: xxx-tesla-can-controller
---

# monster-xxx-tesla-can-controller

## Overview

A multi-device ESP32-based Tesla Model Y control system that uses encrypted BLE to coordinate between three devices: an M5Dial controller with LVGL touchscreen UI, an ESP32-C6 CAN reader with dual SN65HVD230 transceivers, and an ESP32-based WS2812 ambient light controller. Provides vehicle monitoring (speed, battery, tire pressure), advanced controls (light control, drift mode via ESP disable, battery preheating), 0-100 km/h performance testing, and ambient lighting effects.

## Technical Details

- **Platform**: ESP32-S3 (M5Dial), ESP32-C6 (CAN reader), ESP32 (ambient light)
- **Language**: C++ (ESP-IDF framework)
- **CAN Interface**: Dual SN65HVD230 transceivers on ESP32-C6 (CAN0 500kbps, CAN1 125kbps) via TWAI
- **License**: MIT (Copyright 2026 monster-xxx)

## Architecture

The repository contains documentation and a planned project structure (the `docs/` directory has architecture and hardware specs). The project is organized as a three-device system:

- **Device A (M5Dial)**: Main controller with LVGL UI, rotary encoder, touch screen — acts as BLE central
- **Device B (ESP32-C6)**: CAN bus reader with dual-channel CAN (high-speed 500kbps + low-speed 125kbps), FreeRTOS-based — acts as BLE peripheral
- **Device C (ESP32)**: Ambient light controller with 12V WS2812 RGB strip, PWM dimming — acts as BLE peripheral
- Communication: AES-256-GCM encrypted BLE between all devices
- `docs/architecture.md` — Detailed system architecture, hardware interfaces, communication topology
- `docs/hardware_spec.md` — Hardware specifications and BOM

## CAN Bus Integration

- Dual CAN channel design: CAN0 at 500kbps (powertrain) and CAN1 at 125kbps (body/infotainment)
- Uses SN65HVD230 transceivers (2x) connected to ESP32-C6 TWAI peripheral
- Vehicle data decoded: speed, battery SOC, gear position, tire pressure
- Control functions: light commands, ESP disable (drift mode), battery preheating
- Connected via OBD-II port to Tesla CAN hub

## Relevance to Our Project

Demonstrates an ambitious multi-device BLE-connected CAN control architecture. The dual-CAN-channel approach (high-speed + low-speed) and encrypted BLE communication pattern are valuable architectural references. The M5Dial UI and ambient light integration show advanced use cases beyond basic CAN modification.

- **Reusability**: Medium
- **Key Takeaways**:
  - Dual CAN channel architecture (500kbps powertrain + 125kbps body)
  - AES-256-GCM encrypted BLE communication between devices
  - SN65HVD230 CAN transceiver integration with ESP32-C6 TWAI
  - LVGL-based touchscreen UI on M5Dial for vehicle control
  - Performance testing (0-100 km/h) implementation
  - Ambient light control via WS2812 RGB strips
  - Primarily documentation/design — check if actual firmware code is present beyond docs
