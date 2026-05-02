# Hardware Reference

`hardware/` is a reference snapshot and scratch area, not the shipping firmware workspace.

## Status

- Active release firmware lives in `firmware/`.
- CI, release artifacts, and PlatformIO commands target `firmware/`.
- Files here are useful for comparison, archived experiments, and test references only.

## Contents

| Path             | Purpose                                       |
| ---------------- | --------------------------------------------- |
| `hardware/lib/`  | Archived headers and implementation fragments |
| `hardware/test/` | Reference tests tied to the archived layout   |

## Guidance

- Add new shipping firmware work under `firmware/`.
- Update wiring and setup docs under `docs/guides/`.
- Treat this folder as read-only unless you are intentionally reconciling old experiments into the active firmware tree.

````

### Ack/Error

```json
{"t":"ack","cmd":"fsd:on"}
{"t":"error","msg":"Invalid variant"}
````

## Code Structure

```
hardware/
├── lib/
│   ├── core/
│   │   ├── types.h        - Frame, State, Features, Variant
│   │   ├── forward.h      - Forward declarations
│   │   ├── config/        - Pin definitions per platform
│   │   ├── driver/        - MCP2515 hardware interface per platform
│   │   └── persist/       - Settings save/load per platform
│   ├── infra/
│   │   ├── can.h          - CAN IDs, bus defs, frame helpers
│   │   └── burst.h        - Non-blocking burst send
│   ├── feature/
│   │   ├── <name>/cmd.h      - Command parser
│   │   ├── <name>/protocol.h - CAN frame builder
│   │   └── ...            - 27 feature folders
│   ├── handler/
│   │   ├── hw4.h          - HW4 message handler
│   │   ├── hw3.h          - HW3 message handler
│   │   ├── legacy.h       - Legacy message handler
│   │   └── dispatch/      - Platform-specific message routing
│   └── io/
│       ├── serial/        - Serial/BT command router per platform
│       ├── wifi/          - WiFi REST API + dashboard (ESP32)
│       └── ble/           - BLE GATT service (ESP32)
├── src/
│   └── esp32/main.cpp
└── platformio.ini         - Build configuration
```

### Module Responsibilities

**Core Infrastructure (core/):**

- **types.h** - Core data structures (Frame, State, Features, Variant)
- **forward.h** - Forward declarations for cross-layer references
- **config/** - Hardware pin assignments per platform
- **driver/** - Low-level MCP2515 SPI communication per platform
- **persist/** - Settings save/load (NVS on ESP32)

**Infrastructure (infra/):**

- **can.h** - CAN ID definitions, bus constants, frame helpers
- **burst.h** - Non-blocking burst send (`startBurst` + `burstTick`)

**Feature Layer (feature/):**

Each feature has its own folder with `cmd.h` (command parser) and optionally
`protocol.h` (CAN frame builder). Examples:

- **charge/** - Charge start/stop/port control
- **trunk/** - Trunk/frunk/glovebox one-shot commands
- **fsd/** - FSD enable/disable (cmd only, handler-based)
- **profile/** - Speed profile + follow distance mapping

**Handler Layer (handler/):**

- **hw4.h** - HW4-specific CAN message processing
- **hw3.h** - HW3-specific CAN message processing
- **legacy.h** - Legacy vehicle CAN message processing
- **dispatch/** - Routes messages to variant handlers, manages filters

**I/O Layer (io/):**

- **serial/** - Serial/Bluetooth command dispatch per platform
- **wifi/** - WiFi REST API server + embedded dashboard (ESP32)
- **ble/** - BLE GATT Nordic UART service (ESP32)

## Default State

- Variant: HW4
- FSD: OFF
- Nag Suppression: OFF
- Speed Profile: 1 (Standard)
- Speed Offset: 0
- ISA Chime Suppression: OFF
- Frame Streaming: OFF
- Raw CAN Listen: OFF

## License

GPL-3.0 - See legacy reference projects for original implementations.
