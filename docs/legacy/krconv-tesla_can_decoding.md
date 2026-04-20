---
title: krconv-tesla_can_decoding
description: A collection of Tesla CAN decoding tools focused on an ESPHome component that implements a GVRET (Generalized Vehicle Re
category: legacy
folder: legacy
tags: [legacy, community, external]
author: krconv
repo: tesla_can_decoding
---

# krconv-tesla_can_decoding

## Overview

A collection of Tesla CAN decoding tools focused on an ESPHome component that implements a GVRET (Generalized Vehicle Reverse Engineering Tool) TCP server. This allows an ESP32 running ESPHome with CAN bus to stream CAN frames to tools like SavvyCAN over a TCP connection for real-time decoding and analysis.

## Technical Details

- **Platform**: ESP32 (via ESPHome framework)
- **Language**: C++ (ESPHome component) + Python (ESPHome config)
- **CAN Interface**: ESPHome's built-in `canbus` component (ESP32 TWAI or MCP2515)
- **License**: MIT License (Copyright 2025 Kodey Converse)

## Architecture

- `esphome/components/gvret_tcp/gvret_tcp.h` — Header defining `GvretTcpServer` component class with TCP server, frame queue, binary/CSV encoding modes
- `esphome/components/gvret_tcp/gvret_tcp.cpp` — Implementation of GVRET binary protocol over TCP: server socket management, CAN frame encoding, protocol command handling (device info, bus params, keep-alive, binary output toggle, frame transmission)
- `esphome/components/gvret_tcp/__init__.py` — ESPHome component registration with YAML config schema (port, bus_index, on_transmit automation trigger)

The component integrates with ESPHome's `canbus` component — frames received via `canbus.on_frame` are forwarded to `GvretTcpServer::forward_frame()`, queued with mutex protection, and sent to connected TCP clients in either binary GVRET format or CSV format.

## CAN Bus Integration

Acts as a TCP bridge for CAN frames. The GVRET protocol commands supported include:

- `COMMAND_BUILD_CAN_FRAME` (0x00) — Transmit a CAN frame from client to bus
- `COMMAND_GET_DEVICE_INFO` (0x07) — Device identification
- `COMMAND_GET_CAN_BUS_PARAMETERS` (0x06) — Bus configuration
- `COMMAND_ENABLE_BINARY_OUTPUT` (0xE7) — Switch between binary and CSV frame encoding

Frames are encoded with microsecond timestamps, 4-byte CAN ID, DLC, and up to 8 data bytes. Compatible with SavvyCAN's GVRET network mode for remote CAN analysis.

## Relevance to Our Project

The GVRET TCP server component is a novel approach to making ESP32 CAN data available to desktop analysis tools wirelessly. Could be integrated into our firmware for diagnostic/development purposes.

- **Reusability**: Medium
- **Key Takeaways**:
  - ESPHome custom component pattern for CAN bus integration
  - GVRET binary protocol implementation for SavvyCAN compatibility
  - TCP-based CAN frame streaming with queue and mutex for thread safety
  - Bidirectional: supports both receiving and transmitting CAN frames via TCP client
  - Clean separation of protocol handling from CAN bus hardware
