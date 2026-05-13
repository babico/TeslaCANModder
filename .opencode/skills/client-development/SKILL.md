---
name: client-development
description: Expo React Native client app development for TeslaCANModder including UI components, state management, hardware connections, and in-app documentation
license: GPL-3.0
compatibility: opencode
metadata:
    area: client
    stack: typescript, react-native
---

## What I do

Guide agents through the TeslaCANModder Expo React Native client app in `client/`. I cover:

- Component and screen development with React Native
- State management patterns (hooks, command bus, frame ingestion)
- Hardware connections (Web Serial, Bluetooth, BLE, WiFi)
- In-app documentation rendering from raw markdown
- Testing with Jest + React Native Testing Library

## When to use me

Use this skill when:

- Modifying React Native / Expo code in `client/`
- Adding new UI screens, components, or features
- Working with state management, hardware connections, or frame ingestion
- Changing the browser flasher or in-app documentation rendering
- Adding or modifying client tests

## Project structure

```
client/
├── src/
│   ├── components/     reusable UI components
│   ├── screens/        app screens
│   ├── state/          state management, hooks, stores
│   ├── hardware/       hardware connection abstractions
│   ├── ui/             base UI primitives (Button, Card, Input, etc.)
│   ├── types/          TypeScript type definitions
│   ├── styles/         global CSS/styles
│   └── test/           test mocks and utilities
├── tests/              Jest + Testing Library tests
├── package.json        client package manifest
└── tsconfig.json       TypeScript configuration
```

## Tech stack

- React Native 0.81.5 (via Expo)
- React 19.1.0
- TypeScript (strict)
- Jest + React Native Testing Library
- Web Serial API for browser flasher
- Tailwind CSS / utility classes for styling

## Coding standards

- Strict TypeScript where available.
- Functional React components with hooks. No class components.
- `camelCase` for JS/TS variables and functions.
- `PascalCase` for React components and TS interfaces.
- ESLint rules:
    - `eqeqeq` strict - always `===` / `!==`.
    - `no-console` - only `console.warn` and `console.error` are allowed.
    - `@typescript-eslint/no-explicit-any` warns; tests are exempt.
    - Unused vars: prefix with `_` to silence.
    - `no-eval` and `no-implied-eval` are errors.
- Prettier settings: tabs, tab width 4, print width 100, semicolons, double quotes, trailing commas everywhere, LF line endings, always-parenthesized arrow params.

## State management patterns

The client uses a combination of:

- Custom hooks in `client/src/state/` for local/component state
- Command bus pattern for device communication
- IndexedDB for persistent local storage
- Frame ingestion pipeline for CAN data processing

Key state files:

- `client/src/state/commandBus.ts` - command dispatch
- `client/src/state/frameIngestion.ts` - CAN frame processing
- `client/src/state/monitorConnectionPresets.ts` - connection management
- `client/src/state/monitorDiagnostics.ts` - diagnostics tracking

## Hardware connection

The client supports multiple connection methods:

- **Web Serial API** (browser) - USB serial connection
- **Bluetooth Serial** (mobile) - Bluetooth classic serial
- **BLE** (mobile) - Bluetooth Low Energy via NimBLE
- **WiFi** (all platforms) - REST API connection to device AP

## In-app documentation

The documentation screen renders raw markdown from `docs/` directly. There is no separate generated docs bundle. When adding new docs:

- Place markdown files in appropriate `docs/` subdirectory
- Use frontmatter for metadata (title, description, category, etc.)
- The client will automatically discover and render them

## Testing

Run client tests:

```bash
npm run build:protocol && npm run test:client
```

Tests use Jest + React Native Testing Library. Mock files are in `client/src/test/mocks/`.

Add tests for:

- New components (rendering, interactions)
- New state hooks (behavior, edge cases)
- Command dispatch changes
- Frame parsing changes

## Development server

Do NOT start `expo start`, `npm run web`, or other long-running watchers from automated tool runs. Suggest them as manual commands for the user:

```bash
npm run web -w @teslacanmodder/client
# or
cd client && npm start
```

## Type checking

```bash
npm run typecheck:client
```

## Before finishing

1. Run `npm run typecheck:client` and fix any errors.
2. Run `npm run test:client` and ensure all tests pass.
3. Run `npm run lint:all` from repo root.
4. Verify the browser build works if UI changes were made.
5. Clean up any temporary files.

## Key files to reference

- `client/src/AppExperience.tsx` - main app entry
- `client/src/state/` - all state management
- `client/src/ui/` - base UI components
- `client/src/hardware/` - connection abstractions
- `docs/architecture/unified-client-guide.md` - client architecture
- `docs/architecture/monitor-architecture.md` - monitor system
- `docs/architecture/layout-system.md` - layout system
