# TeslaCANModder — Debug Tool

Modular CLI for board debugging, CAN bus analysis, and vehicle control.

## Quick Start

```bash
npm install          # install dev deps (jest); serialport is a peer dep
node debug.js smoke --port COM3
```

## Commands

| Command       | Description                         |
| ------------- | ----------------------------------- |
| `smoke`       | Protocol health-check (default)     |
| `watch`       | Live CAN frame / state monitor      |
| `test`        | FSD & profile round-trip tests      |
| `flash`       | Flash firmware via avrdude          |
| `scan`        | Discover active CAN IDs             |
| `dump`        | Record frames to JSONL / CSV        |
| `replay`      | Replay a recorded JSONL dump        |
| `benchmark`   | Measure CAN throughput & latency    |
| `vehicle`     | Send vehicle control commands       |

## Global Options

| Flag              | Default       | Description                     |
| ----------------- | ------------- | ------------------------------- |
| `--port`          | *(required)*  | Serial port (e.g. `COM3`)      |
| `--baud`          | `115200`      | Baud rate                       |
| `--variant`       | —             | Board variant (`hw3` / `hw4`)   |
| `--timeout`       | `3000`        | Command timeout (ms)            |
| `--warmup`        | `1500`        | Boot settle delay (ms)          |
| `--no-color`      | `false`       | Disable ANSI colour output      |
| `--json`          | `false`       | Print summary as JSON           |
| `--verbose`       | `false`       | Extra diagnostic output         |

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
    └── vehicle.js        # Vehicle command sender
```

## Testing

```bash
npm test                 # runs jest
```
