---
title: Bluetooth (BLE)
title_tr: Bluetooth (BLE)
description: NimBLE Bluetooth Low Energy setup and UART service
category: reference
folder: reference
tags: [ble, bluetooth, adapter]
order: 9
icon: 📶
---

# Bluetooth Low Energy (BLE)

ESP32 firmware variants with BLE enabled (`esp32_ble`, `esp32_wifi_ble`) use **NimBLE** for Bluetooth Low Energy communication. This works natively with iOS and Android — no pairing PIN required.

## Overview

| Property           | Value                          |
| ------------------ | ------------------------------ |
| Device Name        | `TeslaCANModder`               |
| Protocol           | BLE GATT (Nordic UART Service) |
| Library            | NimBLE-Arduino                 |
| iOS Compatible     | Yes                            |
| Android Compatible | Yes                            |
| Simultaneous WiFi  | Yes                            |

## Nordic UART Service (NUS)

TeslaCANModder uses the Nordic UART Service UUIDs, which are widely supported by BLE terminal apps:

| Characteristic  | UUID                                   | Direction      |
| --------------- | -------------------------------------- | -------------- |
| **Service**     | `6e400001-b5a3-f393-e0a9-e50e24dcca9e` | —              |
| **RX** (write)  | `6e400002-b5a3-f393-e0a9-e50e24dcca9e` | Phone → Device |
| **TX** (notify) | `6e400003-b5a3-f393-e0a9-e50e24dcca9e` | Device → Phone |

## How to Connect

### iOS

1. Install a BLE UART app (e.g. **nRF Connect**, **LightBlue**, **Serial Bluetooth Terminal**)
2. Scan for BLE devices
3. Connect to **TeslaCANModder**
4. Subscribe to the TX characteristic for notifications
5. Write commands to the RX characteristic (e.g. `fsd:on\n`)

### Android

1. Install **Serial Bluetooth Terminal** or **nRF Connect**
2. Scan for BLE devices → connect to **TeslaCANModder**
3. Send commands as text, terminated with newline (`\n`)

## Command Protocol

BLE uses the exact same command protocol as USB Serial:

- Send commands as ASCII text terminated with `\n` (newline)
- Responses are JSON, same format as serial output
- All commands from the [Command Reference](commands) work over BLE
- Status messages, logs, and frame streams are mirrored to BLE

## Runtime Control

BLE can be enabled or disabled at runtime via the WiFi REST API (if WiFi is also enabled):

```bash
# Check BLE status
curl http://192.168.4.1/api/ble/status

# Disable BLE
curl -X POST http://192.168.4.1/api/ble/config \
  -H "Content-Type: application/json" \
  -d '{"enabled":false}'

# Re-enable BLE
curl -X POST http://192.168.4.1/api/ble/config \
  -H "Content-Type: application/json" \
  -d '{"enabled":true}'
```

BLE state is persisted in NVS flash and survives reboots.

The embedded HTML dashboard also has a BLE settings card with an enable/disable toggle.

## Technical Details

- **TX Power:** ESP_PWR_LVL_P9 (maximum range)
- **Auto-reconnect:** When a device disconnects, advertising restarts automatically
- **Buffer:** 256-byte ring buffer for received BLE data
- **Character validation:** Same as serial parser (a–z, A–Z, 0–9, colon, dash, underscore)
- **Max command length:** 31 characters
