---
title: yoziru/esphome-tesla-ble
description: ESPHome custom component for ESP32 devices that communicates with Tesla vehicles over Bluetooth Low Energy (BLE) using t
category: legacy
folder: legacy
tags: [legacy, community, external]
author: yoziru
repo: esphome-tesla-ble
---

# yoziru/esphome-tesla-ble

## Overview

ESPHome custom component for ESP32 devices that communicates with Tesla vehicles over Bluetooth Low Energy (BLE) using the `yoziru/tesla-ble` library. It enables charging management, vehicle status monitoring, and control (locks, trunk, climate, windows, etc.) through Home Assistant. Tested with M5Stack NanoC6 and Tesla firmware 2024.26.3.1+.

## Technical Details

- **Platform**: ESP32 (ESP-IDF 5.3.0 via PlatformIO 6.8.1), ESPHome framework
- **Language**: C++ (custom components), Python (ESPHome codegen/config validation), YAML (device configuration)
- **CAN Interface**: N/A — uses BLE (Bluetooth Low Energy), not CAN bus
- **License**: AGPL-3.0 (GNU Affero General Public License v3)

## Architecture

The project is structured as ESPHome custom components with YAML-based configuration:

- **`components/tesla_ble_vehicle/`** — Main component that handles BLE communication with the vehicle:
  - `tesla_ble_vehicle.h/.cpp` — Core component class (`TeslaBLEVehicle`), extends ESPHome `PollingComponent` and `BLEClientNode`. Manages BLE connection, GATT events, and command dispatch.
  - `vehicle_state_manager.h/.cpp` — Manages vehicle state including sensors, switches, numbers, locks, covers, and climate. Receives protobuf callbacks (VCSEC, CarServer) and updates ESPHome entities.
  - `ble_adapter_impl.h` / `storage_adapter_impl.h` — Adapter implementations for the tesla-ble library (BLE I/O and key storage).
  - `command_builder.h` — Helper for building vehicle commands.
  - `__init__.py` — ESPHome component registration, config validation, and C++ codegen. Defines all entity classes (buttons, switches, locks, covers, climate, sensors).
- **`components/tesla_ble_listener/`** — BLE scanner that discovers Tesla vehicles by matching VIN-derived advertisement names to find the BLE MAC address.
- **`packages/`** — Modular YAML configs: `base.yml` (ESPHome device config), `client.yml` (BLE client + tesla_ble_vehicle setup with polling intervals), `common.yml`, `board.yml`, `listener.yml`, `project.yml`.
- **`boards/`** — Board-specific YAML configs for ESP32 Generic, M5Stack AtomS3, M5Stack NanoC6.
- **`tesla-ble-*.yml`** — Top-level configs for each board variant (build entry points).

Key protocol details: Uses Tesla BLE service UUID `00000211-b2d1-43f0-9b88-960cebf8b91e` with separate read/write GATT characteristics. Vehicle state is received via protobuf-encoded callbacks (VCSEC for security status, CarServer for charge/climate/drive/tire/closures state).

## CAN Bus Integration

No direct CAN integration. This project communicates with the Tesla vehicle exclusively over BLE using Tesla's proprietary BLE protocol (protobuf-based VCSEC and CarServer messages). It does not read or write CAN bus messages.

## Relevance to Our Project

This repo represents an alternative communication path to Tesla vehicles — BLE instead of CAN. It provides valuable reference for:

- Understanding Tesla's protobuf message structures (VCSEC_VehicleStatus, CarServer_ChargeState, CarServer_ClimateState, CarServer_DriveState, CarServer_TirePressureState, CarServer_ClosuresState), which may overlap with CAN-decoded data fields.
- The ESPHome component architecture pattern could inform how we structure Home Assistant integration for our CAN-based project.
- IEC 61851 charge state mapping logic is directly reusable.

- **Reusability**: Low — BLE-only, no CAN bus overlap in communication layer
- **Key Takeaways**:
  - Tesla protobuf message schemas (charge state, climate, drive, closures) provide a reference for what data fields are available and their semantics
  - ESPHome custom component pattern (C++ component + Python codegen + YAML config) is a well-structured model for Home Assistant integration
  - IEC 61851 state mapping from Tesla charge states is directly applicable
  - Smart polling with sleep-awareness (11-min wake window, active vs. awake intervals) is a useful pattern for any Tesla polling implementation
  - BLE key pairing workflow and VCSEC security protocol documentation are useful context for understanding Tesla's security model

## Upstream (2026-06-20)

8 new commits on `main` (v2026.6.7 → v2026.6.8):

- `yoziru/tesla-ble` library bump 5.0.6 → 5.1.1 (`1de8ac2`).
- `command_builder.h` removed; command logic moved inline to `tesla_ble_vehicle.cpp` (`5154bf2`).
- `send_command_result` now tracks command **phase** and **outcome** (`bcf5ffb`).
- `charger_phases` sensor from `ChargeState` proto (`c194917`).
- CI: PlatformIO package cache + stale-run cancel (`ebc40af`).
- README rewrite: quick-start, install methods, role docs (`f935126`).

Net: 660 lines deleted, 486 added across 14 files. The component shed the `command_builder.h` indirection and grew inline command logic. The underlying `yoziru/tesla-ble` library 5.0.6→5.1.1 is the most relevant change for anyone re-syncing the embedded dependency. Full commit table: `docs/legacy/upstream-review-2026-06-20.md`.
