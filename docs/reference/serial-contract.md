---
title: Serial Contract
title_tr: Seri Kontrat
description: Wire format for commands and responses over USB, BLE, and WiFi transports
category: reference
folder: reference
tags: [serial, contract, wire-format, json-rpc, ble, wifi]
order: 5
icon: 🔌
---

# Serial Contract

The board accepts commands and emits responses on every transport. The wire format is identical on USB CDC, BLE Nordic UART (NUS), and the WiFi REST API. Every line on the wire is a single-line JSON envelope terminated by a single newline (`\n`, no carriage return).

```mermaid
flowchart LR
    Client([Client / Tool]) -- "{"cmd":"..."}\n" --> SerialParser
    SerialParser -- "handleChar()" --> Dispatch
    Dispatch -- ack / log / status --> SerialParser
    SerialParser -- "{t:...}\n" --> Client
    classDef transport fill:#1e3a5f,stroke:#4a7fb5,color:#fff
    class SerialParser transport
```

## Transport Comparison

| Transport | Frame          | Line ending      | Multiplex   |
| --------- | -------------- | ---------------- | ----------- |
| USB CDC   | ASCII line     | `\n`             | One CLI     |
| BLE NUS   | ASCII line     | `\n`             | One CLI     |
| WiFi REST | HTTP POST body | `Content-Length` | Per request |

All three share the same command vocabulary, response envelope, and field semantics. The BLE NUS and USB CDC transports are byte-for-byte compatible. WiFi REST maps one HTTP request to one command and returns the first non-empty response.

## Request Envelope

### Required shape

```json
{ "cmd": "<name>" }
```

The `cmd` field is a single case-sensitive lowercase string from the [Command Reference](./commands.md). Unknown commands return `{"t":"error","msg":"Unknown command"}` without mutating state.

### Optional fields

| Field   | Type   | Use                                                        |
| ------- | ------ | ---------------------------------------------------------- |
| `args`  | object | Command-specific key/value arguments (e.g. `{ "v": 100 }`) |
| `id`    | string | Client-assigned correlation id echoed in the response      |
| `reply` | bool   | Set `false` to suppress the response (fire-and-forget)     |

The current firmware accepts only `cmd`; extra fields are ignored. The shape is reserved for future extensions.

### Encoding

- One envelope per line.
- UTF-8 bytes.
- Single trailing `\n`. No carriage return.
- No pretty-printing. Compact JSON.
- Maximum line length: `SERIAL_CMD_BUFFER_SIZE` (firmware-configurable; current 768 bytes).

## Response Envelope

Every response is a single line of compact JSON. All responses include a `t` (type) field:

| `t` value | Purpose                                              |
| --------- | ---------------------------------------------------- |
| `ack`     | Command accepted, applied, and persisted as needed   |
| `error`   | Rejected, includes `msg` with human-readable reason  |
| `log`     | Informational side-effect (NVS save, boot, etc.)     |
| `status`  | Full board state snapshot                            |
| `boot`    | One-time message on firmware startup                 |
| `frame`   | Streamed CAN frame (only when `stream:on` is active) |
| `pong`    | Reply to `ping`                                      |
| `ack:<x>` | Reply to a future typed command `<x>`                |

### ack

```json
{ "t": "ack", "cmd": "fsd:on" }
```

Confirms the command was applied. For toggle commands (`fsd:on`, `lock`, etc.) the new state is included in the next `status` message, not in the ack.

### error

```json
{ "t": "error", "msg": "rpc: expected json object" }
```

Common error messages:

| `msg`                         | Cause                                                             |
| ----------------------------- | ----------------------------------------------------------------- |
| `rpc: expected json object`   | Line did not start with `{` (i.e. plain-text command)             |
| `rpc: expected {"cmd":"..."}` | JSON parsed but no `cmd` string field                             |
| `Unknown command`             | `cmd` not in the dispatch table                                   |
| `invalid arg:<name>`          | Argument out of range or wrong type                               |
| `feature disabled`            | Required build flag is off (e.g. `BUS_VEHICLE_ACTIVE` for summon) |

### status

Periodic snapshot of the full `State` struct. The first `status` message after boot has `meta.up` near zero. Subsequent messages every 5 seconds (or 250 ms when `status:live:on`). Field meanings are documented in [state-fields.md](./state-fields.md).

### frame

Emitted only when `stream:on` is active:

```json
{ "t": "frame", "dir": "rx", "bus": 0, "id": 1021, "dlc": 8, "d": "0102030405060708" }
```

`dir` is `rx` (received) or `tx` (transmitted). `bus` is the bus index (0=chassis, 1=vehicle, 2=body). `d` is the data field as 16 hex chars.

## Validation Workflow

The `npm run validate:serial-contract` script cross-checks:

- C++ `BootMessage` / `StatusMessage` schemas (`firmware/lib/interface/common/json.h`)
- TypeScript `BootMessage` / `StatusMessage` types (`packages/protocol/src/`)
- The `boot`, `status`, `frame`, `ack`, `log` field names that ship in this document

Run after any change to a schema, status field, or transport handler. The CI job runs it on every push and fails if any of the three sources drift apart.

## Firmware Source

- USB CDC parser: `firmware/lib/io/serial/usb/esp32/common.h` (`handleChar`)
- Mirror copy (legacy tree): `firmware/lib/interface/common/json.h`
- BLE NUS parser: `firmware/lib/ble/nus/esp32/common.h` (same `handleChar` semantics)
- HTTP request handler: `firmware/lib/io/wifi/api/command.h`
- Error sender: `sendError(F("rpc: ..."))`

When editing the wire format, update both the production copy and the legacy mirror in the same commit. The two files must stay byte-identical for the parser.

## Tooling

- `tools/lib/session.js` `BoardSession.send(cmd)` — auto-wraps plain text in JSON-RPC. Pass `{raw: true}` to push bytes verbatim.
- `tools/serial-http-bridge.js` `POST /api/command` — accepts `{ "cmd": "status" }` JSON bodies.
- `tools/debug.js` — the CLI wrapper for bench work. Use `node tools/debug.js ping`, `node tools/debug.js status`, etc.
