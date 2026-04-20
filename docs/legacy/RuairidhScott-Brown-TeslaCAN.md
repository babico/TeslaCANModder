---
title: RuairidhScott-Brown-TeslaCAN
description: A Python-based CAN bus communication tool for Tesla vehicles that uses the `python-can` and `cantools` libraries. It imp
category: legacy
folder: legacy
tags: [legacy, community, external]
author: RuairidhScott
repo: Brown-TeslaCAN
---

# RuairidhScott-Brown-TeslaCAN

## Overview

A Python-based CAN bus communication tool for Tesla vehicles that uses the `python-can` and `cantools` libraries. It implements a multiprocessing architecture to read/write CAN messages across two PCAN USB interfaces, with DBC file-based message decoding and configurable message filtering.

## Technical Details

- **Platform**: PC (Python)
- **Language**: Python 3.10+
- **CAN Interface**: PCAN USB (dual-bus: PCAN_USBBUS1 and PCAN_USBBUS2)
- **License**: None

## Architecture

- `src/tesla_can/tesla_can.py` — Core module containing:
  - `TeslaCANProcess` — A `multiprocessing.Process` subclass that creates a CAN interface, reads/writes messages, and filters by ID
  - `load_dbc_file()` — Loads Tesla DBC files via `cantools`
  - `read_in_csv()` / `read_in_ids_to_filter()` — Reads filter lists from CSV
- `data/tesla.dbc` — Tesla CAN database file with signal definitions
- `data/messages.csv` — CSV list of CAN message IDs to filter
- `pyproject.toml` — Project config with dependencies: `python-can`, `cantools`, `pandas`, `uptime`, `msgpack`
- `tests/` — Test directory
- `setup.sh` / `setup.ps1` — Environment setup scripts

The dual-process architecture bridges two PCAN buses: messages received on bus 2 are put into a shared queue, and bus 1 reads from it (and vice versa), effectively acting as a CAN bridge/proxy.

## CAN Bus Integration

Direct CAN bus integration via `python-can` library with PCAN hardware interface. Uses a Tesla DBC file for message decoding with `cantools`. The filtering system allows selective forwarding of specific CAN IDs between two bus interfaces. Message IDs to filter are loaded from `data/messages.csv`. The system supports both receiving and sending CAN messages.

## Relevance to Our Project

Useful as a reference for PC-based CAN bus analysis and bridging. The DBC file and filtering approach could inform our tooling, though our firmware runs on embedded hardware.

- **Reusability**: Medium
- **Key Takeaways**:
  - Dual-bus CAN bridge architecture with multiprocessing
  - Tesla DBC file (`data/tesla.dbc`) is a valuable reference for signal definitions
  - CSV-based message filtering approach
  - `python-can` + `cantools` stack for PC-based Tesla CAN analysis
