---
title: SlipknotTN-Tesla_CanBus_Reader
description: A Java application that reads and decodes raw Tesla CAN bus log dumps. It parses hex-encoded CAN frames from text log fi
category: legacy
folder: legacy
tags: [legacy, community, external]
author: SlipknotTN
repo: Tesla_CanBus_Reader
---

# SlipknotTN-Tesla_CanBus_Reader

## Overview

A Java application that reads and decodes raw Tesla CAN bus log dumps. It parses hex-encoded CAN frames from text log files and decodes specific signals (speed, SOC, torque, gear, pedal position, energy stats, temperatures, etc.) using a manually defined signal map with bit-level extraction, scaling, and offset.

## Technical Details

- **Platform**: PC (Java)
- **Language**: Java
- **CAN Interface**: N/A (reads offline log files, not live CAN)
- **License**: MIT

## Architecture

- `src/com/slipi/Main.java` — Entry point, reads log files line by line and decodes them
- `src/com/slipi/core/CodesMap.java` — HashMap mapping CAN ID hex strings to `Code` objects with signal definitions (bit position, length, scale, offset, signed flag)
- `src/com/slipi/core/DataDecodingUtils.java` — Core decoding logic: hex-to-bytes, bit extraction via BitSet, two's complement for signed values, scale/offset application
- `src/com/slipi/core/DataField.java` — Signal definition (startBit, numBits, signed, scale, offset)
- `src/com/slipi/core/DataKeyEnumOBD.java` — Enum of all decoded signal names (65+ signals)
- `src/com/slipi/core/ValuedDataField.java` — Decoded value container
- `src/com/slipi/core/Code.java` — CAN message definition (name, DLC, list of DataFields)
- `dumps/` — Collection of raw CAN log files from a real Tesla (various scenarios: battery, charging, driving, beams)

## CAN Bus Integration

Decodes the following Tesla CAN messages (offline from log files):

| CAN ID | Name | Signals |
| --- | --- | --- |
| 0x118 | Drive state and pedals | Drive state, brake press, gear (P/R/N/D), accelerator pedal % |
| 0x1D8 | Rear Torque | Rear torque request (Nm), rear torque (Nm) |
| 0x257 | Speedometer | Signed speed (km/h), UI speed, mph/kph flag |
| 0x292 | SOC state | UI SOC %, min/max/avg SOC %, full pack energy |
| 0x293 | Steering and traction | Steering mode (comfort/standard/sport), traction mode |
| 0x352 | BMS energy status | Energy status index, nominal full pack energy, remaining energy, ideal energy, UI SOC (post-2024.20), fully charged flag |
| 0x3B6 | Odometer | Odometer (km) |

Also contains commented DBC-style signal definitions for CAN ID 0x273 (627) `UI_vehicleControl` with 40+ signals for lights, locks, mirrors, wipers, seat heating, etc.

The `DataKeyEnumOBD` enum lists 65+ signal names covering speed, torque, power, SOC, temperature, gear, lighting, and more.

## Relevance to Our Project

Highly valuable for signal decoding reference. The `CodesMap.java` provides detailed bit-level definitions (start bit, length, scale, offset) for key Tesla CAN signals that can inform our DBC file and decoding logic.

- **Reusability**: Medium
- **Key Takeaways**:
  - Detailed bit-level signal definitions for Tesla CAN IDs (0x118, 0x1D8, 0x257, 0x292, 0x293, 0x352, 0x3B6)
  - Two's complement decoding for signed CAN signals
  - Real CAN dump files from a Tesla vehicle (battery, charging, driving scenarios)
  - Commented DBC definitions for UI_vehicleControl (0x273) with 40+ signals
  - SOC calculation changed after firmware 2024.20 (from 0x292 to 0x352 index 2)
