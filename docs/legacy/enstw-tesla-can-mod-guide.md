---
title: enstw-tesla-can-mod-guide
description: A detailed hardware wiring and installation guide for setting up an Adafruit Feather RP2040 CAN (MCP2515) board on a 202
category: legacy
folder: legacy
tags: [legacy, community, external]
author: enstw
repo: tesla-can-mod-guide
---

# enstw-tesla-can-mod-guide

## Overview

A detailed hardware wiring and installation guide for setting up an Adafruit Feather RP2040 CAN (MCP2515) board on a 2024 Tesla Model 3 Highland (HW4) for CAN bus nag suppression. Covers the complete physical installation process including cable preparation, X179 connector access, DC/DC conversion for board power, and firmware flashing.

## Technical Details

- **Platform**: Adafruit Feather RP2040 CAN (MCP2515)
- **Language**: N/A (documentation/guide only, no source code)
- **CAN Interface**: MCP2515 via X179 connector (Body Bus CAN-H/CAN-L)
- **License**: None

## Architecture

This repo contains only documentation:

- `README.md` — Complete step-by-step wiring and installation guide
- `images/pinout-labeled.png` — Enhance Auto Gen 2 cable pinout after cutting
- `images/x170-enhanced.png` — X179 connector pinout diagram
- `images/x179-connector.png` — Physical connector location photo
- `images/connected-setup.png` — Completed wiring photo

No source code is present. The guide references firmware from other projects (dongho74s-tesla-open-can-mod).

## CAN Bus Integration

Detailed CAN bus wiring documentation:

- **X179 connector pinout**: Pin 1 = +12V, Pin 9 = Body CAN-H, Pin 10 = Body CAN-L, Pin 13 = Chassis CAN-H, Pin 14 = Chassis CAN-L, Pin 20 = GND
- **Enhance Auto Gen 2 Cable**: Provides a plug-and-play X179 pigtail (Commander-side connector must be cut off for custom use)
- **Wire mapping**: Black with stripe = CAN-H, Black solid = CAN-L, Red = 12V+, Black = GND
- **Power**: 12V from X179 stepped down to 5V via MP1584EN buck converter to power the RP2040 board
- **Critical**: TERM jumper must be cut (120Ω termination resistor) — vehicle CAN bus already terminated

## Relevance to Our Project

Highly relevant as a hardware installation reference — provides the exact physical wiring guide needed to connect a CAN board to a Model 3 Highland.

- **Reusability**: High (as documentation reference)
- **Key Takeaways**:
  - Complete X179 connector pinout (Body Bus and Chassis Bus)
  - Enhance Auto Gen 2 cable modification procedure (cut Commander connector, use bare wires)
  - MP1584EN 5V buck converter for powering RP2040 from vehicle 12V
  - X179 location: passenger side footwell, behind right panel trim
  - Must wait 8-10 minutes after car power-off before wiring
  - USB-C breakout board approach for clean power delivery to Feather board
  - Heat shrink tubing for insulation and dust protection
  - Pre-install checklist for quality assurance
