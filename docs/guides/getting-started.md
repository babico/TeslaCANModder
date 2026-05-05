---
title: Getting Started
title_tr: Başlarken
description: Initial setup and overview of TeslaCANModder
category: guides
folder: guides
tags: [setup, quickstart, overview]
order: 1
icon: 🚀
---

# Getting Started

TeslaCANModder is an open-source CAN bus modification tool for Tesla vehicles. It intercepts and modifies CAN frames to enable features like FSD, nag suppression, nag killer (EPAS torque spoofing), speed profiles, summon, battery preconditioning, track mode, BMS telemetry, and vehicle control commands.

For a single end-to-end setup flow (build, flash, connect, and validation), use the [Full Setup Guide](full-setup.md).

## Supported Boards

| Board              | CAN Driver    | Buses | Connectivity            |
| ------------------ | ------------- | ----- | ----------------------- |
| **ESP32-S DevKit** | MCP2515 (SPI) | 1–4   | USB, WiFi (AP/STA), BLE |

## Quick Start — ESP32-S DevKit

1. Wire 1–3× MCP2515 modules to ESP32 via SPI (see [Hardware Setup](hardware-setup)) — chassis bus is required for DAS injection; vehicle / body are optional
2. Go to **Flasher** tab → select ESP32 firmware variant → flash
3. Go to **Dashboard** → Connect USB → verify boot message
4. Connect to WiFi AP `TeslaCANModder` (password: `T3SL@c@n123.`) for wireless control
5. Or pair via BLE using any Nordic UART compatible app
6. Select your vehicle variant (HW4 / HW3 / Legacy)
7. Enable features — all OFF by default
8. Install in vehicle via X179 connector

For a step-by-step validation list while doing first setup, use the [Quickstart Checklist](quickstart-checklist.md).

## Vehicle Variants

| Variant    | Vehicles                  | Key Features                                                                          |
| ---------- | ------------------------- | ------------------------------------------------------------------------------------- |
| **HW4**    | 2023+ with HW4 (FSD v14+) | FSD, Nag, Nag Killer, Profile, ISA Chime, Summon, BMS, Preconditioning, Track Mode    |
| **HW3**    | 2019–2023 with HW3        | FSD, Nag, Nag Killer, Profile, Speed Offset, Summon, BMS, Preconditioning, Track Mode |
| **Legacy** | Pre-HW3 vehicles          | FSD, Nag, Profile (limited)                                                           |

Select the variant in the Dashboard connection bar or via `variant:hw4` / `variant:hw3` / `variant:legacy` command. Setting is saved to NVS on the ESP32.

## Safety Warning

> **WARNING:** This device modifies vehicle CAN bus messages. Use at your own risk.
> Improper use may affect vehicle safety systems. Always test in safe environments.
> Educational and research purposes only.
