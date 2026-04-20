---
title: Security Model
description: WiFi password and BLE pairing as the board access control model
category: guides
folder: guides
tags: [security, wifi, ble, authentication]
order: 6
icon: 🔒
---

# Security Model

This is a single-user, single-board project. The owner controls their own vehicle. The security model protects against **unauthorized nearby access** — not against the owner.

## Access Control Layers

### 1. WiFi Password (Primary)

The ESP32 runs a WPA2-PSK access point. Only clients that know the WiFi password can connect to the board's REST API.

- Default SSID: configured in firmware
- Password: set during first setup, stored in NVS
- All HTTP endpoints require WiFi connectivity
- CORS is set to `Access-Control-Allow-Origin: *` (safe because WiFi password gates access)

### 2. BLE Pairing (Secondary)

BLE connections use NimBLE with bonding. Only paired devices can send commands.

- Pairing mode must be explicitly triggered (`bleencrypt:pair`)
- AES-256-GCM encryption available for BLE traffic
- Unpaired devices cannot read or write characteristics

### 3. API Key (Optional)

An optional API key can be enabled for defense-in-depth:

- Disabled by default (`apiKeyRequired = false`)
- 32-character hex key auto-generated on first boot
- When enabled, mutable endpoints require `X-API-Key` header
- Enable: firmware NVS setting
- The key infrastructure exists if a user wants to opt in

## What Is NOT Restricted

These were intentionally removed or left unrestricted:

| Feature | Rationale |
| ------- | --------- |
| No command rate limiting | Owner sending commands to their own board |
| No feature timeouts | Owner controls when features activate/deactivate |
| No cooldown periods | Owner decides activation frequency |
| No auto-disable timers | Features stay active until owner disables them |
| Wildcard CORS | WiFi password is the access gate |
| Serial bridge binds 0.0.0.0 | Owner may access from phone on same network |

## OTA Safety

The only automatic safety gate is **OTA detection**. When a Tesla over-the-air update is detected on CAN ID 0x318, all CAN TX is paused (`txPaused = true`) to prevent bricking the vehicle during a firmware update. This protects the board itself, not user freedom.

## Recommendations

1. Use a strong WiFi password (12+ characters)
2. Enable BLE encryption if using Bluetooth control
3. Keep the board physically secured inside the vehicle
4. Enable API key if the board is accessible on a shared network
