---
name: protocol-package
description: Shared TypeScript ESM package for TeslaCANModder command definitions, CAN decoder data, and serial contract validation
license: GPL-3.0
compatibility: opencode
metadata:
    area: protocol
    stack: typescript
---

## What I do

Guide agents through the `@teslacanmodder/protocol` shared TypeScript package in `packages/protocol/`. I cover:

- Command definitions that are the source of truth for firmware and client
- CAN decoder data (IDs, signals, frame structures)
- Serial contract validation between firmware and TypeScript layers
- TypeScript strict mode and ESM module practices

## When to use me

Use this skill when:

- Modifying shared types, commands, or decoder data in `packages/protocol/`
- Adding new CAN ID definitions or signal mappings
- Changing protocol parsing helpers
- Updating the serial contract between firmware and client

## Project structure

```
packages/protocol/
├── src/
│   ├── commands/       command definitions and validation
│   ├── types/          shared TypeScript types
│   ├── decoder/        CAN frame decoder data and helpers
│   └── index.ts        package exports
├── test/               Jest ESM tests
├── package.json        package manifest ("type": "module")
└── tsconfig.json       TypeScript configuration
```

## Tech stack

- TypeScript (strict, ESM)
- Jest with ESM support
- Node.js >= 18

## Coding standards

- Strict TypeScript where available.
- ESM modules (`"type": "module"` in package.json).
- `camelCase` for variables and functions.
- `PascalCase` for interfaces and types.
- ESLint rules from root config apply.
- Prettier: tabs, tab width 4, print width 100, semicolons, double quotes, trailing commas everywhere.

## Serial contract validation

The serial contract between firmware and protocol package must remain in sync.

Validate the contract:

```bash
npm run validate:serial-contract
```

This checks that:

- Command IDs match between firmware and protocol
- Frame formats are consistent
- Type definitions align with firmware behavior

## Commands and types

Command definitions are the source of truth for:

- Firmware command dispatch
- Client command generation
- Tools CLI command parsing

When adding a new command:

1. Add the command definition in `packages/protocol/src/commands/`
2. Export it from `packages/protocol/src/index.ts`
3. Update the serial contract if needed
4. Add firmware handler in `firmware/lib/vehicle/can/feature/` or `firmware/lib/client/command/`
5. Add client UI in appropriate screen/component
6. Run `npm run validate:serial-contract`

## CAN decoder data

Decoder data lives in `packages/protocol/src/decoder/`. This includes:

- CAN ID mappings
- Signal bit positions and scaling factors
- Frame structure definitions

When modifying decoder data:

- Cross-reference with `docs/reference/can-ids.md`
- Cross-reference with `docs/reference/signal-matrix.md`
- Ensure consistency with firmware frame handlers

## Testing

Run protocol tests:

```bash
npm run test:protocol
```

Tests use Jest with native ESM support. Always add tests for:

- New command definitions
- New type constraints
- New decoder helpers
- Parsing edge cases

## Building

Build the protocol package:

```bash
npm run build:protocol
```

The client and tools depend on the built protocol package. Build protocol before testing client or tools.

## Type checking

```bash
npm run typecheck:protocol
```

## Before finishing

1. Run `npm run build:protocol` successfully.
2. Run `npm run test:protocol` and ensure all tests pass.
3. Run `npm run validate:serial-contract` and fix any mismatches.
4. Run `npm run lint:all` from repo root.
5. If client or tools were affected, run their tests too.
6. Clean up any temporary files.

## Key files to reference

- `packages/protocol/src/commands/` - command definitions
- `packages/protocol/src/types/` - shared types
- `packages/protocol/src/decoder/` - decoder data
- `docs/reference/commands.md` - command reference
- `docs/reference/can-ids.md` - CAN ID reference
- `docs/reference/state-fields.md` - state field documentation
- `scripts/validate-serial-contract.mjs` - contract validator
