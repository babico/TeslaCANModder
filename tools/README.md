# TeslaCANModder — Debug Tool

Modular CLI for board debugging, CAN bus analysis, and vehicle control.

## Quick Start

```bash
npm install          # install dev deps (jest); serialport is a peer dep
node debug.js smoke --port COM3
```

## Expo Go Bridge (COM -> HTTP)

Use this when testing on Expo Go with a USB-attached ESP32 on your PC.

```bash
npm run bridge -- --port COM4 --baud 115200 --listen 8080
```

Bridge endpoints:

- `GET /health`
- `GET /api/status`
- `POST /api/command` with JSON body: `{ "cmd": "status" }`

In the client app, use HTTP transport and set base URL to your PC IP, for example:

- `http://192.168.1.23:8080`

Keep phone and PC on the same network.

## Commands

| Command         | Description                                          |
| --------------- | ---------------------------------------------------- |
| `smoke`         | Protocol health-check (default)                      |
| `watch`         | Live CAN frame / state monitor                       |
| `test`          | FSD & profile round-trip tests                       |
| `flash`         | Flash firmware via avrdude                           |
| `scan`          | Discover active CAN IDs                              |
| `dump`          | Record frames to JSONL / CSV                         |
| `replay`        | Replay a recorded JSONL dump                         |
| `benchmark`     | Measure CAN throughput & latency                     |
| `vehicle`       | Send vehicle control commands                        |
| `drive-context` | Capture D-05/D-11/D-13 evidence from status stream   |

Additional script:

- `npm run bridge -- --port COM4 --listen 8080` — Start serial HTTP bridge for Expo Go.

## Global Options

| Flag         | Default      | Description                      |
| ------------ | ------------ | -------------------------------- |
| `--port`     | *(required)* | Serial port (e.g. `COM3`)        |
| `--baud`     | `115200`     | Baud rate                        |
| `--variant`  | —            | Board variant (`hw3` / `hw4`)    |
| `--timeout`  | `3000`       | Command timeout (ms)             |
| `--warmup`   | `1500`       | Boot settle delay (ms)           |
| `--no-color` | `false`      | Disable ANSI colour output       |
| `--json`     | `false`      | Print summary as JSON            |
| `--verbose`  | `false`      | Extra diagnostic output          |

## Command-Specific Options

### watch

| Flag           | Default  | Description                    |
| -------------- | -------- | ------------------------------ |
| `--duration`   | `10000`  | Watch duration (ms)            |
| `--filter`     | —        | Filter CAN IDs (comma-sep)     |
| `--diff`       | `false`  | Show bit-level diff            |

### flash

| Flag        | Default  | Description                       |
| ----------- | -------- | --------------------------------- |
| `--hex`     | —        | Path to .hex firmware file        |
| `--erase`   | `false`  | Chip-erase before flash           |

Recommended flash workflow:

1. Build firmware first in `hardware/` (PlatformIO).
2. Confirm the target COM port is free.
3. Run flash command with explicit `.hex` path.
4. Verify serial boot/status output after flashing.

Example:

```bash
node tools/debug.js flash --port COM5 --hex hardware/.pio/build/uno/firmware.hex
```

With chip erase:

```bash
node tools/debug.js flash --port COM5 --hex hardware/.pio/build/uno/firmware.hex --erase
```

Notes:

- Flash command uses avrdude from PlatformIO package paths when present.
- If avrdude is not found, install PlatformIO or add avrdude to PATH.
- Flash command attempts post-flash serial verification and prints latest status summary when available.

### test

| Flag           | Default  | Description                    |
| -------------- | -------- | ------------------------------ |
| `--fsd-value`  | —        | FSD state to test              |
| `--profile`    | —        | Speed profile to test          |

### scan

| Flag           | Default  | Description                    |
| -------------- | -------- | ------------------------------ |
| `--duration`   | `5000`   | Scan listen window (ms)        |
| `--sort`       | `count`  | Sort by `id` or `count`        |

### dump

| Flag           | Default     | Description                   |
| -------------- | ----------- | ----------------------------- |
| `--duration`   | `10000`     | Record duration (ms)          |
| `--output`     | `dump.jsonl`| Output file path              |
| `--format`     | `jsonl`     | File format (`jsonl` / `csv`) |
| `--filter`     | —           | CAN IDs to capture            |

### replay

| Flag        | Default  | Description                       |
| ----------- | -------- | --------------------------------- |
| `--input`   | —        | JSONL file to replay              |
| `--speed`   | `1`      | Playback speed multiplier         |

### benchmark

| Flag           | Default  | Description                    |
| -------------- | -------- | ------------------------------ |
| `--duration`   | `10000`  | Benchmark window (ms)          |

### vehicle

| Flag        | Default  | Description                       |
| ----------- | -------- | --------------------------------- |
| `--action`  | —        | Vehicle command name              |

Available vehicle actions: `mirror-fold`, `mirror-unfold`, `mirror-left-down`, `mirror-left-up`, `mirror-right-down`, `mirror-right-up`, `lock`, `unlock`, `child-lock-on`, `child-lock-off`, `trunk-open`, `trunk-close`, `frunk-open`, `frunk-close`, `trunk-stop`, `headlights-on`, `headlights-off`, `fog-on`, `fog-off`, `hazards-on`, `hazards-off`, `turn-left`, `turn-right`, `wiper-single`, `wiper-continuous`, `wiper-auto`, `wiper-off`, `window-vent`, `window-close`, `sentry-on`, `sentry-off`, `climate-on`, `climate-off`, `charge-open`, `charge-close`, `charge-start`.

### drive-context

Captures runtime status evidence for remaining Drive validation tasks:

- D-05: turn signal + BSM activity
- D-11: doors/frunk/trunk open-state activity
- D-13: cruise/max/limit speed-context activity

Output report includes:

- latest captured snapshot values
- observed coverage flags for each task domain
- consistency errors (if any)
- `closureReadiness` gates for D-05, D-11, D-13 and an `allReady` recommendation
- `closureChecklist.missingScenarioIds` mapped to ESP32-14 scenario IDs still needed

| Flag                  | Default   | Description                                          |
| --------------------- | --------- | ---------------------------------------------------- |
| `--drive-duration`    | `30000`   | Capture window in ms                                 |
| `--drive-output`      | —         | Optional report file path (JSON)                     |
| `--drive-note-output` | —         | Optional output file containing one validation note  |
| `--drive-min-samples` | `1`       | Minimum status samples required for valid evidence   |
| `--drive-expect-full` | `false`   | Fail command unless all closureReadiness gates pass  |

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
    ├── flash.js          # Firmware flashing via avrdude
    ├── scan.js           # CAN ID discovery
    ├── dump.js           # Frame recording
    ├── replay.js         # Frame playback
    ├── benchmark.js      # Throughput measurement
    ├── vehicle.js        # Vehicle command sender
    └── drive-context.js  # Drive-context evidence capture
```

## Testing

```bash
npm test                 # runs jest
```
