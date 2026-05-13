---
name: tools-development
description: Debug CLI and serial-to-HTTP bridge development for TeslaCANModder bench work and validation
license: GPL-3.0
compatibility: opencode
metadata:
    area: tools
    stack: nodejs, typescript
---

## What I do

Guide agents through the TeslaCANModder debug CLI and tooling in `tools/`. I cover:

- Adding CLI commands to the debug tool
- Working with the serial-to-HTTP bridge
- Creating workspace-level validation scripts
- Serial protocol parsing and device communication

## When to use me

Use this skill when:

- Modifying the debug CLI in `tools/`
- Working with the serial-to-HTTP bridge
- Adding new diagnostic or bench validation scripts
- Changing workspace-level validation scripts in `scripts/`

## Project structure

```
tools/
├── commands/           CLI command implementations
├── lib/                shared utility modules
├── test/               Jest ESM tests
├── debug.js            main CLI entry point
├── serial-http-bridge.js  serial-to-HTTP bridge
├── package.json        package manifest ("type": "module")
└── README.md           tools documentation

scripts/
├── npm-audit-actionable.cjs     security audit helper
├── release-matrix-smoke.mjs     release smoke tests
└── validate-serial-contract.mjs serial contract validator
```

## Tech stack

- Node.js >= 18
- TypeScript (where applicable)
- ESM modules (`"type": "module"` in package.json)
- Jest with ESM support
- Serialport library for serial communication

## Coding standards

- Strict TypeScript where available.
- ESM modules.
- `camelCase` for variables and functions.
- ESLint rules from root config apply, with CLI exemption for `console.log`.
- Prettier: tabs, tab width 4, print width 100, semicolons, double quotes, trailing commas everywhere.

## CLI commands

The debug CLI (`tools/debug.js`) provides bench work and validation commands:

```bash
node tools/debug.js <command> [options]
```

Common commands:

- `drive-context` - capture and validate drive context signals
- `monitor` - live CAN frame monitoring
- `flash` - firmware flashing helper
- `bridge` - start serial-to-HTTP bridge

Drive context validation (for runtime signal releases):

```bash
node tools/debug.js drive-context --port COM5 --drive-duration 60000 --drive-min-samples 10 --drive-expect-full --drive-output artifacts/drive-context-report.json --drive-note-output artifacts/drive-context-note.txt
```

## Serial-to-HTTP bridge

The bridge (`tools/serial-http-bridge.js`) proxies serial CAN frames to HTTP endpoints for integration testing.

## Testing

Run tools tests:

```bash
npm run test:tools
```

Tests use Jest with native ESM support. Add tests for:

- New CLI commands
- New utility functions
- Serial protocol parsing
- HTTP bridge behavior

## Scripts

Workspace-level scripts in `scripts/`:

- `validate-serial-contract.mjs` - Validates serial contract between firmware and protocol package
- `release-matrix-smoke.mjs` - Smoke tests for release firmware matrix
- `npm-audit-actionable.cjs` - Actionable security audit (run via `npm run audit:ci`)

## Before finishing

1. Run `npm run test:tools` and ensure all tests pass.
2. Run `npm run lint:all` from repo root.
3. If CLI commands changed, test them manually against a device or mock.
4. Clean up any temporary files.

## Key files to reference

- `tools/debug.js` - CLI entry point
- `tools/serial-http-bridge.js` - HTTP bridge
- `tools/commands/` - CLI command implementations
- `scripts/validate-serial-contract.mjs` - contract validator
- `scripts/release-matrix-smoke.mjs` - release smoke tests
