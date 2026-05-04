# IO Schemas

Unified machine-readable schema for all firmware IO transports.

## Files

- `io.schema.json` — Single source of truth for all IO. Covers serial, BLE, and WiFi.

## Transport Summary

| Transport         | Protocol               | Direction     | Auth                |
| ----------------- | ---------------------- | ------------- | ------------------- |
| Serial (USB CDC)  | Newline-delimited JSON | Bidirectional | None                |
| BLE (Nordic UART) | Newline-delimited JSON | Bidirectional | None                |
| WiFi (HTTP REST)  | JSON over HTTP         | Bidirectional | X-API-Key on writes |

Serial and BLE share an **identical command/response protocol** — the same command strings and the same output message shapes apply to both. The schema's `oneOf` block validates any serial/BLE output message.

WiFi uses a **separate flat JSON state shape** (native `true`/`false` booleans, not `0`/`1`) returned by `GET /status` and `POST /cmd`. This shape is defined as `#/$defs/WiFiState`.

## Schema Top-level Properties

| Property        | Description                                                                     |
| --------------- | ------------------------------------------------------------------------------- |
| `schemaVersion` | Schema format version (current: 1)                                              |
| `transports`    | Metadata for each IO transport (serial, ble, wifi) with UUIDs, endpoints, auth  |
| `commands`      | Flat list of all valid command strings with descriptions and feature cross-refs |
| `messages`      | Serial/BLE output message manifest (tag → schema ref → transports)              |
| `features`      | Feature manifest: id, kind, commands, outputTags, statePaths                    |
| `oneOf`         | JSON Schema validator for serial/BLE output messages                            |
| `$defs`         | All shared type definitions                                                     |

## Notable `$defs`

| Def                           | Description                                     |
| ----------------------------- | ----------------------------------------------- |
| `CommandInput`                | Plain-text command string (serial/BLE)          |
| `WiFiCommand`                 | `{ cmd: string }` request body for `POST /cmd`  |
| `WiFiState`                   | Full device state returned by the WiFi REST API |
| `CanBusValues`                | `{ chassis, vehicle, body }` integer triple     |
| `CanHealth`                   | Per-bus `{ on, det }` health flags              |
| `CanStatus`                   | Full CAN diagnostics block used in Boot/Status  |
| `Boot` / `Status`             | Full snapshot messages                          |
| `StatusCompact`               | Compact snapshot message                        |
| `Bms` / `Tpms` / `Powertrain` | Telemetry messages                              |

## Boolean Encoding

- **Serial / BLE**: booleans are encoded as integers `0` or `1` (see `Bool01` def)
- **WiFi REST**: booleans are native JSON `true` / `false`

## Notes

- Run `npm run validate:serial-contract` from the repo root to validate serial output against the `oneOf` + `$defs` in this schema.
