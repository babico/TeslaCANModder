---
title: gregjhogan / tesla-pre-ap-epas-patch
description: A Python tool that patches the EPAS (Electronic Power Assisted Steering) firmware on pre-Autopilot Tesla vehicles to ena
category: legacy
folder: legacy
tags: [legacy, community, external]
author: gregjhogan
repo: tesla-pre-ap-epas-patch
---

# gregjhogan / tesla-pre-ap-epas-patch

## Overview

A Python tool that patches the EPAS (Electronic Power Assisted Steering) firmware on pre-Autopilot Tesla vehicles to enable steering over CAN bus. It uses UDS (Unified Diagnostic Services) via a comma.ai Panda dongle to extract, patch, and flash modified firmware that allows CAN-based steering control on vehicles where the gateway normally disables it.

## Technical Details

- **Platform**: PC (Python script), requires comma.ai Panda OBD-II dongle
- **Language**: Python 3
- **CAN Interface**: comma.ai Panda (USB-to-CAN dongle)
- **License**: GPL-3.0

## Architecture

- `patch.py` — Main script: extracts firmware from EPAS via UDS, validates MD5 against known good firmware, applies binary patches, recalculates CRC32 checksums, and flashes modified firmware back
- `requirements.txt` — Python dependencies (tqdm for progress bars)
- `epas-bootloader-0x3ff7000-0x3ffacbd.bin` — EPAS bootloader binary needed for the flash process
- Depends on openpilot's Panda library for UDS communication

Key functions:

- `get_security_access_key()` — Implements the EPAS security access algorithm (seed→key)
- `extract_firmware()` — Reads firmware from EPAS via UDS `ReadMemoryByAddress`
- `patch_firmware()` — Replaces specific ARM instructions to load constant `1` instead of extracting CAN signal values
- `flash_firmware()` — Erases and writes patched firmware back to EPAS via UDS

## CAN Bus Integration

Deeply integrated with Tesla CAN bus at the firmware level:

- **CAN ID 0x101** — `GTW_epasControl`: Contains `GTW_epasControlType` (3 bits: INHIBIT/ANGLE/TORQUE/BOTH) and `GTW_epasLDWEnable` (1 bit). On non-AP vehicles, both are set to 0 (disabled).
- **CAN ID 0x214** — `EPB_epasControl`: Contains `EPB_epasEACAllow` (3 bits: DISABLE/ENABLE). Electronic parking brake sets this to 0 on non-AP vehicles.
- The firmware patch replaces the signal extraction code with constant `1` values, effectively enabling CAN steering control regardless of what the gateway sends.
- Operates on the chassis CAN bus (connected in parallel with EPAS, bypassing the gateway).

## Relevance to Our Project

Moderate relevance. While our project focuses on autopilot/FSD features rather than steering firmware patching, this repo provides valuable insight into Tesla's UDS protocol, EPAS security mechanisms, and CAN bus signal structure.

- **Reusability**: Low — different approach (firmware patching vs. CAN frame injection)
- **Key Takeaways**:
  - Tesla EPAS security access algorithm (seed-key challenge) is documented and implemented
  - UDS protocol usage for Tesla ECU communication (diagnostic session, firmware read/write)
  - CAN signal definitions for `GTW_epasControl` (0x101) and `EPB_epasControl` (0x214)
  - CRC32 checksum recalculation approach for patched firmware regions
  - Demonstrates that Tesla CAN bus steering control requires both gateway and EPB signals
