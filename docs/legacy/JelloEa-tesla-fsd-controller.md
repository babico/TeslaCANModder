---
title: JelloEa-tesla-fsd-controller
description: An ESP32-based Tesla FSD CAN controller with a built-in WiFi access point and web UI for real-time configuration. Users 
category: legacy
folder: legacy
tags: [legacy, community, external]
author: JelloEa
repo: tesla-fsd-controller
---

# JelloEa-tesla-fsd-controller

## Overview

An ESP32-based Tesla FSD CAN controller with a built-in WiFi access point and web UI for real-time configuration. Users connect to the ESP32's hotspot via phone/browser to change HW mode, speed profile, ISA chime suppression, and other settings without re-flashing firmware. Includes OTA firmware update capability.

## Technical Details

- **Platform**: ESP32 (ESP32-DevKitC)
- **Language**: C++ (Arduino/PlatformIO)
- **CAN Interface**: ESP32 native TWAI peripheral with external SN65HVD230 transceiver (GPIO 5 TX, GPIO 4 RX)
- **License**: GPL-3.0

## Architecture

- `src/main.cpp` — Main application split across two ESP32 cores:
  - **Core 0**: WiFi AP, AsyncWebServer (status JSON API, config API, OTA upload endpoint), NVS config persistence.
  - **Core 1**: CAN bus read/modify/write loop using TWAI driver.
- `include/` — Shared headers:
  - `can_frame_types.h` — CAN frame type definitions
  - `drivers/twai_driver.h` — ESP32 TWAI driver wrapper
  - `handlers.h` — CAN message handlers (same FSD logic as Tesla-Open-CAN-Mod)
  - `web_ui.h` — Embedded HTML for the web control panel
- Config stored in ESP32 NVS (Non-Volatile Storage) via Preferences library.
- Web API endpoints: `/api/status` (JSON stats), `/api/set` (parameter changes), `/api/ota` (firmware upload).
- PlatformIO build with ESPAsyncWebServer and AsyncTCP dependencies.

## CAN Bus Integration

Same CAN message handling as Tesla-Open-CAN-Mod:

- **0x399 (921)** — ISA speed chime suppression (HW4, optional)
- **0x3F8 (1016)** — Follow distance reading for speed profile mapping
- **0x3FD (1021)** — Autopilot control: FSD enable (bit 46), FSD V14 (bit 60), emergency vehicle detection (bit 59), nag suppression (bit 19), speed profile
- Supports Legacy, HW3, and HW4 modes selectable via web UI.
- Includes "China Mode" option for region-specific behavior.

## Relevance to Our Project

Highly relevant — demonstrates the WiFi web UI control pattern for runtime configuration that our project's web interface follows. The dual-core architecture (network on Core 0, CAN on Core 1) is a good pattern for real-time CAN processing.

- **Reusability**: High
- **Key Takeaways**:
  - Dual-core ESP32 architecture: WiFi/web on Core 0, CAN on Core 1
  - Runtime configuration via web API without re-flashing
  - NVS-based config persistence across reboots
  - OTA firmware update over WiFi
  - Input validation on web API parameters
  - Flag-based restart pattern for OTA (avoids delay in async context)
