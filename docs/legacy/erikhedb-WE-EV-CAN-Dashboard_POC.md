---
title: erikhedb-WE-EV-CAN-Dashboard_POC
description: A proof-of-concept EV information system built around an Orion BMS 2 and Tesla Model 3 Rear Drive Unit (RDU) connected t
category: legacy
folder: legacy
tags: [legacy, community, external]
author: erikhedb
repo: WE-EV-CAN-Dashboard_POC
---

# erikhedb-WE-EV-CAN-Dashboard_POC

## Overview

A proof-of-concept EV information system built around an Orion BMS 2 and Tesla Model 3 Rear Drive Unit (RDU) connected to a Raspberry Pi 5 via SocketCAN. Features a Python-based CAN logger/parser for Orion BMS messages and a Flask web dashboard for real-time visualization of pack voltage, current, SOC, cell voltages, temperatures, and relay states. The author notes this codebase is being retired in favor of a Go-based rewrite.

## Technical Details

- **Platform**: Raspberry Pi 5 with PiCAN2 Duo (or similar CAN HAT)
- **Language**: Python (uv-managed project), HTML/JavaScript (dashboard)
- **CAN Interface**: SocketCAN via PiCAN2 Duo HAT on Raspberry Pi
- **License**: None

## Architecture

- `logger/main.py` — CAN logger entry point (stub — mostly in can_parser.py)
- `logger/can_parser.py` — Comprehensive Orion BMS CAN message parser with DBC-based signal definitions for CAN IDs 0x6B0–0x6B4 and extended IDs (0x1806E7F4, 0x1806E5F4, 0x1806E9F4, 0x18FF50E5)
- `logger/data/` — JSON output directory for parsed BMS data
- `web/web_app.py` — Flask web server that reads parsed JSON data and renders a dashboard
- `web/dashboard.html` — HTML dashboard template showing pack metrics, cell data, temperatures
- `sample/` — Sample data files
- `docs/` — Architecture, hardware, Linux CAN setup, and project structure documentation
  - `docs/architecture.md`
  - `docs/hardware.MD`
  - `docs/linux-can.md`
  - `docs/project_structure.md`
- `pyproject.toml` / `uv.lock` — Python dependency management via uv

## CAN Bus Integration

Direct CAN integration focused on Orion BMS (not Tesla vehicle CAN bus):

**Orion BMS CAN Messages:**

- 0x6B0: Pack current, instantaneous voltage, SOC%, relay state
- 0x6B1: Pack DCL/CCL (discharge/charge current limits), high/low temperatures
- 0x6B2: High/low cell voltages and IDs, populated cell count
- 0x6B3: Pack CCL/DCL, failsafe statuses
- 0x6B4: J1772 AC power/current limits, plug state
- Extended frames (0x1806E7F4, 0x1806E5F4, 0x1806E9F4): Maximum pack/cell voltage, charger relay DTC

The parser uses a DBC-style signal definition system with bitfield extraction (start bit, length, factor, offset) for each signal.

## Relevance to Our Project

Moderately relevant — demonstrates a complete CAN logging/parsing/dashboard pipeline on Raspberry Pi, though the specific CAN messages are for Orion BMS (aftermarket) rather than Tesla's native CAN bus.

- **Reusability**: Medium
- **Key Takeaways**:
  - Clean Python CAN parser architecture with dataclass-based signal definitions
  - DBC-style signal definition pattern (CANSignal with start_bit, length, factor, offset)
  - Flask dashboard for real-time CAN data visualization
  - Raspberry Pi + SocketCAN (PiCAN2) hardware integration pattern
  - Orion BMS CAN message definitions (useful if integrating aftermarket BMS)
  - uv-based Python project management
  - Separate logger and web components with JSON data bridge
