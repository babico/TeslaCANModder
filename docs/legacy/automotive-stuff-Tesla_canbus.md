---
title: automotive-stuff-Tesla_canbus
description: A Raspberry Pi-based Tesla CAN bus logger and dashboard that reads CAN data from a Model S/X via SocketCAN, decodes doze
category: legacy
folder: legacy
tags: [legacy, community, external]
author: automotive
repo: stuff-Tesla_canbus
---

# automotive-stuff-Tesla_canbus

## Overview

A Raspberry Pi-based Tesla CAN bus logger and dashboard that reads CAN data from a Model S/X via SocketCAN, decodes dozens of vehicle parameters (battery, motors, steering, HVAC, DC-DC converter), publishes them over MQTT, and displays them on a web dashboard accessible from the vehicle's center console browser.

## Technical Details

- **Platform**: Raspberry Pi 4 with PICAN2 HAT
- **Language**: Python 3
- **CAN Interface**: SocketCAN (`can0`) via PICAN2 at 500 kbit/s (listen-only) using `python-can`
- **License**: None

## Architecture

```
canlogger2.1.py          — Main CAN reader + MQTT publisher
mosquitto/
  mosquitto.conf         — Local MQTT broker configuration
www/
  index.html             — Web dashboard UI
  js/                    — Dashboard JavaScript
utils/
  logg_to_csv.py         — Log to CSV converter
  slurper.py             — Data ingestion utility
  tester.py              — Test utility
old/                     — Older versions
images/                  — Setup photos and diagrams
```

The data flow is:

1. `canlogger2.1.py` reads CAN frames from `can0` via SocketCAN
2. Decodes frames into vehicle parameters using inline decode functions
3. Publishes decoded values to local Mosquitto MQTT broker (127.0.0.1:1883)
4. Web dashboard (`www/index.html`) subscribes to MQTT topics and renders live data

## CAN Bus Integration

Connects to CAN3 (Powertrain) on Tesla Model S/X. Extensively decodes the following CAN IDs:

| CAN ID (dec) | Hex | Decoded Parameters |
| ----------- | ---- | ------------------- |
| 258 (0x102) | 102 | Battery voltage, current, power |
| 262 (0x106) | 106 | Rear motor RPM |
| 277 (0x115) | 115 | Front motor RPM |
| 278 (0x116) | 116 | Rear torque estimate, vehicle speed |
| 325 (0x145) | 145 | Front torque estimate |
| 340 (0x154) | 154 | Rear torque measured, accelerator pedal position |
| 468 (0x1D4) | 1D4 | Front torque measured, torque bias |
| 528 (0x210) | 210 | DC-DC converter: current, voltage, coolant inlet temp, input/output power, efficiency |
| 562 (0x232) | 232 | BMS max discharge/charge limits |
| 614 (0x266) | 266 | Rear inverter: 12V rail, mechanical power, dissipation, stator current, regen/drive power max |
| 648 (0x288) | 288 | Rear drive ratio, wheel speeds (left/right) |
| 682 (0x2AA) | 2AA | HVAC recycle status |
| — | — | Steering angle |

- Passive read-only (listen-only mode) — does not transmit
- Uses `sudo ip link set can0 up type can bitrate 500000 listen-only on`
- Conversion constants: miles→km (1.609344), kW→HP (1.34102209)

## Relevance to Our Project

Excellent reference for Tesla Model S/X CAN message decoding with extensive parameter coverage.

- **Reusability**: Medium
- **Key Takeaways**:
  - Most comprehensive CAN ID decode table in the legacy collection (12+ IDs with bit-level extraction)
  - MQTT-based architecture for decoupled data flow is a proven pattern
  - Web dashboard on vehicle center console is a creative deployment approach
  - Bit-level decode formulas for battery, motor, DC-DC, HVAC are well-documented in code
  - Model S/X CAN3 (Powertrain) focused — some IDs may differ on Model 3/Y
  - PICAN2 + Raspberry Pi hardware platform is different from our ESP32 approach
  - No license specified — reuse requires caution
