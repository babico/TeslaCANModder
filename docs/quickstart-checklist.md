# Quickstart Checklist

Use this checklist to verify every step of a first-time setup. Check off each item as you go. If a step fails, see the linked troubleshooting section.

---

## 1. Hardware Preparation

- [ ] **Board selected**: Arduino Uno (1-bus USB) or ESP32-S DevKit (3-bus USB/WiFi/BLE)
- [ ] **MCP2515 module(s)** ready with **8 MHz crystal** — not 16 MHz
- [ ] **Data-capable USB cable** — charge-only cables will not work
- [ ] **Wiring complete** per [Hardware Setup](hardware-setup.md)
  - Arduino: CS → D10, INT → D2, SPI → D11/D12/D13
  - ESP32 Bus 0: CS → GPIO 15, INT → GPIO 34, SPI → 18/19/23
  - ESP32 Bus 1: CS → GPIO 27, INT → GPIO 35 *(optional)*
  - ESP32 Bus 2: CS → GPIO 26, INT → GPIO 33 *(optional)*
- [ ] **CAN bus wires** ready for X179 connector (CAN-H / CAN-L)

> **Stuck?** See [Troubleshooting → Board Not Connecting](troubleshooting.md#board-not-connecting)

---

## 2. Firmware Flashing

- [ ] Open TeslaCANModder in **Chrome or Edge** (WebSerial required)
- [ ] Go to **Flasher** tab
- [ ] Select the correct firmware variant for your board
- [ ] Click **Flash** — hold BOOT button on ESP32 if it stalls
- [ ] Wait for "Flash complete" confirmation
- [ ] Board auto-reboots — LED should light up

> **Stuck?** See [Troubleshooting → Board Not Connecting](troubleshooting.md#board-not-connecting)

---

## 3. First Connection

- [ ] Go to **Dashboard** tab
- [ ] Click **Connect USB**
- [ ] Console shows boot message with board info and variant
- [ ] Connection bar shows **CAN Active** or **Waiting** (if not in vehicle)

> **Stuck?** Install CH340/CP2102 USB drivers. Close Arduino IDE or other serial monitors.

---

## 4. Variant Selection

- [ ] Identify your vehicle: **HW4** (2023+), **HW3** (2019–2023), or **Legacy** (pre-HW3)
- [ ] Click the matching variant button in the connection bar
- [ ] Console confirms: `variant set to hw4` (or hw3/legacy)
- [ ] Setting is saved to NVS/EEPROM — persists across reboots

> **Stuck?** See [Vehicle Features](vehicle-features.md) for variant comparison

---

## 5. Vehicle Installation

- [ ] **Vehicle powered off** during installation
- [ ] Connect CAN-H / CAN-L to X179 connector:
  - FSD bus: Pins 13 (CAN-H) / 14 (CAN-L)
  - Vehicle bus: Pins 9 / 10 *(ESP32 3-bus only)*
  - Body bus: Pins 5 / 6 *(ESP32 3-bus only)*
- [ ] Secure board and wiring — no loose connections
- [ ] Power on vehicle — screen active
- [ ] Console should show **CAN Active** and frame count increasing

> **Stuck?** See [Troubleshooting → No CAN Frames](troubleshooting.md#no-can-frames)

---

## 6. Feature Activation

- [ ] All features start **OFF** by default — safe starting state
- [ ] Enable **FSD**: Dashboard → Controls → FSD → Enable
  - Requires FSD to be selected in the vehicle's UI first
- [ ] Enable **Nag Suppress**: Controls → Nag → Enable
- [ ] Set **Speed Profile**: Controls → Speed Profile → pick one (or Auto)
- [ ] *(Optional)* Enable **Streaming** to see live CAN frames in Monitor tab
- [ ] *(Optional)* Test vehicle commands (Locks, Mirrors, etc.) on Vehicle tab

> **Stuck?** See [Troubleshooting → FSD Not Activating](troubleshooting.md#fsd-not-activating)

---

## 7. Wireless Setup (ESP32 Only)

### WiFi

- [ ] Connect to AP: SSID `TeslaCANModder`, Password `teslacan123`
- [ ] Test: open `http://192.168.4.1` in browser
- [ ] *(Optional)* Configure STA mode to join your home network

### BLE

- [ ] Open mobile app or nRF Connect
- [ ] Scan for `TeslaCANModder` device
- [ ] Connect — should see boot message in console

> **Stuck?** See [Troubleshooting → WiFi Not Connecting](troubleshooting.md#wifi-not-connecting) or [BLE Not Working](troubleshooting.md#ble-not-working)

---

## 8. Verification Checklist

Run through these final checks before relying on the setup:

| Check | Expected | Status |
| ----- | -------- | ------ |
| Boot message in console | Board info + variant shown | ☐ |
| CAN bus status | "CAN Active" in connection bar | ☐ |
| FSD enable/disable | Console confirms state change | ☐ |
| Nag enable/disable | Console confirms state change | ☐ |
| Stream on | Frames appear in Monitor tab | ☐ |
| Standby recovery | Board resumes after car wake | ☐ |

---

## Quick Reference

| Action | Command |
| ------ | ------- |
| Ping board | `ping` |
| Check status | `status` |
| Enable FSD | `fsd:on` |
| Disable FSD | `fsd:off` |
| Enable Nag Suppress | `nag:on` |
| Set Profile (Normal) | `profile:1` |
| Auto Profile | `profile:auto` |
| Start streaming | `stream:on` |
| Stop streaming | `stream:off` |

For all commands, see [Commands Reference](commands.md).
