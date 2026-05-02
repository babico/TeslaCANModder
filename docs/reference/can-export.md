---
title: CAN Dataset Export
description: Export Tesla CAN decoder datasets to DBC, CSV, and summary JSON formats
category: reference
folder: reference
tags: [can, export, dbc, csv, decoder]
order: 6
---

# CAN Dataset Export

The `canExport` module converts Tesla CAN decoder JSON datasets into three portable formats: **DBC**, **CSV**, and **summary JSON**.

## Formats

### DBC (`.dbc`)

[DBC files](https://www.csselectronics.com/pages/can-dbc-file-database-intro) are the industry–standard format for CAN signal definitions, supported by SavvyCAN, Wireshark, Vector CANdb++, and most analysis tools.

> **Important:** The decoder JSON carries only signal _labels_ (enum values), not bit positions or widths. All `SG_` entries use a `0|8@1+` placeholder layout. **`VAL_` enum lines are fully accurate.** You can use the DBC as a starting point and add bit positions manually.

### CSV (`.csv`)

Flat table with one row per enum value — easy to import into Excel, pandas, or any data tool.

Columns:

| Column                 | Description                           |
| ---------------------- | ------------------------------------- |
| `frame_id_dec`         | CAN frame ID (decimal)                |
| `frame_id_hex`         | CAN frame ID (hex)                    |
| `frame_name`           | Frame / message name                  |
| `bus_name`             | Bus name (e.g. `ETH`, `VEH`, `CH`)    |
| `signal_name`          | Signal name within the frame          |
| `enum_symbol`          | Internal symbol name for the enum map |
| `possible_values_note` | Note when no enum map is available    |
| `value_dec`            | Enum value (decimal)                  |
| `value_hex`            | Enum value (hex)                      |
| `value_label`          | Human-readable label for this value   |

Signals with no enum values (numeric/bitfield only) still emit one row with the value columns left empty.

### Summary JSON (`.json`)

Compact, human-friendly JSON — strips redundant nesting and the verbose `possible_values_note` boilerplate. Useful for code review, diffs, and quick lookup.

```json
{
	"source": { "vehicle": "Model 3", "firmware": "2026.2", "mcu": "MCU2", "soc": "Intel" },
	"counts": { "frames": 577, "signals": 40484 },
	"frames": [
		{
			"id": 280,
			"hex": "0x118",
			"name": "DI_systemStatus",
			"bus": "ETH",
			"signals": [
				{
					"name": "DI_gear",
					"values": { "0": "INVALID", "1": "P", "2": "R", "3": "N", "4": "D", "7": "SNA" }
				}
			]
		}
	]
}
```

---

## CLI Usage

### Standalone (recommended for export-only tasks)

```bash
# Print DBC to stdout
node tools/commands/export.js --input client/assets/can-decoder/mcu2.json --format dbc

# Save CSV to file
node tools/commands/export.js -i client/assets/can-decoder/modelsx_intel.json -f csv -o output/modelsx_intel.csv

# Summary JSON (default format)
node tools/commands/export.js --input client/assets/can-decoder/mcu3.json
```

### Via `tcm-debug` (no `--port` required for export)

```bash
node tools/debug.js export --input client/assets/can-decoder/mcu2.json --format dbc --output out/mcu2.dbc
```

### Options

| Flag              | Short | Description                            | Default      |
| ----------------- | ----- | -------------------------------------- | ------------ |
| `--input <file>`  | `-i`  | Path to a decoder JSON file            | _(required)_ |
| `--format <fmt>`  | `-f`  | Output format: `dbc`, `csv`, or `json` | `json`       |
| `--output <file>` | `-o`  | Write output to file instead of stdout | stdout       |

---

## Programmatic API

Import the three functions directly in Node.js:

```js
import { toDbcString, toCsvString, toSummaryJson } from "@teslacanmodder/tools/lib/canExport.js";
import { readFileSync } from "node:fs";

const dataset = JSON.parse(readFileSync("client/assets/can-decoder/mcu2.json", "utf8"));

// DBC string
const dbc = toDbcString(dataset);

// CSV string
const csv = toCsvString(dataset);

// Summary object (call JSON.stringify to get text)
const summary = toSummaryJson(dataset);
```

All three functions are **pure** — they take a dataset object and return a string (or plain object for `toSummaryJson`). No file I/O.

---

## Available Datasets

Located in `client/assets/can-decoder/`:

| File                 | Vehicle   | MCU  | SoC   | Frames |
| -------------------- | --------- | ---- | ----- | ------ |
| `mcu2.json`          | Model 3   | MCU2 | Intel | 577    |
| `mcu3.json`          | Model 3   | MCU3 | AMD   | 577    |
| `modelsx_intel.json` | Model S/X | MCU2 | Intel | 1728   |
| `modelsx_amd.json`   | Model S/X | MCU3 | AMD   | 579    |

All datasets were generated from firmware **2026.2**.

---

## Running Tests

```bash
npm test -w @teslacanmodder/tools
```

The `canExport.test.js` suite covers all three export functions: edge cases, empty datasets, special characters, field quoting, and enum value mapping.
