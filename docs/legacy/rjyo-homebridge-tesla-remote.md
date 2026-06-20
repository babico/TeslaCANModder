---
title: rjyo-homebridge-tesla-remote
description: A Homebridge plugin that exposes Tesla vehicle controls to Apple HomeKit and Siri. Supports climate control (thermostat)
category: legacy
folder: legacy
tags: [legacy, community, external]
author: rjyo
repo: homebridge-tesla-remote
---

# rjyo-homebridge-tesla-remote

## Overview

A Homebridge plugin that exposes Tesla vehicle controls to Apple HomeKit and Siri. Supports climate control (thermostat), door locks, battery level monitoring, and charge control via the Tesla API (using the teslams library and OAuth tokens).

## Architecture

```mermaid
flowchart LR
    HomeKit["Apple HomeKit / Siri"] --> Plugin["Homebridge plugin<br/>(Node.js)"]
    Plugin --> Teslams["teslams library"]
    Teslams --> OAuth["Tesla OAuth tokens"]
    OAuth --> API["Tesla REST API"]
    API --> Car["Tesla vehicle"]
    classDef path fill:#1e3a5f,stroke:#4a7fb5,color:#fff
    class Plugin,API path
```

## Technical Details

- **Platform**: Node.js (Homebridge plugin)
- **Language**: JavaScript (ES6, Babel transpiled)
- **CAN Interface**: N/A
- **License**: MIT

## Architecture

- `src/index.js` — Plugin registration entry point, registers the "Tesla" accessory with Homebridge.
- `src/createTesla.js` — Main Tesla accessory class implementing HomeKit services:
  - `Thermostat` — Climate control (get/set temperature, HVAC on/off)
  - `LockMechanism` — Door lock/unlock
  - `BatteryService` — Battery level and charging state
  - `Switch` — Charging on/off control
- `package.json` — Depends on `teslams` library for Tesla API communication.
- Uses token-based authentication (no username/password stored).

## CAN Bus Integration

No direct CAN integration. Communicates with Tesla vehicles exclusively through the Tesla REST API via the `teslams` Node.js library. Commands include climate control, door lock/unlock, charge start/stop, and state queries.

## Relevance to Our Project

No relevance to CAN bus or firmware work. This is a cloud API integration for home automation.

- **Reusability**: None
- **Key Takeaways**:
  - None directly applicable to CAN bus or firmware development
