---
title: WiFi REST API
title_tr: WiFi REST API
description: HTTP endpoints and wireless AP configuration
category: reference
folder: reference
tags: [wifi, api, rest, http]
order: 10
icon: 📡
---

# WiFi REST API

ESP32 firmware envs with WiFi enabled (any env containing `_wifi`, e.g. `esp32_wifi_chassis_8mhz`, `esp32_wifi_ble_chassis_vehicle_body_8mhz`) create a wireless access point for HTTP control.

## WiFi Modes

### Access Point (AP) Mode — Default

The ESP32 creates its own WiFi network:

| Setting     | Default Value    |
| ----------- | ---------------- |
| SSID        | `TeslaCANModder` |
| Password    | `T3SL@c@n123.`   |
| IP Address  | `192.168.4.1`    |
| Port        | `80`             |
| Channel     | `6`              |
| Max Clients | `4`              |

### Station (STA) Mode

Connect the ESP32 to your existing WiFi network:

1. Open the embedded dashboard at `http://192.168.4.1`
2. Switch WiFi mode to **STA**
3. Enter your network SSID and password
4. Click **Connect**

If STA connection fails, the device automatically falls back to AP mode.

> WiFi configuration is saved to NVS flash and persists across reboots.

## REST Endpoints

### System

| Method | Endpoint       | Description                                 |
| ------ | -------------- | ------------------------------------------- |
| `GET`  | `/`            | Embedded HTML dashboard                     |
| `GET`  | `/api/ping`    | Health check — returns `{"t":"pong","v":1}` |
| `GET`  | `/api/status`  | Full board state JSON                       |
| `GET`  | `/api/disable` | Emergency disable all injections            |

### Command Execution — the One True Endpoint

| Method | Endpoint       | Description                                    |
| ------ | -------------- | ---------------------------------------------- |
| `POST` | `/api/command` | Execute **any** wire command — preferred path. |

**Request body:**

```json
{ "cmd": "fsd:on" }
```

**Response:** an Ack — `{"t":"ack","cmd":"fsd:on"}`. Clients that need new state poll `GET /api/status` (or open the SSE/serial stream).

> **Design rule** — every new feature ships as a wire command, _not_ a new HTTP route.
> Wire commands work identically over Serial, BLE NUS, and HTTP, and stay in sync
> automatically across the docs/protocol/firmware cross-check test
> (`packages/protocol/test/integration/cross-check.test.ts`).
>
> The catalogue lives in [`commands.md`](commands.md). Examples:
> `gamepad:scan`, `gamepad:pair:AA:BB:CC:DD:EE:FF`, `gamepad:bind:0:drive:on`,
> `drive:on`, `drive:cap:25`, `apgate:status`.

#### When NOT to add a new HTTP route

Adding `server.on("/api/foo", …)` is almost never the right answer.
Use a wire command instead unless **all** of the following are true:

- The response is a binary payload (e.g. CSV file download), **or**
- The request is part of the auth/bootstrap chicken-and-egg (`/api/auth/key`), **or**
- The endpoint must be reachable before serial dispatch is available
  (e.g. `/api/disable` must work even if dispatch is wedged).

Anything else — toggles, configs, scans, pairings — belongs on `/api/command`.

#### Auth

When `apiKey` is set in `State`, mutating commands require the `X-API-Key` header:

```bash
curl -X POST http://192.168.4.1/api/command \
  -H "Content-Type: application/json" \
  -H "X-API-Key: $TCM_KEY" \
  -d '{"cmd":"drive:on"}'
```

`GET /api/status`, `GET /api/ping`, and the auth bootstrap endpoints are always open.

### WiFi Configuration

| Method | Endpoint           | Description                           |
| ------ | ------------------ | ------------------------------------- |
| `GET`  | `/api/wifi/status` | Current WiFi status and configuration |
| `POST` | `/api/wifi/config` | Change WiFi mode / credentials        |

**GET `/api/wifi/status`** — AP mode response:

```json
{
	"mode": "ap",
	"ssid": "TeslaCANModder",
	"ip": "192.168.4.1",
	"clients": 1,
	"channel": 6,
	"mac": "AA:BB:CC:DD:EE:FF"
}
```

**GET `/api/wifi/status`** — STA mode response:

```json
{
	"mode": "sta",
	"ssid": "MyHomeWiFi",
	"ip": "192.168.1.42",
	"rssi": -65,
	"connected": true,
	"gateway": "192.168.1.1",
	"mac": "AA:BB:CC:DD:EE:FF"
}
```

**POST `/api/wifi/config`** — Switch to STA:

```json
{
	"mode": "sta",
	"ssid": "MyHomeWiFi",
	"password": "mypassword"
}
```

**POST `/api/wifi/config`** — Switch to AP:

```json
{
	"mode": "ap",
	"ssid": "CustomName",
	"password": "mypassword8"
}
```

> AP password must be 8–64 characters or empty (open network).

### BLE Control

> **All BLE control is wire-command driven.** There are _no_ dedicated
> `/api/ble/*` REST endpoints anymore. Send commands via `POST /api/command`
> and read state from the `ble` sub-object of `GET /api/status`.
>
> | Wire command   | Effect                                                                          |
> | -------------- | ------------------------------------------------------------------------------- |
> | `ble:on`       | Start the BLE radio; persist `enabled=true` to NVS                              |
> | `ble:off`      | Stop the BLE radio; persist `enabled=false` to NVS                              |
> | `ble:name:<n>` | Update advertised device name (1–32 chars); persist to NVS                      |
> | `ble:status`   | Emit `{"t":"ble","enabled":...,"connected":...,"deviceName":...}` on the stream |
>
> The `ble` sub-object inside `GET /api/status` exposes the same fields
> (`enabled`, `connected`, `deviceName`) for single-poll dashboards.

### TPMS & Diagnostics

| Method | Endpoint   | Description                              |
| ------ | ---------- | ---------------------------------------- |
| `GET`  | `/api/log` | Debug ring buffer dump (last 256 events) |

> TPMS readings are exposed as the `tpms` sub-object inside `GET /api/status`
> — the standalone `/api/tpms` endpoint has been removed.

### Gamepad (BLE central)

> **All gamepad operations are wire commands** — there are _no_ dedicated
> gamepad routes worth using directly. Send them via `POST /api/command`:
>
> | Wire command                           | Effect                                      |
> | -------------------------------------- | ------------------------------------------- |
> | `gamepad:on` / `gamepad:off`           | Enable/disable BLE central role             |
> | `gamepad:scan`                         | Start a BLE scan for nearby HID gamepads    |
> | `gamepad:pair:AA:BB:CC:DD:EE:FF`       | Pair a specific MAC                         |
> | `gamepad:unpair`                       | Forget paired device                        |
> | `gamepad:bind:<n>:<cmd>`               | Bind button `n` (0..15) to a wire command   |
> | `gamepad:hold:<n>:<cmd>`               | Long-press binding (≥500 ms) for button `n` |
> | `gamepad:axis:<n>:<dz\|expo\|inv>:<v>` | Per-axis tuning (dz/expo/invert)            |
> | `gamepad:cancel`                       | One-shot DAS cancel burst (panic-stop)      |
> | `gamepad:status`                       | Emit a `gamepad` JSON line on the stream    |

### CAN Recorder

> All recorder control is via wire commands. Read state from the `recorder`
> sub-object inside `GET /api/status` (`enabled`, `count`, `capacity`,
> `captured`, `dropped`, `lastCaptureMs`).
>
> | Wire command      | Effect                                              |
> | ----------------- | --------------------------------------------------- |
> | `recorder:on`     | Start capturing all CAN frames into the ring buffer |
> | `recorder:off`    | Stop capturing                                      |
> | `recorder:clear`  | Reset the buffer and stats                          |
> | `recorder:status` | Emit `{"t":"recorder",...}` line                    |
>
> The binary `GET /api/recorder/download` (CSV file) has no wire equivalent
> and remains a dedicated endpoint.

**`tpms` sub-object inside `GET /api/status`:**

```json
{
	"ok": true,
	"fl": 2.45,
	"fr": 2.5,
	"rl": 2.48,
	"rr": 2.47,
	"tfl": 25,
	"tfr": 26,
	"trl": 24,
	"trr": 25
}
```

Pressures are in bar, temperatures in °C. `ok` is `false` when no TPMS data has been received yet.

**GET `/api/log`** response:

```json
[
	{ "ts": 12345, "msg": "FSD enabled" },
	{ "ts": 12400, "msg": "ECE R79 bypass active" }
]
```

**`ble` sub-object inside `GET /api/status`:**

```json
{
	"enabled": true,
	"connected": false,
	"deviceName": "TeslaCANModder"
}
```

**Disable BLE via wire command:**

```bash
curl -X POST http://192.168.4.1/api/command \
  -H "Content-Type: application/json" \
  -d '{"cmd":"ble:off"}'
```

> BLE state is saved to NVS and persists across reboots.

## Example Requests

```bash
# Check status
curl http://192.168.4.1/api/status

# Enable FSD
curl -X POST http://192.168.4.1/api/command \
  -H "Content-Type: application/json" \
  -d '{"cmd":"fsd:on"}'

# Set speed profile
curl -X POST http://192.168.4.1/api/command \
  -H "Content-Type: application/json" \
  -d '{"cmd":"profile:3"}'

# Enable nag killer (EPAS torque spoofing)
curl -X POST http://192.168.4.1/api/command \
  -H "Content-Type: application/json" \
  -d '{"cmd":"nag:killer:on"}'

# Start battery preconditioning
curl -X POST http://192.168.4.1/api/command \
  -H "Content-Type: application/json" \
  -d '{"cmd":"precondition:on"}'

# Enable track mode
curl -X POST http://192.168.4.1/api/command \
  -H "Content-Type: application/json" \
  -d '{"cmd":"trackmode:on"}'

# Query BMS telemetry
curl -X POST http://192.168.4.1/api/command \
  -H "Content-Type: application/json" \
  -d '{"cmd":"bms"}'

# Switch to STA mode
curl -X POST http://192.168.4.1/api/wifi/config \
  -H "Content-Type: application/json" \
  -d '{"mode":"sta","ssid":"MyWiFi","password":"pass1234"}'

# Check WiFi status
curl http://192.168.4.1/api/wifi/status

# Disable BLE
curl -X POST http://192.168.4.1/api/command \
  -H "Content-Type: application/json" \
  -d '{"cmd":"ble:off"}'

# Emergency disable all modifications
curl http://192.168.4.1/api/disable

# Query TPMS data (read the `tpms` sub-object of /api/status)
curl http://192.168.4.1/api/status

# Set drive mode to Performance
curl -X POST http://192.168.4.1/api/command \
  -H "Content-Type: application/json" \
  -d '{"cmd":"drivemode:performance"}'

# Enable ECE R79 bypass
curl -X POST http://192.168.4.1/api/command \
  -H "Content-Type: application/json" \
  -d '{"cmd":"ecer79:on"}'

# Dump debug log
curl http://192.168.4.1/api/log
```

## Embedded Dashboard

The ESP32 serves a built-in HTML dashboard at `http://192.168.4.1/` with:

- Real-time status display (variant, CAN online, FSD, nag, profile)
- WiFi settings (AP/STA mode switching, credentials)
- BLE settings (enable/disable toggle, connection status)
- Hardware variant selector (HW4 / HW3 / Legacy)
- FSD toggle controls (FSD, nag, ISA chime, speed profile)
- Vehicle commands (lock, trunk, mirrors, climate, summon, etc.)
- System log output
- Powertrain telemetry panel (speed, gear, pedal, steering, motor RPMs)
- Turn signal, seatbelt emulation, wiper persist, mirror auto-fold controls
- CAN simulation mode for bench testing

## Phase 2 Status Fields

The `/api/status` response now includes the following additional fields:

| Field               | Type    | Description                                    |
| ------------------- | ------- | ---------------------------------------------- |
| `seatbeltEmulation` | boolean | Rear seatbelt emulation enabled                |
| `wiperPersist`      | boolean | Wiper speed persistence enabled                |
| `mirrorAutoFold`    | boolean | Mirror auto-fold on lock enabled               |
| `canSim`            | boolean | CAN simulation mode active                     |
| `hasPowertrain`     | boolean | Powertrain telemetry data available            |
| `powertrain`        | object  | Powertrain data (when `hasPowertrain` is true) |
| `powertrain.speed`  | number  | Vehicle speed (km/h)                           |
| `powertrain.gear`   | number  | Gear state (1=P, 2=R, 3=N, 4=D)                |
| `powertrain.pedal`  | number  | Accelerator pedal (0–100%)                     |
| `powertrain.steer`  | number  | Steering angle (degrees, ÷10)                  |
| `powertrain.rpmR`   | number  | Rear motor RPM                                 |
| `powertrain.rpmF`   | number  | Front motor RPM                                |

## CORS

All API endpoints include CORS headers (`Access-Control-Allow-Origin: *`) for cross-origin requests from the web UI.
