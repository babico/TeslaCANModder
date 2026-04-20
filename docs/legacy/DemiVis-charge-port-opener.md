---
title: DemiVis-charge-port-opener
description: A simple CircuitPython project that uses an Adafruit Feather M0 RFM69 radio module to transmit the Tesla charge port ope
category: legacy
folder: legacy
tags: [legacy, community, external]
author: DemiVis
repo: charge-port-opener
---

# DemiVis-charge-port-opener

## Overview

A simple CircuitPython project that uses an Adafruit Feather M0 RFM69 radio module to transmit the Tesla charge port open signal at 315 MHz. It sends a known RF signal sequence that mimics the Tesla charge port button, repeatedly broadcasting it to open the charge port wirelessly.

## Technical Details

- **Platform**: Adafruit Feather M0 RFM69 (SAMD21)
- **Language**: CircuitPython 8.2.9
- **CAN Interface**: N/A (uses 315 MHz RF radio, not CAN bus)
- **License**: None

## Architecture

- `code.py` — Main script: configures the RFM69 radio at 315 MHz with Tesla's sync word (`0x8ACB`), then repeatedly sends a hardcoded 42-byte RF signal every 5 seconds (10 bursts per cycle)
- `lib/adafruit_bus_device/` — CircuitPython I2C/SPI bus device library
- `boot_out.txt` — Board identification (Feather M0 RFM69)

The code bypasses the standard adafruit_rfm69 `send()` method (which adds headers) and writes the raw Tesla signal directly to the FIFO register for exact signal reproduction.

## CAN Bus Integration

No direct CAN integration. This project uses 315 MHz RF to emulate the Tesla wall charger button signal. The charge port open signal is a specific bit pattern transmitted over radio, not CAN bus.

## Relevance to Our Project

Tangentially relevant — demonstrates Tesla RF protocol for charge port, but uses a completely different communication method (RF vs CAN).

- **Reusability**: Low
- **Key Takeaways**:
  - Tesla charge port opens via 315 MHz RF signal with sync word `0x8ACB`
  - The exact RF signal payload is documented (42 bytes)
  - Shows that some Tesla functions use RF rather than CAN bus
