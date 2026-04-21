---
title: Quickstart Checklist
title_tr: Hızlı Başlangıç Listesi
description: Step-by-step first-time setup verification
category: guides
folder: guides
tags: [quickstart, checklist, setup]
order: 2
icon: ✅
---

# Quickstart Checklist

Use this checklist to verify every step of a first-time setup. Check off each item as you go. If a step fails, see the linked troubleshooting section.

For a detailed narrative setup flow that includes both browser flasher and CLI flasher paths, see the [Full Setup Guide](full-setup.md).

---

## 1. Hardware Preparation

- [ ] **Board selected**: ESP32-S DevKit (1–3 bus USB/WiFi/BLE)
- [ ] **MCP2515 module(s)** ready with **8 MHz crystal** — not 16 MHz
- [ ] **Data-capable USB cable** — charge-only cables will not work
- [ ] **Wiring complete** per [Hardware Setup](hardware-setup.md)
    - ESP32 Bus 0: CS → GPIO 15, INT → GPIO 34, SPI → 18/19/23
    - ESP32 Bus 1: CS → GPIO 27, INT → GPIO 35 _(optional)_
    - ESP32 Bus 2: CS → GPIO 26, INT → GPIO 33 _(optional)_
- [ ] **CAN bus wires** ready for X179 connector (CAN-H / CAN-L)

> **Stuck?** See [Troubleshooting → Board Not Connecting](../troubleshooting/debug-guide.md#board-not-connecting)

---

## 2. Firmware Flashing

- [ ] Open TeslaCANModder in **Chrome or Edge** (WebSerial required)
- [ ] Go to **Flasher** tab
- [ ] Select the correct firmware variant for your board
- [ ] Click **Flash** — hold BOOT button on ESP32 if it stalls
- [ ] Wait for "Flash complete" confirmation
- [ ] Board auto-reboots — LED should light up

> **Stuck?** See [Troubleshooting → Board Not Connecting](../troubleshooting/debug-guide.md#board-not-connecting)

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

> **Stuck?** See [Vehicle Features](../reference/vehicle-features.md) for variant comparison

---

## 5. Vehicle Installation

- [ ] **Vehicle powered off** during installation
- [ ] Connect CAN-H / CAN-L to X179 connector:
    - FSD bus: Pins 13 (CAN-H) / 14 (CAN-L)
    - Vehicle bus: Pins 9 / 10 _(ESP32 3-bus only)_
    - Body bus: Pins 2 / 3 _(ESP32 3-bus only)_
- [ ] Secure board and wiring — no loose connections
- [ ] Power on vehicle — screen active
- [ ] Console should show **CAN Active** and frame count increasing

> **Stuck?** See [Troubleshooting → No CAN Frames](../troubleshooting/debug-guide.md#no-can-frames)

---

## 6. Feature Activation

- [ ] All features start **OFF** by default — safe starting state
- [ ] Enable **FSD**: Dashboard → Controls → FSD → Enable
    - Requires FSD to be selected in the vehicle's UI first
- [ ] Enable **Nag Suppress**: Controls → Nag → Enable
- [ ] Set **Speed Profile**: Controls → Speed Profile → pick one (or Auto)
- [ ] _(Optional)_ Enable **Streaming** to see live CAN frames in Monitor tab
- [ ] _(Optional)_ Test vehicle commands (Locks, Mirrors, etc.) on Vehicle tab

> **Stuck?** See [Troubleshooting → FSD Not Activating](../troubleshooting/debug-guide.md#fsd-not-activating)

---

## 7. Wireless Setup (ESP32 Only)

### WiFi

- [ ] Connect to AP: SSID `TeslaCANModder`, Password `T3SL@c@n123.`
- [ ] Test: open `http://192.168.4.1` in browser
- [ ] _(Optional)_ Configure STA mode to join your home network

### BLE

- [ ] Open the client app or nRF Connect
- [ ] Scan for `TeslaCANModder` device
- [ ] Connect — should see boot message in console

> **Stuck?** See [Troubleshooting → WiFi Not Connecting](../troubleshooting/debug-guide.md#wifi-not-connecting) or [BLE Not Working](../troubleshooting/debug-guide.md#ble-not-working)

---

## 8. Verification Checklist

Run through these final checks before relying on the setup:

| Check                   | Expected                       | Status |
| ----------------------- | ------------------------------ | ------ |
| Boot message in console | Board info + variant shown     | ☐      |
| CAN bus status          | "CAN Active" in connection bar | ☐      |
| FSD enable/disable      | Console confirms state change  | ☐      |
| Nag enable/disable      | Console confirms state change  | ☐      |
| Stream on               | Frames appear in Monitor tab   | ☐      |
| Standby recovery        | Board resumes after car wake   | ☐      |

---

## Quick Reference

| Action               | Command        |
| -------------------- | -------------- |
| Ping board           | `ping`         |
| Check status         | `status`       |
| Enable FSD           | `fsd:on`       |
| Disable FSD          | `fsd:off`      |
| Enable Nag Suppress  | `nag:on`       |
| Set Profile (Normal) | `profile:1`    |
| Auto Profile         | `profile:auto` |
| Start streaming      | `stream:on`    |
| Stop streaming       | `stream:off`   |

For all commands, see [Command Reference](../reference/commands.md).
