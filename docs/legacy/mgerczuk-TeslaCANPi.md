---
title: mgerczuk-TeslaCANPi
description: A Raspberry Pi Zero-based CAN bus logger for Tesla Model 3 that stores data offline on the Pi's SD card. The system incl
category: legacy
folder: legacy
tags: [legacy, community, external]
author: mgerczuk
repo: TeslaCANPi
---

# mgerczuk-TeslaCANPi

## Overview

A Raspberry Pi Zero-based CAN bus logger for Tesla Model 3 that stores data offline on the Pi's SD card. The system includes power management via an Arduino Pro Mini, a UPS module for clean shutdowns, and a .NET/Mono service that collects CAN data, serves it via HTTP, and communicates over Bluetooth using the ELM327 protocol. Data is compatible with the TeslaLogger project.

## Technical Details

- **Platform**: Raspberry Pi Zero + Arduino Pro Mini ATMega328
- **Language**: C# (.NET/Mono), Arduino C++
- **CAN Interface**: Waveshare RS485 CAN HAT (MCP2515, SPI, 12MHz oscillator)
- **License**: Apache License 2.0

## Architecture

- `VisualStudio/TeslaCAN/Program.cs` — Main entry point: starts database, CAN data collector, HTTP server, and Bluetooth ELM327 interface
- `VisualStudio/TeslaCAN/SQLite.cs` — SQLite database for CAN data storage
- `VisualStudio/TeslaCAN/Syslog.cs` — Linux syslog integration
- `VisualStudio/TeslaCAN/CanDB/` — CAN database/DBC parsing
- `VisualStudio/TeslaCAN/Elm327/` — ELM327 Bluetooth protocol implementation
- `VisualStudio/TeslaCAN/SocketCAN/` — SocketCAN interface binding
- `VisualStudio/TeslaCAN/TeslaLogger/` — TeslaLogger data format compatibility
- `VisualStudio/TeslaCAN/www/w3.css` — Static CSS asset served by the built-in HTTP server
- `VisualStudio/DbcParser/` — DBC file parser library
- `VisualStudio/Mono.BlueZ/` — BlueZ Bluetooth stack bindings for Mono
- `VisualStudio/TeslaCANTests/` — Unit test project
- `VisualStudio/Linux Service/TeslaCAN.service` — systemd service unit for auto-start on boot
- `Arduino/raspi_power.ino` — Arduino power management: monitors external power, controls Pi startup/shutdown via GPIO, enters deep sleep when no power
- `Fritzing/` — Hardware wiring diagrams
- `Images/` — Photos of the assembled hardware

## CAN Bus Integration

- Uses Waveshare RS485 CAN HAT connected via SPI with MCP2515 controller
- Configured in `/boot/config.txt` with `dtoverlay=mcp2515-can0,oscillator=12000000,interrupt=25`
- Collects CAN data through SocketCAN interface
- Parses frames using DBC definitions (compatible with ScanMyTesla data format)
- Provides ELM327 Bluetooth interface for external app compatibility

## Relevance to Our Project

Demonstrates a complete CAN logging system with power management, data persistence, and multiple interfaces. The DBC parser, SocketCAN bindings, and ELM327 Bluetooth implementation are well-architected reference implementations.

- **Reusability**: Medium
- **Key Takeaways**:
  - Robust power management design (Arduino controls Pi boot/shutdown via GPIO)
  - ELM327 Bluetooth protocol implementation for CAN data access
  - DBC parser for decoding Tesla CAN frames
  - Waveshare CAN HAT configuration (MCP2515 with 12MHz oscillator at SPI 1MHz)
  - SQLite-based CAN data storage pattern
  - TeslaLogger data format compatibility
