# Tesla CAN Modder Tools

Active command-line tooling for firmware flashing, smoke tests, transport bridging, and runtime evidence capture.

## Active Entry Points

| Command                                       | Purpose                                  |
| --------------------------------------------- | ---------------------------------------- |
| `node tools/debug.js ...`                     | Main diagnostics CLI                     |
| `npm run bridge -- --port COM4 --listen 8080` | Serial-to-HTTP bridge for client testing |
| `tcm-debug`                                   | Package bin wrapper for `debug.js`       |

The old client docs generator is gone. The client now reads raw markdown from `docs/` directly.

## Quick Start

```bash
node tools/debug.js smoke --port COM3
npm run bridge -- --port COM4 --baud 115200 --listen 8080
```

## Core Commands

| Command         | Purpose                                                  |
| --------------- | -------------------------------------------------------- |
| `smoke`         | Board health check and protocol sanity                   |
| `test`          | FSD/profile round-trip validation                        |
| `flash`         | ESP32 flashing via esptool or PlatformIO-managed tooling |
| `watch`         | Live frame and status monitoring                         |
| `scan`          | CAN ID discovery                                         |
| `dump`          | Capture JSONL or CSV traces                              |
| `replay`        | Replay recorded traces                                   |
| `benchmark`     | Throughput and latency measurement                       |
| `vehicle`       | Send higher-level vehicle actions                        |
| `drive-context` | Capture D-05, D-11, and D-13 closure evidence            |

## Bridge Endpoints

`serial-http-bridge.js` exposes:

- `GET /health`
- `GET /api/status`
- `POST /api/command`

Use it when the board is attached to a development machine but the client is running on another device on the same network.

## Testing

```bash
npm test -w @teslacanmodder/tools
```

| `--drive-note-output` | — | Optional output file containing one validation note |
| `--drive-min-samples` | `1` | Minimum status samples required for valid evidence |
| `--drive-expect-full` | `false` | Fail command unless all closureReadiness gates pass |

Example:

```bash
node tools/debug.js drive-context --port COM5 --drive-duration 45000 --drive-output artifacts/drive-context-report.json

# strict mode (returns non-zero unless D-05/D-11/D-13 are all ready)
node tools/debug.js drive-context --port COM5 --drive-duration 60000 --drive-min-samples 10 --drive-expect-full --drive-output artifacts/drive-context-report.json --drive-note-output artifacts/drive-context-note.txt
```

## Architecture

```
tools/
├── debug.js              # Entry point — parses args, dispatches commands
├── package.json          # ESM config, deps
├── lib/
│   ├── args.js           # CLI argument parsing & option resolution
│   ├── output.js         # ANSI colours & structured logging
│   ├── session.js        # Serial port + BoardSession protocol layer
│   └── diagnosis.js      # Build feature matrix & diagnose board state
└── commands/
    ├── smoke.js          # Protocol health-check
    ├── watch.js          # Live frame monitor with bit-diff
    ├── test.js           # FSD / profile round-trip tests
    ├── flash.js          # ESP32 firmware flashing via esptool
    ├── scan.js           # CAN ID discovery
    ├── dump.js           # Frame recording
    ├── replay.js         # Frame playback
    ├── benchmark.js      # Throughput measurement
    ├── vehicle.js        # Vehicle command sender
    └── drive-context.js  # Drive-context evidence capture
```

## Running Tests

```bash
npm test                 # runs jest
```
