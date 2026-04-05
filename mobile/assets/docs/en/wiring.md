# Wiring Guide

Detailed wiring diagrams for all hardware configurations.

## MCP2515 #1 → Arduino Uno (Required)

| MCP2515 | Arduino | Notes |
|---------|---------|-------|
| VCC | 5V | |
| GND | GND | |
| CS | D10 | SPI chip select |
| INT | D2 | Interrupt (INT0) |
| SCK | D13 | SPI clock |
| MISO | D12 | SPI data out |
| MOSI | D11 | SPI data in |

Connect CAN-H / CAN-L to X179 pins 13/14 (VehicleBus).

## MCP2515 #2 → Arduino Uno (Optional Dual CAN)

A second MCP2515 lets you monitor a second CAN bus (e.g. PowertrainBus). The firmware auto-detects it at boot.

| MCP2515 #2 | Arduino | Notes |
|------------|---------|-------|
| VCC | 5V | Shared rail |
| GND | GND | Shared rail |
| CS | D9 | Unique chip select |
| INT | D3 | Interrupt (INT1) |
| SCK | D13 | Shared with #1 |
| MISO | D12 | Shared with #1 |
| MOSI | D11 | Shared with #1 |

SPI lines (SCK/MISO/MOSI) are shared — only CS and INT differ.

**Warning:** Arduino Uno has exactly 2 hardware interrupts: D2 (INT0) and D3 (INT1). Do not reuse D3 when using dual CAN.

## HC-05 Bluetooth → Arduino Uno (Optional)

| HC-05 | Arduino | Notes |
|-------|---------|-------|
| VCC | 5V | |
| GND | GND | |
| RX | D4 | Via voltage divider (5V → 3.3V) |
| TX | D5 | Direct connection |

**Warning:** HC-05 RX is 3.3V logic. Use a 1kΩ + 2kΩ voltage divider on Arduino D4 → HC-05 RX line.

## X179 Vehicle Connection

| X179 Pin | Connection | Purpose |
|----------|------------|---------|
| Pin 1 | Buck converter VIN+ | 12V power |
| Pin 20 | Buck converter VIN- | Ground |
| Pin 13 | MCP2515 #1 CAN-H | VehicleBus high |
| Pin 14 | MCP2515 #1 CAN-L | VehicleBus low |

Buck converter 5V output → Arduino USB port. For MCP2515 #2, connect CAN-H/CAN-L to your chosen secondary bus connector pair.

## Power Notes

- Vehicle 12V is available whenever the car is "awake" (screen on, charging, driving)
- The buck converter provides stable 5V even during voltage dips
- Total current draw: ~150mA (Arduino + MCP2515 + HC-05)
- The board enters standby automatically when CAN bus goes silent
