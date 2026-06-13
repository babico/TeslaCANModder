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

The repo has evolved into a goal-oriented install guide workspace with a canonical target configuration (`install-target.yaml`), progress tracking (`PROGRESS.md`), and a Feather Hypery11 port plan with a captured patch artifact for porting hypery11 CAN logic to the RP2040 platform.

## Technical Details

- **Platform**: Adafruit Feather RP2040 CAN (MCP2515) — target hardware
- **Language**: N/A (documentation/guide), C++ patch artifact for ev-open-can-tools
- **CAN Interface**: MCP2515 via X179 connector (Chassis Bus CAN-H/CAN-L, pin 13/14)
- **License**: None

## Architecture

Documentation and planning artifacts:

- `README.md` — Complete step-by-step wiring and installation guide
- `install-target.yaml` — Canonical vehicle, hardware, bus, and rollout configuration
- `PROGRESS.md` — Physical install state, firmware target, staged rollout plan, risk decisions
- `FEATHER_HYPERY11_PORT_PLAN.md` — Resumable implementation plan for porting hypery11 CAN logic to Feather RP2040
- `patches/2026-05-11-feather-hypery11-port.patch` — Captured patch (1315 lines) with Section 1-3 changes: `CanMode` enum, `setMode()`/`mode()` API, `FeatherHypery11Handler`, native test suites (205 tests)
- `patches/README.md` — Patch application instructions
- `AGENTS.md` — Goal-oriented workspace instructions for AI agents
- `images/` — Wiring diagrams, connector photos, setup photos

No standalone source code. The guide references firmware from other projects and captures hypery11-derived logic as a patch against `ev-open-can-tools`.

## CAN Bus Integration

Detailed CAN bus wiring documentation:

- **X179 connector pinout**: Pin 1 = +12V, Pin 9 = Body CAN-H, Pin 10 = Body CAN-L, Pin 13 = Chassis CAN-H, Pin 14 = Chassis CAN-L, Pin 20 = GND
- **Target bus**: X179 pin 13/14 (Chassis CAN / Bus 6 mixed forwarding) — verified by frame visibility
- **Enhance Auto Gen 2 Cable**: Provides a plug-and-play X179 pigtail (Commander-side connector must be cut off for custom use)
- **Wire mapping**: Black with stripe = CAN-H, Black solid = CAN-L, Red = 12V+, Black = GND
- **Power**: 12V from X179 stepped down to 5V via MP1584EN buck converter to power the RP2040 board
- **Critical**: TERM jumper must be cut (120Ω termination resistor) — vehicle CAN bus already terminated

Ported CAN logic (via patch artifact):
- **Section 1**: `CanMode` enum (`Normal`, `ListenOnly`), driver-level listen-only gate, MCP2515 mode persistence across `init()`/`setFilters()`, TX blocking in listen-only
- **Section 2-3**: `FeatherHypery11Handler` with DAS-gated nag suppression, EPAS counter+1 echo, GTW car state parsing, follow-distance stalk mapping
- **Test matrix**: 205 native tests across 4 suites (`native`, `native_nag`, `native_can_mode`, `native_hypery11`)

## Relevance to Our Project

Highly relevant as a hardware installation reference and Feather RP2040 port strategy.

- **Reusability**: High (as documentation reference and patch artifact)
- **Key Takeaways**:
  - Complete X179 connector pinout (Body Bus and Chassis Bus)
  - Enhance Auto Gen 2 cable modification procedure (cut Commander connector, use bare wires)
  - MP1584EN 5V buck converter for powering RP2040 from vehicle 12V
  - X179 location: passenger side footwell, behind right panel trim
  - Must wait 8-10 minutes after car power-off before wiring
  - USB-C breakout board approach for clean power delivery to Feather board
  - Heat shrink tubing for insulation and dust protection
  - Pre-install checklist for quality assurance
  - Feather Hypery11 port plan with resumable patch artifact (205 passing native tests)
  - Listen-only mode as safe default before active TX — driver-level enforcement
  - DAS-gated nag suppression logic ported from hypery11 to RP2040 platform
  - Goal-oriented agent instructions (`AGENTS.md`) for future workspace sessions
