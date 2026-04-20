---
title: SergeyStaroletov-Tesla-CAN-packets-generator
description: A simple Arduino sketch that generates and sends pre-defined Tesla CAN packets over an MCP2515 CAN bus shield. It transm
category: legacy
folder: legacy
tags: [legacy, community, external]
author: SergeyStaroletov
repo: Tesla-CAN-packets-generator
---

# SergeyStaroletov-Tesla-CAN-packets-generator

## Overview

A simple Arduino sketch that generates and sends pre-defined Tesla CAN packets over an MCP2515 CAN bus shield. It transmits hardcoded frames for RPM, speed, inverter temperature, BMS state, battery state of charge, estimated energy, battery lifetime stats, gear, and odometer, along with random noise messages.

## Technical Details

- **Platform**: Arduino (with CAN-BUS Shield)
- **Language**: C++ (Arduino)
- **CAN Interface**: MCP2515 via SPI (CAN-BUS Shield v1.1, 125 kbps configured but commented code shows 500 kbps)
- **License**: None

## Architecture

- `sketch_tesla_can_gen.ino` — Single sketch file containing all logic
- `mcp_can.cpp` / `mcp_can.h` / `mcp_can_dfs.h` — Local copy of the MCP_CAN library

The sketch is straightforward — `setup()` initializes the CAN bus, and `loop()` sends a series of hardcoded CAN frames with 50ms delays between each, plus a random noise message at the end.

## CAN Bus Integration

Directly sends CAN messages with Tesla-specific IDs:

| CAN ID (hex) | CAN ID (dec) | Purpose | Data Length |
| --- | --- | --- | --- |
| 0x106 | 262 | RPM | 8 bytes |
| 0x256 | 598 | Speed (two variants, random selection) | 8 bytes |
| 0x306 | 774 | Inverter temperature | 8 bytes |
| 0x102 | 258 | BMS / Battery temperature (two variants) | 8 or 6 bytes |
| 0x302 | 770 | Battery state of charge | 3 bytes |
| 0x382 | 898 | Estimated energy | 8 bytes |
| 0x3D2 | 978 | Battery lifetime energy stats | 8 bytes |
| 0x116 | 278 | Gear | 6 bytes |
| 0x562 | 1378 | Odometer | 4 bytes |

All data payloads are hardcoded hex values, not dynamically generated. The sketch also sends random CAN messages with random IDs (0–9999) as bus noise.

## Relevance to Our Project

Useful as a reference for Tesla CAN message IDs and basic packet structure. Could be adapted for bench testing our firmware by providing simulated CAN traffic.

- **Reusability**: Low
- **Key Takeaways**:
  - Provides a quick list of Tesla CAN IDs for RPM, speed, BMS, gear, odometer
  - Hardcoded packet data can serve as bench-test stimulus
  - Shows MCP2515 CAN shield initialization pattern
  - Note: configured at 125 kbps, but Tesla uses 500 kbps
