---
title: JonnoFTW-rpi-can-logger
description: A comprehensive Raspberry Pi-based CAN bus data logger that supports multiple vehicle types including Tesla, OBD2, FMS (
category: legacy
folder: legacy
tags: [legacy, community, external]
author: JonnoFTW
repo: rpi-can-logger
---

# JonnoFTW-rpi-can-logger

## Overview

A comprehensive Raspberry Pi-based CAN bus data logger that supports multiple vehicle types including Tesla, OBD2, FMS (trucks/buses), and Outlander PHEV. It logs CAN data plus GPS position to SD card and can upload to a remote server via WiFi/4G. Includes a Bluetooth companion app and web-based data visualiser.

## Technical Details

- **Platform**: Raspberry Pi 3 / Raspberry Pi Zero W + PiCAN shield
- **Language**: Python 3
- **CAN Interface**: SocketCAN (PiCAN2 with MCP2515 via SPI overlay), compatible with python-can library
- **License**: None (no LICENSE file present)

## Architecture

- `rpi_can_logger/main.py` — Entry point; parses CLI args or YAML config, initialises CAN bus, GPS, and selected logger
- `rpi_can_logger/logger/tesla_pids.py` — Tesla-specific CAN PID decoders (BMS voltage, drive unit power/torque, battery SOC, DC-DC converter)
- `rpi_can_logger/logger/loggers.py` — Logger implementations: `TeslaSniffingLogger`, `SniffingOBDLogger`, `QueryingOBDLogger`, `FMSLogger`, `BustechLogger`
- `rpi_can_logger/gps/` — GPS serial reader (NMEA via UART)
- `rpi_can_logger/uploaders/` — Data upload handlers (web API, MongoDB)
- `systemd/` — Systemd service files for auto-start on boot
- `setup.py` — Installs systemd services and configures Bluetooth
- YAML config files for different vehicle types (e.g., `example_tesla_conf.yaml`)

## CAN Bus Integration

Directly reads Tesla CAN bus in sniffing mode via SocketCAN. Tesla config logs specific PIDs:

- `PID_TESLA_BMS_CUR_VOLTAGE` — Battery voltage/current
- `PID_TESLA_REAR_DRIVE_UNIT_INFO/POWER/TORQUE` — Rear motor data
- `PID_TESLA_FRONT_DRIVE_UNIT/POWER/TORQUE` — Front motor data
- `PID_TESLA_BATTERY_POWER_LIMITS`, `PID_TESLA_BATTERY_ODOMETER`, `PID_TESLA_BATTERY_STATE_OF_CHARGE`
- `PID_TESLA_DC_DC_CONVERTER_STATUS`

Decoders are adapted from Jason Hughes' Tesla Model S CAN deciphering document. Uses MCP2515 overlay in `/boot/config.txt` with 16 MHz oscillator at 500 kbps.

## Relevance to Our Project

Provides a mature, well-structured Python reference for Tesla CAN sniffing with specific PID decoders. The Tesla PID decoding functions are directly useful as a reference for our signal definitions.

- **Reusability**: Medium
- **Key Takeaways**:
  - Tesla PID decoder functions with bit-level extraction patterns
  - SocketCAN + PiCAN2 MCP2515 overlay configuration
  - YAML-based configurable logging architecture
  - Systemd service integration for headless vehicle deployment
