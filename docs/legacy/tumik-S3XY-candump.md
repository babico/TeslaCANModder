---
title: tumik-S3XY-candump
description: A Python tool for dumping CAN bus data from Tesla vehicles using the Enhauto S3XY Commander (formerly S3XY buttons) hard
category: legacy
folder: legacy
tags: [legacy, community, external]
author: tumik
repo: S3XY-candump
---

# tumik-S3XY-candump

## Overview

A Python tool for dumping CAN bus data from Tesla vehicles using the Enhauto S3XY Commander (formerly S3XY buttons) hardware. Connects to the Commander's WiFi interface, receives raw CAN data via the Panda protocol, and writes it to log files in candump and SavvyCAN formats. Tested on Model 3 2022.

## Technical Details

- **Platform**: Python (any OS — Raspberry Pi, laptop, etc.)
- **Language**: Python
- **CAN Interface**: Enhauto S3XY Commander via WiFi (Panda protocol over TCP)
- **License**: GPL-3.0

## Architecture

- `s3xy-candump.py` — Main script. Connects to Panda IP (192.168.4.1:1338), loads DBC file to get CAN message IDs for subscription, then enters a loop reading and logging frames
- `s3xycandump/` — Package with modular components:
  - `panda.py` — Panda protocol TCP client implementation
  - `canmsg.py` — CAN message data structure
  - `candump.py` — candump format writer
  - `savvycan.py` — SavvyCAN/GVRET format writer
  - `dumpfile.py` — Generic dump file manager
- `model3dbc/` — Git submodule containing Model3CAN.dbc (DBC file for message ID list)
- Dual bus support via the S3XY Commander (connects to two CAN buses on the vehicle)

## CAN Bus Integration

Reads raw CAN frames from the Tesla vehicle via the S3XY Commander's WiFi Panda interface. Uses the DBC file as a filter list for which CAN IDs to subscribe to. Does not decode the messages — just captures raw frames. Output formats:

- **candump**: Compatible with Linux can-utils candump log format
- **SavvyCAN**: Compatible with SavvyCAN/GVRET format, which can also be imported into Openpilot Cabana via `make_cabana_route`

## Relevance to Our Project

Demonstrates a wireless CAN capture approach using consumer hardware (S3XY Commander) — useful for data collection and analysis without physical OBD access.

- **Reusability**: Medium
- **Key Takeaways**:
  - Panda protocol implementation in Python for S3XY Commander WiFi interface
  - Dual-bus CAN capture capability
  - Multiple output format support (candump, SavvyCAN)
  - DBC file used as message subscription filter
  - SavvyCAN output can be imported to Openpilot Cabana for advanced analysis
  - Good reference for building wireless CAN capture tools
