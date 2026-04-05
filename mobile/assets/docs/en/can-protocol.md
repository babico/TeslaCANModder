# CAN Protocol

Technical details of the CAN bus communication protocol used by TeslaCANModder.

## Overview

The firmware communicates with the Tesla VehicleBus CAN network at 500 kbps. It reads and writes standard CAN 2.0A frames (11-bit IDs, up to 8 data bytes).

## Frame Format

Each CAN frame received by the firmware is forwarded to the serial/BT interface as JSON:

```json
{
  "type": "frame",
  "id": 1160,
  "dlc": 8,
  "data": "0102030405060708",
  "dir": "rx",
  "seq": 42,
  "bus": 0
}
```

| Field | Type | Description |
|-------|------|-------------|
| type | string | Always "frame" |
| id | number | CAN ID (decimal, 11-bit) |
| dlc | number | Data length code (0–8) |
| data | string | Hex-encoded data bytes |
| dir | string | "rx" (received) or "tx" (transmitted) |
| seq | number | Sequence counter |
| bus | number | 0 = primary MCP2515, 1 = secondary |

## Known CAN IDs

| CAN ID | Hex | Description |
|--------|-----|-------------|
| 1160 | 0x488 | VehicleBus status |
| 881 | 0x371 | FSD control |
| 962 | 0x3C2 | Nag suppression |
| 599 | 0x257 | Speed profile |
| 1001 | 0x3E9 | Speed offset |
| 785 | 0x311 | ISA speed chime |
| 644 | 0x284 | Summon control |
| 1200 | 0x4B0 | Door/lock status |
| 801 | 0x321 | Climate control |
| 1024 | 0x400 | Charge status |
| 513 | 0x201 | Light control |
| 770 | 0x302 | Seat heating |

## Bus Speed & Timing

- **Baud Rate:** 500 kbps (VehicleBus standard)
- **Crystal:** 8 MHz on MCP2515 module
- **SPI Clock:** 8 MHz (Arduino SPI default)
- **Interrupt Mode:** Falling edge on INT pin

## Dual CAN Bus

When a second MCP2515 is connected:
- Bus 0 (primary): CS=D10, INT=D2 — VehicleBus
- Bus 1 (secondary): CS=D9, INT=D3 — Any secondary bus
- Both buses operate at 500 kbps independently
- The `bus` field in frame JSON indicates the source

## CAN Frame Decoding

The app includes a built-in CAN frame decoder that maps known IDs to human-readable descriptions. The decoder provides:

- ID label lookup (e.g., 0x488 → "VehicleBus Status")
- Byte-level field extraction for known frames
- Bit-diff highlighting for monitoring changes

## Serial Protocol

All communication between the board and app uses JSON lines (one JSON object per line, terminated by `\n`).

### Board → App Messages
- `{"type":"boot","hw":"uno","driver":"mcp2515","variant":"hw4",...}`
- `{"type":"status","fsd":true,"nag":false,"profile":1,...}`
- `{"type":"frame","id":1160,"dlc":8,"data":"...","dir":"rx"}`
- `{"type":"ack","cmd":"fsd","ok":true}`
- `{"type":"error","msg":"unknown command"}`
- `{"type":"log","msg":"CAN bus recovered"}`
- `{"type":"pong"}`

### App → Board Commands
- `{"cmd":"ping"}`
- `{"cmd":"fsd","on":true}`
- `{"cmd":"stream","on":true}`
- etc.
