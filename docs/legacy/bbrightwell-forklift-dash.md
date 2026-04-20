---
title: bbrightwell-forklift-dash
description: A full-screen CAN bus dashboard for the **Linde E20 (1252/1254 series)** electric forklift, not a Tesla project. It runs
category: legacy
folder: legacy
tags: [legacy, community, external]
author: bbrightwell
repo: forklift-dash
---

# bbrightwell-forklift-dash

## Overview

A full-screen CAN bus dashboard for the **Linde E20 (1252/1254 series)** electric forklift, not a Tesla project. It runs in Chromium on a Raspberry Pi 5, reads CAN data via SocketCAN, decodes signals using DBC files via cantools, and displays live telemetry (speed, battery SOC, hydraulics, tilt) on a Linde-branded dark UI with a Tesla-style power graph.

## Technical Details

- **Platform**: Raspberry Pi 5 + Chromium kiosk
- **Language**: Python (CAN bridge/decoder), JavaScript/HTML/CSS (dashboard UI)
- **CAN Interface**: SocketCAN (`can0`) via python-can + cantools DBC decoder
- **License**: None

## Architecture

- `can-bridge/bridge.py` — CAN-to-WebSocket bridge; reads CAN frames from SocketCAN (or replays a log), decodes via DBC, broadcasts JSON over WebSocket
- `can-bridge/decoder.py` — DBC-based frame decoder using cantools
- `can-bridge/log_replayer.py` — Replays recorded CAN trace logs in real time
- `index.html` / `style.css` / `js/` — Three-panel browser dashboard (speed/direction/throttle, vehicle graphic, battery/hydraulics/tilt)
- `start.sh` — Launcher script that starts bridge, HTTP server, and Chromium
- `canbus-definitions/` — Linde forklift DBC files (submodule)

## CAN Bus Integration

Reads CAN frames from `can0` at 500 kbit/s using python-can. Decodes signals using Linde-specific DBC files via cantools. Signals include battery voltage, SOC, temperatures, hour meter, hydraulic pressure, tilt angle, speed, and direction. CAN IDs are Linde-specific, not Tesla.

## Relevance to Our Project

The **architecture** (CAN → Python decoder → WebSocket → browser dashboard) is a solid reference pattern. The DBC-based decoding pipeline and simulated replay mode are directly applicable concepts, even though the CAN IDs are for a forklift, not a Tesla.

- **Reusability**: Medium
- **Key Takeaways**:
  - CAN-to-WebSocket bridge pattern for real-time dashboard display
  - DBC-based signal decoding with cantools is clean and reusable
  - Log replay / simulation mode is useful for development without hardware
  - Tesla-style power graph visualization could be adapted
