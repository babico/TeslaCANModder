---
title: cbusillo-TeslaPiCAN
description: A Python-based Tesla Model 3 CAN bus reader using python-can and cantools on a Raspberry Pi with SocketCAN. It implement
category: legacy
folder: legacy
tags: [legacy, community, external]
author: cbusillo
repo: TeslaPiCAN
---

# cbusillo-TeslaPiCAN

## Overview

A Python-based Tesla Model 3 CAN bus reader using python-can and cantools on a Raspberry Pi with SocketCAN. It implements an async CAN message subscriber system, decodes signals using DBC files, and includes a volume/scroll wheel flick feature that periodically sends CAN commands. The DBC files directory is currently empty (likely a submodule that was not initialized).

## Technical Details

- **Platform**: Raspberry Pi (SocketCAN)
- **Language**: Python (asyncio)
- **CAN Interface**: SocketCAN (`can0`) at 500 kbit/s via python-can + cantools
- **License**: None

## Architecture

- `main.py` — Core application:
  - `CANBusSubscriber` — Pub/sub pattern for CAN message callbacks, supports subscribing to specific CAN IDs or all messages
  - `read_can_messages()` — Async loop reading from SocketCAN bus, dispatching to subscribers
  - `create_signal_dict()` — Builds signal dictionaries from DBC message definitions with multiplexer support
  - Signal logging for specific CAN IDs (0x321: brake fluid/coolant level, 0x3d8: elevation)
  - Volume flick feature with configurable interval and jitter
- `dbc/model3/` — DBC file directory for Model3CAN.dbc (empty — submodule not cloned)
- `.gitmodules` — References external DBC submodule

## CAN Bus Integration

Reads Tesla Model 3 CAN bus at 500 kbit/s via SocketCAN. Uses cantools for DBC-based signal decoding. Specific signals monitored:

- **CAN ID 0x321**: `VCFRONT_brakeFluidLevel`, `VCFRONT_coolantLevel`
- **CAN ID 0x3d8**: `Elevation3D8`
- Supports multiplexed CAN frames with proper mux ID handling
- Can send CAN messages (volume flick feature implies write capability)

## Relevance to Our Project

Useful as a reference for Python async CAN bus architecture. The subscriber pattern is clean and extensible. However, the empty DBC directory and limited signal set reduce its standalone value.

- **Reusability**: Medium
- **Key Takeaways**:
  - Clean async pub/sub pattern for CAN message handling in Python
  - Multiplexer-aware signal dictionary builder from DBC definitions
  - cantools integration for DBC-based Tesla CAN decoding
  - Specific CAN ID references: 0x321 (brake fluid, coolant), 0x3d8 (elevation)
