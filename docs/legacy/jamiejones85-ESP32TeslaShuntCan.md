---
title: jamiejones85-ESP32TeslaShuntCan
description: An ESP32 project that reads current measurements from a Tesla P100D shunt sensor via SPI and broadcasts the readings ove
category: legacy
folder: legacy
tags: [legacy, community, external]
author: jamiejones85
repo: ESP32TeslaShuntCan
---

# jamiejones85-ESP32TeslaShuntCan

## Overview

An ESP32 project that reads current measurements from a Tesla P100D shunt sensor via SPI and broadcasts the readings over CAN bus. Designed for integrating a Tesla battery shunt into custom EV or energy storage builds.

## Technical Details

- **Platform**: ESP32
- **Language**: C++ (Arduino)
- **CAN Interface**: ESP32 built-in CAN (using `CAN.h` library)
- **License**: MIT

## Architecture

- `ESP32TeslaShuntCan.ino` — Main sketch. Initializes the shunt via SPI, sets up CAN at 500 kbit/s, and runs a 10ms periodic task to read current and broadcast it.
- `Shunt.h` / `Shunt.cpp` — Shunt driver class. Communicates with the Tesla P100D current shunt over SPI (chipSelect = SS pin, 500kHz, MODE0). Reads raw ADC values and converts to milliamps.
- Uses `TaskScheduler` library for periodic 10ms task execution.
- Based on Tom deBree's Tesla-Current-Shunt code with CAN output added by joromy.

## CAN Bus Integration

- **CAN ID 0x521** — Transmits current reading as a little-endian 32-bit signed integer in bytes 2–5. Broadcast every 10ms at 500 kbit/s.
- This is a custom CAN message (not a standard Tesla CAN ID), designed for integration with other CAN-connected devices like BMS controllers or inverters.

## Relevance to Our Project

Marginally relevant — demonstrates ESP32 CAN bus TX and Tesla hardware component integration, but does not interact with the vehicle's autopilot or FSD CAN messages.

- **Reusability**: Low
- **Key Takeaways**:
  - Simple example of ESP32 CAN TX using the built-in CAN library
  - Tesla P100D shunt SPI communication protocol
  - TaskScheduler pattern for periodic CAN message broadcasting
