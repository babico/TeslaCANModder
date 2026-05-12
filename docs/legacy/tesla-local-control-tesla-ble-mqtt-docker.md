---
title: tesla-local-control / tesla_ble_mqtt_docker
description: Docker container and Home Assistant add-on that bridges Tesla BLE vehicle-command protocol to MQTT. Enables full vehicle control and state polling without Fleet API, using Raspberry Pi or any Linux host near the car.
category: legacy
folder: legacy
tags: [legacy, community, external]
author: tesla-local-control
repo: tesla_ble_mqtt_docker
---

# tesla-local-control / tesla_ble_mqtt_docker

## Overview

Docker container (and Home Assistant add-on) that bridges Tesla's BLE vehicle-command protocol to MQTT. Runs on a Raspberry Pi or any Linux host within ~3 m of the car. Sends commands and reads vehicle state via MQTT without relying on the Fleet API. Current release is **v0.5.0** which adds Bluetooth stability improvements and automatic state polling.

## Technical Details

- **Platform**: Linux (Docker), Raspberry Pi 3+, any host with Bluetooth
- **Language**: Shell (bash), uses `teslamotors/vehicle-command` Go binary for BLE
- **Protocol**: Tesla BLE vehicle-command (protobuf over GATT), MQTT
- **License**: Apache-2.0

## Architecture

```
app/                  — shell scripts implementing the MQTT bridge logic
  tesla_ble_mqtt.sh   — main loop: MQTT subscribe → BLE command dispatch
  poll_state.sh       — periodic state polling (charge/climate/TPMS/closures/drive)
  ha_discovery.sh     — Home Assistant MQTT auto-discovery payload publisher
Dockerfile            — builds the container with vehicle-command binary
docker-compose.yml    — reference compose file
stack.env             — environment variable template (VIN, MQTT broker, credentials)
INSTALL.md            — setup guide
```

The `vehicle-command` binary (from `teslamotors/vehicle-command`) handles BLE key pairing, session establishment, and protobuf command encoding. The shell scripts wrap it with MQTT subscribe/publish logic.

## MQTT Topic Structure

### Commands (subscribe)

```
tesla/<VIN>/command/<action>    — e.g. lock, unlock, climate_on, charge_start
tesla/<VIN>/command/set_charge_limit/<percent>
tesla/<VIN>/command/set_temps/<driver>/<passenger>
```

### State (publish)

```
tesla/<VIN>/state/charge/...    — SoC, charge limit, charging state, power
tesla/<VIN>/state/climate/...   — inside/outside temp, HVAC state
tesla/<VIN>/state/drive/...     — speed, gear, odometer
tesla/<VIN>/state/closures/...  — doors, windows, trunk, frunk
tesla/<VIN>/state/tire_pressure/... — FL/FR/RL/RR kPa
```

### Home Assistant auto-discovery

Publishes `homeassistant/<domain>/tesla_<VIN>_<entity>/config` payloads on connect, enabling zero-config HA integration.

## Polling Behavior

- Polling is off by default; enabled via MQTT switch entity
- Minimum interval: 30 s (recommended ≥ 660 s to avoid keeping car awake)
- Poll only runs if car is already awake — does not wake the car
- Configurable per-category exclusion via `NO_POLL_SECTIONS` env var (charge, climate, tire-pressure, closures, drive)
- `IMMEDIATE_UPDATE=true` (default) updates state topics immediately after a successful command without waiting for next poll

## Relevance to Our Project

This repo is a reference for MQTT bidirectional control architecture. Our `mqtt_bridge.h` currently only publishes CAN-decoded telemetry. Key gaps vs this repo:

- **MQTT command ingestion** — we have no subscribe loop. This repo shows the pattern: subscribe to `cmd/+` topics, parse action, dispatch to vehicle interface.
- **HA auto-discovery** — we publish raw topics. This repo publishes structured discovery payloads that make entities appear automatically in HA.
- **State polling categories** — our MQTT publishes whatever CAN data is available. This repo organizes state into named categories with independent poll control.
- **BLE command dispatch** — this repo dispatches all vehicle commands over BLE. Our BLE layer has session/key but no command dispatch.

The shell-based architecture is not directly portable to our ESP32 firmware, but the MQTT topic schema and HA discovery pattern are directly applicable to our `mqtt_bridge.h`.
