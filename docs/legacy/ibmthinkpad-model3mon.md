---
title: ibmthinkpad / model3mon
description: A Teensy-based Tesla Model 3 battery monitoring system that reads BMS (Battery Management System) data from the vehicle 
category: legacy
folder: legacy
tags: [legacy, community, external]
author: ibmthinkpad
repo: model3mon
---

# ibmthinkpad / model3mon

## Overview

A Teensy-based Tesla Model 3 battery monitoring system that reads BMS (Battery Management System) data from the vehicle CAN bus and displays it via USB serial and an optional Nextion HMI touchscreen display. It decodes multiple CAN messages to show cell temperatures, brick voltages, state of charge, current draw, charge/discharge totals, and capacity metrics.

## Technical Details

- **Platform**: Teensy 3.2
- **Language**: C++ (Arduino)
- **CAN Interface**: FlexCAN (Teensy built-in CAN controller)
- **License**: MIT

## Architecture

- `m3mon.ino` — Main sketch: CAN bus initialization (500 kbps), message receive loop, CAN frame decoding with bitwise extraction of BMS signals, debug serial output
- `bmsvalues.h` — `BMSValues` class: data container for all BMS telemetry (cell temps, brick voltages, SOC, voltage, current, charge totals, capacity)
- `nexdisplay.cpp` / `nexdisplay.h` — Nextion HMI display driver: serializes `BMSValues` to Nextion display commands via UART
- `m3mon.hmi` / `m3mon.tft` — Nextion Editor project and compiled display firmware
- `hmifont.zi` / `hmifont2.zi` — Custom fonts for the Nextion display

## CAN Bus Integration

Read-only CAN bus integration focused on BMS data extraction at 500 kbps:

| CAN ID | Name | Signals Decoded |
| ------ | ---- | --------------- |
| `0x332` | ID332BattCellMinMax | Cell temp max/min (number & value), brick voltage max/min (number & value), model temp max/min — multiplexed (mux 0 = temps, mux 1 = voltages) |
| `0x3B2` | ID3B2BMS_log2 | CAC avg/min/max (Ah capacity), total charge/discharge Ah — multiplexed (mux 0, mux 6) |
| `0x132` | ID132HVBattAmpVolt | Pack voltage (0.01V scale), smooth current (-0.1A scale), raw current (-0.05A + 500 offset) |
| `0x2D2` | BMS voltage limits | Configured min/max voltage (0.01V scale) |
| `0x3D2` | TotalChargeDischarge | Lifetime charge/discharge kWh (0.001 kWh scale, 32-bit) |
| `0x292` | ID292BMS_SOC | UI SOC %, average SOC % (0.1% scale, 10-bit) |
| `0x3F2` | BMS counters | Total AC charge kWh, total DC charge kWh — multiplexed |

All signal decoding uses bitwise operations on the raw 64-bit CAN frame with DBC-style scale/offset comments inline.

## Relevance to Our Project

Moderate relevance. The BMS CAN message decoding is directly useful for our BMS dashboard feature. The signal definitions with scale/offset comments serve as a verified reference for Model 3 BMS CAN IDs.

- **Reusability**: Medium — BMS signal definitions are directly reusable; Nextion display code is platform-specific
- **Key Takeaways**:
  - Complete Model 3 BMS CAN message decoding with DBC-format annotations in comments
  - Multiplexed message handling pattern (checking mux index before decoding sub-signals)
  - Signal scale/offset values verified against community DBC: `0x332` cell voltages at 0.002V, temps at 0.5°C - 40
  - FlexCAN library usage on Teensy for native CAN (no external transceiver needed)
  - `BMSValues` data model is a clean reference for what BMS telemetry is available on Model 3
  - CAC (Capacity of Cells) metrics: avg/min/max in Ah — useful for battery health monitoring
  - Lifetime charge/discharge counters in both kWh and Ah
