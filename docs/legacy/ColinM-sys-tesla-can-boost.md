---
title: ColinM-sys-tesla-can-boost
description: A comprehensive Tesla Model 3/Y CAN bus toolkit featuring real-time reading/decoding of 300+ signals, CAN write capabili
category: legacy
folder: legacy
tags: [legacy, community, external]
author: ColinM
repo: sys-tesla-can-boost
---

# ColinM-sys-tesla-can-boost

## Overview

A comprehensive Tesla Model 3/Y CAN bus toolkit featuring real-time reading/decoding of 300+ signals, CAN write capabilities (horn, drive mode), a "ghost mode" that overrides drive mode via CAN injection, a live web dashboard, and drive recording/analysis tools. All communication is done over a Bluetooth OBD2 adapter (OBDLink MX+) using ELM327/STN2120 AT commands from Python scripts.

## Technical Details

- **Platform**: PC/Laptop (Bluetooth to OBDLink MX+)
- **Language**: Python 3.10+, HTML/JavaScript (dashboard)
- **CAN Interface**: OBDLink MX+ (STN2120/ELM327-compatible Bluetooth OBD2 adapter) via serial
- **License**: CC BY-NC 4.0 (Creative Commons Attribution-NonCommercial)

## Architecture

- `tools/pedalmap_v2.py` — Ghost mode: injects CAN ID 0x334 with Performance pedal map at 50ms intervals
- `tools/ghost_mode.py` — Advanced ghost mode with simultaneous data logging and mode selection
- `tools/ghost_ui.py` — UI-based ghost mode controller
- `tools/ghost3d.py` — 3D visualization ghost mode variant
- `tools/gamepad_throttle.py` — Gamepad-driven throttle control via CAN injection (latest session scripts)
- `tools/can_decode.py` — CAN frame decoder with 30+ known Tesla Model 3 signal definitions
- `tools/can_capture.py` — Raw CAN traffic capture
- `tools/live_sniffer.py` — Real-time signal change detection
- `tools/can_write_test.py` — Interactive CAN write testing (horn, drive mode, etc.)
- `tools/dashboard_server.py` — WebSocket server pushing live CAN data to a web-based dashboard
- `tools/dashboard.html` / `tools/performance_dash.html` — Web UIs for real-time visualization
- `tools/drive_recorder.py` — Logs all CAN traffic during drives
- `tools/analyze_drive.py` / `tools/analyze_unknown.py` — Offline analysis of recorded drives
- `tools/scan_ports.py` — Find OBDLink adapter COM port
- `tools/test_connection.py` — Verify adapter communication
- `tools/pedalmap_test.py` — Basic drive mode injection test
- `dbc/Model3CAN.dbc` — Tesla Model 3 DBC file with 2,897 signal definitions
- Root-level experimental scripts (`boost_now.py`, `capture334.py`, `chill_to_standard.py`, `colin_fast.py`, `hammer.py`, `hammer5.py`, `inject_and_read.py`, `torque_test.py`, `tryall.py`, etc.) — early development/testing utilities retained for reference
- `START_GHOST.bat` — Windows batch launcher for ghost mode

## CAN Bus Integration

Extensive CAN integration via OBD2 adapter using AT commands (`ATSH` to set header, direct frame send):

- **Read signals**: Accelerator pedal (0x118), steering angle (0x129), vehicle speed (0x318), battery SOC (0x132), pack voltage (0x252), pack current (0x292), wheel speeds (0x388–0x38B), drive mode (0x334), door states (0x2E1/0x2E3), ambient temp (0x3F5), motor temps (0x376), battery temps (0x201), rear motor power draw (0x266 `RearPower266`, 11-bit signed, scale 0.5, range −30 to +156 kW; note: 0x261 is 12V battery, not traction power), and many more
- **Write commands**: Drive mode change via CAN ID 0x334 (byte 0 bits 5-6: 00=Chill, 01=Standard, 10=Performance), horn via 0x273
- **Ghost mode**: Continuously sends modified 0x334 frames overriding UI drive mode setting; motor controller accepts most recent CAN value regardless of touchscreen

## Relevance to Our Project

Highly relevant — provides a complete Python-based CAN read/write toolkit for Tesla Model 3/Y with detailed signal definitions and working CAN write examples.

- **Reusability**: High
- **Key Takeaways**:
  - Detailed CAN ID mapping for 300+ Tesla Model 3 signals with bit positions, scales, and offsets; DBC file contains 2,897 signal definitions
  - Working CAN write methodology via ELM327 AT commands
  - Ghost mode frame format for drive mode override (0x334, byte 0 bits 5-6)
  - DBC file with 2,897 signal definitions
  - Dashboard server architecture (WebSocket + serial CAN reader)
  - Counter/checksum byte rotation strategy for CAN write frames
  - Rear motor power: use `0x266` (RearPower266, 11-bit signed, scale 0.5); `0x261` is 12V battery voltage, not traction power
