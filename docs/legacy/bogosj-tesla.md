---
title: bogosj/tesla
category: legacy
folder: legacy
tags: [legacy, tesla, api, go, owner-api]
---

# bogosj/tesla

| Field | Value |
| ----- | ----- |
| Submodule | `legacy/bogosj-tesla` |
| URL | https://github.com/bogosj/tesla |
| Author | James Bogosian (forked from jsgoecke/tesla) |
| License | MIT |
| Language | Go |
| Primary focus | Unofficial Tesla Owner API client library |

## Overview

Go wrapper around the Tesla Owner API (documented at [tesla-api.timdorr.com](https://tesla-api.timdorr.com/)). Provides structures and helpers to query vehicle state and issue remote commands over HTTPS: wake, lock/unlock, climate, charge port, charging limits, horn, flash, etc. Includes an OAuth token helper in `cmd/login`.

## Relevant files

| Path | Notes |
| ---- | ----- |
| `client.go` | HTTP client, rate limiting, retry logic |
| `commands.go` | Remote command wrappers (`HonkHorn`, `FlashLights`, `SetChargeLimit`, etc.) |
| `states.go` | Vehicle state deserialization (charge, climate, drive, gui, vehicle config) |
| `vehicles.go` | Vehicle listing and selection |
| `auth.go` / `cmd/login` | Owner API token acquisition |

## Commands / endpoints of interest

| Endpoint | TeslaCANModder equivalent | Notes |
| ---------- | --------------------------- | ----- |
| `POST /api/1/vehicles/{id}/command/honk_horn` | `horn` | Both CAN and API paths exist |
| `POST /api/1/vehicles/{id}/command/flash_lights` | — | No CAN equivalent; API-only convenience |
| `POST /api/1/vehicles/{id}/command/door_lock` / `door_unlock` | `lock` / `unlock` | CAN is faster/locally authoritative |
| `POST /api/1/vehicles/{id}/command/set_charge_limit` | `charge:limit:<percent>` | Our BLE command uses same percent range |
| `POST /api/1/vehicles/{id}/command/set_charging_amps` | `tesla:charge:amps:<N>` | Verify 1–32 A mapping |
| `POST /api/1/vehicles/{id}/command/auto_conditioning_start` | `tesla:climate:on` | Climate-on over API vs BLE vs CAN |
| `POST /api/1/vehicles/{id}/command/wake_up` | `tesla:wake` | BLE wake uses VCSEC, not Owner API |

## Potential improvements for TeslaCANModder

1. **Rate-limit and error-code handling**
   The Go client maps HTTP 408/429 and vehicle-asleep errors explicitly. Our BLE/Fleet API surface (`tesla:*` commands) currently returns generic errors. We could surface richer `t` types (`error` with `code`) for `vehicle asleep`, `rate limited`, `timeout`.

2. **State field parity**
   `states.go` decodes fields such as `charge_energy_added`, `charger_power`, `inside_temp`, `outside_temp`, `tpms_pressure_fl`. Cross-check these against our BMS/climate/TPMS decoders to fill gaps in `status` / `bms` payloads.

3. **API-only commands as fallback**
   Features like `flash_lights` have no CAN equivalent. When WiFi + Owner API token is available, the ESP32 could proxy a small subset of API-only commands. This would live behind a new command namespace (`api:flash`) and is mostly a transport addition, not CAN logic.

## Safety / legal notes

- Owner API tokens are long-lived credentials. Any token storage on the ESP32 (NVS) must be flagged as sensitive and encrypted at rest if possible.
- This repo predates the Fleet API; new Tesla accounts may require Fleet API keys. Treat Owner API support as legacy/optional.
- Reference-only: do not copy OAuth or HTTP code into firmware.
