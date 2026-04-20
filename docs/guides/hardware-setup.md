---
title: Hardware Setup
title_tr: Donanım Kurulumu
description: Wiring diagrams and component lists for all supported boards
category: guides
folder: guides
tags: [hardware, setup, wiring]
order: 3
icon: 🔧
---

# Hardware Setup

## Arduino Uno

### Required Components

- **Arduino Uno R3** — CH340 or ATmega16U2 USB chip
- **MCP2515 CAN Module #1** — 8 MHz crystal + TJA1050 transceiver
- **9V-36V to 5V Buck Converter** — 3A minimum, powers from vehicle 12V
- **Tesla X179 Connector** — Behind center screen, provides 12V + CAN access

### Optional Components

- **HC-05 Bluetooth Module** — Wireless control, requires voltage divider on RX
- **MCP2515 CAN Module #2** — Second CAN bus, shares SPI
- **MCP2515 CAN Module #3** — Third CAN bus, shares SPI

### Wiring — MCP2515 #1 → Arduino Uno

| MCP2515 | Arduino | Notes |
| ------- | ------- | ----- |
| VCC | 5V | |
| GND | GND | |
| CS | D10 | SPI chip select |
| INT | D2 | Interrupt (INT0) |
| SCK | D13 | SPI clock |
| MISO | D12 | SPI data out |
| MOSI | D11 | SPI data in |

Connect CAN-H/CAN-L to X179 pins 13/14 (FSD bus).

### Wiring — MCP2515 #2 → Arduino Uno (Optional)

| MCP2515 #2 | Arduino | Notes |
| ---------- | ------- | ----- |
| VCC | 5V | Shared rail |
| GND | GND | Shared rail |
| CS | D9 | Unique chip select |
| INT | D3 | Interrupt (INT1) |
| SCK | D13 | Shared with #1 |
| MISO | D12 | Shared with #1 |
| MOSI | D11 | Shared with #1 |

> Arduino Uno has exactly 2 hardware interrupts: D2 (INT0) and D3 (INT1). Do not reuse D3 when using the Vehicle bus (`BUS_VEHICLE_ACTIVE=1`).

### Wiring — HC-05 Bluetooth (Optional)

| HC-05 | Arduino | Notes |
| ----- | ------- | ----- |
| VCC | 5V | |
| GND | GND | |
| RX | D4 | Via voltage divider (5V → 3.3V) |
| TX | D5 | Direct connection |

> HC-05 RX is 3.3V logic. Use a 1kΩ + 2kΩ voltage divider on D4 → HC-05 RX.

---

## ESP32-S DevKit

The ESP32 uses **3× MCP2515 modules over SPI** for all CAN buses.

### Required Components

- **ESP32-S DevKit (30-pin or 38-pin)** — Built-in WiFi + BLE
- **MCP2515 CAN Module ×1–3** — 8 MHz crystal + TJA1050 transceiver each
- **9V-36V to 5V Buck Converter** — 3A minimum
- **Tesla X179 Connector** — Behind center screen

> No external Bluetooth module needed! ESP32 has built-in BLE (Bluetooth Low Energy). WiFi and BLE can operate simultaneously.

### Wiring — MCP2515 #1 (Bus 0: FSD)

| MCP2515 #1 | ESP32 | Notes |
| ---------- | ----- | ----- |
| VCC | 5V (VIN) | MCP2515 needs 5V |
| GND | GND | |
| CS | GPIO 15 | SPI chip select |
| INT | GPIO 34 | Input-only pin, good for interrupt |
| SCK | GPIO 18 | VSPI clock |
| MISO | GPIO 19 | VSPI data out |
| MOSI | GPIO 23 | VSPI data in |

Connect CAN-H/CAN-L to **X179 pins 13/14** (FSD / Autopilot CAN).

### Wiring — MCP2515 #2 (Bus 1: Vehicle Control)

| MCP2515 #2 | ESP32 | Notes |
| ---------- | ----- | ----- |
| VCC | 5V (VIN) | Shared rail |
| GND | GND | Shared rail |
| CS | GPIO 27 | Unique chip select |
| INT | GPIO 35 | Input-only pin |
| SCK | GPIO 18 | Shared with #1 |
| MISO | GPIO 19 | Shared with #1 |
| MOSI | GPIO 23 | Shared with #1 |

Connect CAN-H/CAN-L to **X179 pins 9/10** (Vehicle Control CAN).

### Wiring — MCP2515 #3 (Bus 2: Body Control)

| MCP2515 #3 | ESP32 | Notes |
| ---------- | ----- | ----- |
| VCC | 5V (VIN) | Shared rail |
| GND | GND | Shared rail |
| CS | GPIO 26 | Unique chip select |
| INT | GPIO 33 | |
| SCK | GPIO 18 | Shared with #1/#2 |
| MISO | GPIO 19 | Shared with #1/#2 |
| MOSI | GPIO 23 | Shared with #1/#2 |

Connect CAN-H/CAN-L to **X179 pins 2/3** (Body Control CAN).

---

## X179 Vehicle Connection

The X179 connector is located behind the center screen in Tesla vehicles.

### Tesla X179 Connector — Bus Mapping

| X179 Pin | Connection | Purpose |
| -------- | --------- | ------- |
| Pin 1 | Buck converter VIN+ | 12V power |
| Pin 20 | Buck converter VIN- | Ground |
| Pin 13 | MCP2515 #1 CAN-H | FSD / Autopilot CAN high |
| Pin 14 | MCP2515 #1 CAN-L | FSD / Autopilot CAN low |
| Pin 9 | MCP2515 #2 CAN-H | Vehicle Control CAN high |
| Pin 10 | MCP2515 #2 CAN-L | Vehicle Control CAN low |
| Pin 2 | MCP2515 #3 CAN-H | Body Control CAN high |
| Pin 3 | MCP2515 #3 CAN-L | Body Control CAN low |

> Buck converter 5V output → ESP32 VIN pin or Arduino USB port.

### Installation Steps

1. Disconnect 12V battery before working on vehicle electrical systems
2. Remove center screen trim panel
3. Locate X179 connector (white 20-pin)
4. Connect your harness (see wiring above)
5. Route cables to a hidden location
6. Secure board + modules with zip ties or velcro
7. Reinstall trim
