---
title: Adminius-ESP32-ScanMyTesla
description: A CAN-to-Bluetooth adapter firmware for ESP32 that bridges the Tesla vehicle CAN bus to the "ScanMyTesla" Android app. I
category: legacy
folder: legacy
tags: [legacy, community, external]
author: Adminius
repo: ESP32-ScanMyTesla
---

# Adminius-ESP32-ScanMyTesla

## Overview

A CAN-to-Bluetooth adapter firmware for ESP32 that bridges the Tesla vehicle CAN bus to the "ScanMyTesla" Android app. It reads CAN frames via the ESP32's built-in TWAI peripheral in listen-only mode and forwards filtered data over Bluetooth Serial (SPP) to the mobile app for real-time vehicle telemetry display.

## Technical Details

- **Platform**: ESP32 (classic, not S3/C3/H2 variants)
- **Language**: C++ (Arduino framework)
- **CAN Interface**: ESP32 TWAI (built-in CAN peripheral) at 500 kbit/s, listen-only mode, with SN65HVD230 transceiver
- **License**: GPL-3.0

## Architecture

- `ESP32-ScanMyTesla.ino` — Main (v2.0.0, 2024): Initializes TWAI in listen-only mode, buffers incoming CAN frames (16-frame ring buffer), filters by known Tesla IDs, and forwards via Bluetooth Serial
- `ESP32-ScanMyTesla_v1.ino` — Older version kept for reference
- `ESP32_can_connection.png` — Hardware wiring diagram

Key design:

- CAN frames are buffered in a fixed-size array (`BUFFER_LENGTH = 16`)
- Duplicate CAN ID handling: replaces existing buffer entry if same ID + same first byte (multiplex index)
- ID filtering via a boolean array (`ids[2048]`) — only whitelisted IDs are processed
- Bluetooth SPP profile (`ESP-SMT`) for ScanMyTesla app communication

## CAN Bus Integration

- Uses ESP32 TWAI peripheral in **listen-only mode** (passive — does not transmit)
- Connects to Tesla CAN bus at 500 kbit/s via SN65HVD230 transceiver (no termination resistor)
- Filters CAN messages by ID using a whitelist populated by the ScanMyTesla app
- Handles messages 1–8 bytes, drops corrupt frames
- Does NOT modify or inject CAN frames — purely a read-only diagnostic bridge

## Relevance to Our Project

Demonstrates passive CAN reading on ESP32 TWAI and Bluetooth bridging to ScanMyTesla.

- **Reusability**: Medium
- **Key Takeaways**:
  - TWAI listen-only configuration pattern is a useful reference for diagnostic/read-only modes
  - Ring buffer design for CAN frame deduplication by ID + multiplex byte
  - SN65HVD230 transceiver wiring (no termination resistor) documented in connection diagram
  - iOS not supported — Apple blocks Bluetooth Serial
  - Only works with classic ESP32, not ESP32-S3/C3/H2 variants
