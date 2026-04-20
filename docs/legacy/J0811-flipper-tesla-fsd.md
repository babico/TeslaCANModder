---
title: J0811-flipper-tesla-fsd
description: A Flipper Zero application for Tesla FSD CAN bus unlocking. This is a fork of hypery11/flipper-tesla-fsd. It auto-detect
category: legacy
folder: legacy
tags: [legacy, community, external]
author: J0811
repo: flipper-tesla-fsd
---

# J0811-flipper-tesla-fsd

## Overview

A Flipper Zero application for Tesla FSD CAN bus unlocking. This is a fork of hypery11/flipper-tesla-fsd. It auto-detects HW3/HW4 via `GTW_carConfig` (0x398), modifies autopilot CAN frames to enable FSD, suppresses nag, and displays live status on the Flipper screen. Also includes BMS data sniffing (voltage, current, SoC, temperature) and a battery precondition trigger.

## Technical Details

- **Platform**: Flipper Zero (ARM Cortex-M4)
- **Language**: C
- **CAN Interface**: MCP2515 via Electronic Cats CAN Bus Add-On (SPI)
- **License**: GPL-3.0

## Architecture

- `tesla_fsd_app.c` / `tesla_fsd_app.h` — Main Flipper application entry point with scene manager, view dispatcher, and GUI setup.
- `fsd_logic/fsd_handler.c` / `fsd_handler.h` — Core CAN message handling logic. Clean C implementation with:
  - `FSDState` struct holding all runtime state (HW version, speed profile, FSD status, BMS data, OTA flag).
  - Per-CAN-ID handler functions for autopilot, follow distance, legacy stalk, ISA chime, nag killer, BMS, and GTW state.
  - Operation modes: Active (RX+TX), ListenOnly (passive sniff), Service (unrestricted).
- `scenes/` — Flipper UI scenes (main menu, running, settings).
- `scenes_config/` — Scene configuration and function tables.
- `libraries/` — MCP2515 driver adapted for Flipper Zero's SPI HAL.

## CAN Bus Integration

Extensive CAN bus integration covering many message IDs:

- **0x045 (69)** — `STW_ACTN_RQ`: Legacy follow-distance stalk reading
- **0x082 (130)** — `UI_tripPlanning`: Precondition heating trigger (TX)
- **0x132 (306)** — `BMS_hvBusStatus`: Pack voltage and current (read-only)
- **0x292 (658)** — `BMS_socStatus`: State of charge (read-only)
- **0x312 (786)** — `BMS_thermalStatus`: Battery temperature min/max (read-only)
- **0x318 (792)** — `GTW_carState`: OTA-in-progress detection (pauses TX during updates)
- **0x370 (880)** — `EPAS3P_sysStatus`: Nag killer via counter echo
- **0x398 (920)** — `GTW_carConfig`: Auto-detect HW3 vs HW4
- **0x399 (921)** — `DAS_status`: ISA speed chime suppression (HW4)
- **0x3EE (1006)** — Legacy autopilot control (bit 46 FSD, bit 19 nag)
- **0x3F8 (1016)** — Follow distance / speed profile mapping
- **0x3FD (1021)** — HW3/HW4 autopilot control (FSD enable, speed profile, nag suppression)

Safety features: TX paused during Tesla OTA updates, ListenOnly mode available.

## Relevance to Our Project

The most feature-complete FSD CAN implementation in the legacy collection. The clean C handler architecture, BMS sniffing, OTA-awareness, and operation modes are directly relevant.

- **Reusability**: High
- **Key Takeaways**:
  - Auto-detection of HW version from CAN ID 0x398
  - OTA-in-progress detection (0x318) to pause CAN TX — important safety feature
  - BMS data parsing (voltage, current, SoC, temperature) from standard Tesla CAN IDs
  - Precondition heating trigger via 0x082
  - Nag killer via EPAS counter echo method (0x370)
  - Clean state machine with operation modes
