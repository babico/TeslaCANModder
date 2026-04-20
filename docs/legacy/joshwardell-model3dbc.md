---
title: joshwardell-model3dbc
description: A community-maintained DBC (Database CAN) file for Tesla Model 3 and Model Y CAN messages. This is one of the most widel
category: legacy
folder: legacy
tags: [legacy, community, external]
author: joshwardell
repo: model3dbc
---

# joshwardell-model3dbc

## Overview

A community-maintained DBC (Database CAN) file for Tesla Model 3 and Model Y CAN messages. This is one of the most widely referenced Tesla CAN signal databases in the community, used with tools like SavvyCAN, CANBUS-Analyzer, and TeslaX app.

## Technical Details

- **Platform**: N/A (data file)
- **Language**: DBC format (Vector CANdb standard)
- **CAN Interface**: N/A — designed for use with any DBC-compatible CAN tool (Vector, Kvaser, Peak, etc.)
- **License**: MIT License (Copyright 2019 Josh Wardell)

## Architecture

Single file repository:

- `Model3CAN.dbc` — Complete DBC file defining Tesla Model 3/Y CAN message IDs, signal names, bit positions, scaling factors, units, and value descriptions

## CAN Bus Integration

This is the canonical Tesla Model 3/Y CAN signal database. It defines message structures, signal layouts (bit start, length, endianness), scaling/offset values, and human-readable signal names for hundreds of CAN IDs. Referenced by multiple other projects in this legacy collection (e.g., `jsamuel1-tesla_canlogjs`, `LeeGaHyeon-tesla_CAN_traffic_decode`, `krconv-tesla_can_decoding`).

## Relevance to Our Project

Extremely important reference — this is the de facto standard Tesla Model 3/Y DBC file. Our project's CAN message definitions should be validated against this.

- **Reusability**: High
- **Key Takeaways**:
  - Authoritative Tesla Model 3/Y CAN signal definitions
  - Widely adopted by the community (SavvyCAN, comma.ai opendbc, etc.)
  - Can be directly used by `cantools` Python library or C++ DBC parsers
  - Josh Wardell also created the CANserver hardware project (jwardell.com/canserver)
