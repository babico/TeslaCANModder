# Hardware Variants

Supported hardware combinations and their capabilities.

## Arduino Uno R3

The primary (and currently only) supported board.

| Feature | Detail |
|---------|--------|
| MCU | ATmega328P @ 16 MHz |
| Flash | 32 KB (firmware uses ~24 KB with full features) |
| SRAM | 2 KB |
| EEPROM | 1 KB (stores settings) |
| USB | CH340 or ATmega16U2 |
| Digital I/O | 14 pins (6 PWM) |
| SPI | D10-D13 (used by MCP2515) |
| Interrupts | D2 (INT0), D3 (INT1) |

### Pin Allocation

| Pin | Function | Required |
|-----|----------|----------|
| D2 | MCP2515 #1 INT | Yes |
| D3 | MCP2515 #2 INT | Only for Vehicle bus |
| D4 | HC-05 RX (SoftwareSerial) | Only for Bluetooth |
| D5 | HC-05 TX (SoftwareSerial) | Only for Bluetooth |
| D9 | MCP2515 #2 CS | Only for Vehicle bus |
| D10 | MCP2515 #1 CS | Yes |
| D11 | SPI MOSI | Yes |
| D12 | SPI MISO | Yes |
| D13 | SPI SCK | Yes |

## MCP2515 CAN Controller

| Feature | Detail |
|---------|--------|
| Protocol | CAN 2.0A / 2.0B |
| Speed | Up to 1 Mbps (500 kbps used) |
| Interface | SPI @ 10 MHz max |
| Transceiver | TJA1050 (on module) |
| Crystal | 8 MHz (critical — 16 MHz modules won't work) |
| Buffer | 2 receive, 3 transmit |
| Filters | 6 acceptance filters, 2 masks |

## HC-05 Bluetooth Module

| Feature | Detail |
|---------|--------|
| Bluetooth | Classic SPP (Serial Port Profile) |
| Baud Rate | 9600 default, configured to 115200 |
| Range | ~10 meters |
| Logic Level | 3.3V (needs voltage divider on RX) |
| Default PIN | 1234 |
| Power | 3.3V–6V |

### HC-05 Configuration

The firmware assumes HC-05 is configured for 115200 baud. To configure:

```
AT+UART=115200,0,0
AT+NAME=TeslaCANMod
AT+PSWD=1234
```

## Firmware Build Configurations

| Environment | BT | Use Case | Flash Size | RAM Usage |
|-------------|-----|----------|------------|-----------|
| uno | No | Serial only | ~18 KB | ~800 B |
| uno_bt | Yes | Serial + Bluetooth | ~22 KB | ~1.1 KB |

## Compatibility Notes

- **Crystal frequency matters:** Only 8 MHz MCP2515 modules are supported. 16 MHz modules require a different clock divider and won't initialize.
- **SPI sharing:** Both MCP2515 modules share SPi bus. The firmware handles CS arbitration.
- **Interrupt priority:** INT0 (D2) has higher priority than INT1 (D3). Primary bus gets first interrupt.
- **SoftwareSerial limitation:** HC-05 uses SoftwareSerial (D4/D5). Only one software serial can receive at a time.
