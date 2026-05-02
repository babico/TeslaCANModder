---
title: Connections & Integrations
description: Configuration and runtime behavior for MQTT, Tesla BLE, Home Assistant, ESP-NOW, GVRET, ScanMyTesla, and ELM327
category: reference
folder: reference
tags: [connections, mqtt, tesla-ble, home-assistant, esp-now, gvret, scanmytesla, elm327]
order: 8
icon: 🔌
---

# Connections & Integrations

This page documents all integration-oriented connection features exposed by Tesla CAN Modder.

Covered integrations:

- MQTT
- Tesla BLE
- Home Assistant
- ESP-NOW
- GVRET
- ScanMyTesla
- ELM327

## Common Validation Flow

1. Enable feature with command.
2. Wait 2 to 5 seconds.
3. Run `status`.
4. Confirm related state fields changed as expected.

If a feature does not toggle, verify:

- Board firmware variant supports it.
- Required transport is active (usually WiFi or BLE).
- You are not in OTA pause state.

## MQTT

Publishes telemetry snapshots to a broker over WiFi.

Commands:

- `mqtt:on`
- `mqtt:off`
- `mqtt:broker:<host>`
- `mqtt:port:<1-65535>`
- `mqtt:interval:<100-60000>`

Status fields:

- `mqtt`
- `mqttConnected`

Notes:

- Requires WiFi-enabled firmware/runtime.
- Broker/port/interval settings persist in NVS on ESP32 builds.

## Tesla BLE (VCSEC)

Provides Tesla BLE vehicle control path with pairing/auth workflow.

Commands:

- `teslable:on`
- `teslable:off`
- `teslable:auth`
- `teslable:forget`

Status fields:

- `teslaBle`
- `teslaBleConnected`
- `teslaBleAuth`

Notes:

- `teslable:on` enables the feature; authentication is a separate step.
- Connected without auth means BLE link exists but command-level authorization is incomplete.

## Home Assistant

Home Assistant integration (including discovery/reporting behavior).

Commands:

- `ha:on`
- `ha:off`
- `ha:discovery`
- `ha:interval:<500-60000>`

Status fields:

- `homeAssistant`
- `haConnected`
- `haEntities`

Notes:

- Typical flow: enable HA, trigger discovery, then verify entities in HA.
- Entity count helps confirm successful registration.

## ESP-NOW

ESP-NOW broadcast path for low-latency multi-device communication.

Commands:

- `espnow:on`
- `espnow:off`
- `espnow:channel:<1-13>`

Status fields:

- `espNow`
- `espNowChannel`
- `espNowPeers`

Notes:

- Channel must match peer network channel plan.
- Peer count indicates detected/registered recipients.

## GVRET

GVRET-compatible TCP gateway (SavvyCAN workflow).

Commands:

- `gvret:on`
- `gvret:off`
- `gvret:port:<1-65535>`

Status fields:

- `gvret`
- `gvretPort`
- `gvretClients`

Notes:

- When enabled, clients can connect with GVRET-compatible tools.
- `gvretClients` confirms active consumer sessions.

## ScanMyTesla Bridge

Compatibility mode for ScanMyTesla bridge workflows.

Commands:

- `smt:on`
- `smt:off`

Status fields:

- `scanMyTesla`

Notes:

- This is a dedicated compatibility path and separate from generic Tesla BLE VCSEC flow.

## ELM327 Emulation

ELM327 command emulation for OBD-style client compatibility.

Commands:

- `elm327:on`
- `elm327:off`

Status fields:

- `elm327`

Notes:

- Enable only when an ELM327-style client is expected.

## Quick Status Matrix

| Integration    | Primary Enable | Key Connected Field(s)              |
| -------------- | -------------- | ----------------------------------- |
| MQTT           | `mqtt:on`      | `mqttConnected`                     |
| Tesla BLE      | `teslable:on`  | `teslaBleConnected`, `teslaBleAuth` |
| Home Assistant | `ha:on`        | `haConnected`, `haEntities`         |
| ESP-NOW        | `espnow:on`    | `espNowPeers`                       |
| GVRET          | `gvret:on`     | `gvretClients`                      |
| ScanMyTesla    | `smt:on`       | `scanMyTesla`                       |
| ELM327         | `elm327:on`    | `elm327`                            |

## Related References

- [Command Reference](commands.md)
- [WiFi API](wifi-api.md)
- [BLE Adapter](ble-adapter.md)
- [Vehicle Features](vehicle-features.md)
