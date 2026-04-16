# WiFi REST API

ESP32 firmware variants with WiFi enabled (`esp32_wifi`, `esp32_wifi_ble`) create a wireless access point for HTTP control.

## WiFi Modes

### Access Point (AP) Mode — Default

The ESP32 creates its own WiFi network:

| Setting | Default Value |
| -------- | ------------- |
| SSID | `TeslaCANModder` |
| Password | `teslacan123` |
| IP Address | `192.168.4.1` |
| Port | `80` |
| Channel | `6` |
| Max Clients | `4` |

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

| Method | Endpoint | Description |
| ------ | -------- | ----------- |
| `GET` | `/` | Embedded HTML dashboard |
| `GET` | `/api/ping` | Health check — returns `{"t":"pong","v":1}` |
| `GET` | `/api/status` | Full board state JSON |
| `GET` | `/api/disable` | Emergency disable all injections |

### Command Execution

| Method | Endpoint | Description |
| ------- | --------- | ------------ |
| `POST` | `/api/command` | Execute any serial command |

**Request body:**
```json
{"cmd": "fsd:on"}
```

**Response:** Full board state JSON (same as `/api/status`).

All serial commands work over REST. See [Command Reference](commands) for the full list.

### WiFi Configuration

| Method | Endpoint | Description |
| ------- | --------- | ------------ |
| `GET` | `/api/wifi/status` | Current WiFi status and configuration |
| `POST` | `/api/wifi/config` | Change WiFi mode / credentials |

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

| Method | Endpoint | Description |
| ------- | --------- | ------------ |
| `GET` | `/api/ble/status` | BLE state (enabled, connected, device name) |
| `POST` | `/api/ble/config` | Enable or disable BLE at runtime |

**GET `/api/ble/status`** response:
```json
{
  "enabled": true,
  "connected": false,
  "deviceName": "TeslaCANModder"
}
```

**POST `/api/ble/config`** — Disable BLE:
```json
{"enabled": false}
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

# Switch to STA mode
curl -X POST http://192.168.4.1/api/wifi/config \
  -H "Content-Type: application/json" \
  -d '{"mode":"sta","ssid":"MyWiFi","password":"pass1234"}'

# Check WiFi status
curl http://192.168.4.1/api/wifi/status

# Disable BLE
curl -X POST http://192.168.4.1/api/ble/config \
  -H "Content-Type: application/json" \
  -d '{"enabled":false}'

# Emergency disable all modifications
curl http://192.168.4.1/api/disable
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

## CORS

All API endpoints include CORS headers (`Access-Control-Allow-Origin: *`) for cross-origin requests from the web UI.
