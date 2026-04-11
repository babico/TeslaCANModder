# Changelog

All notable changes to this project will be documented in this file.

## [Unreleased]

### Added
- Root `package.json` with npm workspaces (`packages/*`, `web`, `mobile`, `tools`)
- Shared `@teslacanmodder/protocol` package (commands, decoder, parser, types)
- TypeScript migration for web app
- Vitest test framework for web app
- `.editorconfig` for consistent formatting
- `CONTRIBUTING.md` with setup guide and coding standards
- `.env.example` files for web and hardware services
- `.github/workflows/ci.yml` CI/CD pipeline
- Mobile UI redesign with shared design system
- Component and hook tests for web and mobile

### Changed
- Pinned firmware dependency versions (ArduinoJson, NimBLE-Arduino, Unity)
- WiFi defaults now overridable via build flags (`#ifndef` guards)
- Mobile imports shared protocol package instead of local copies
- Web uses Vitest instead of raw Node.js test runner

### Removed
- Duplicate protocol code in `mobile/lib/protocol/` (moved to shared package)
- `web/src/utils/commands.js` (use shared package)

## [1.0.0] — 2026-04-10

### Features
- Multi-bus CAN architecture (FSD, Vehicle, Body) with per-bus build flags
- 7 PlatformIO firmware environments (native, uno, uno_bt, esp32, esp32_wifi, esp32_ble, esp32_wifi_ble)
- Web app with dashboard, vehicle controls, CAN frame monitor, flasher, setup guide
- Mobile app with BLE + Serial connectivity
- CAN frame decoder (577 Tesla frames from mikegapinski dataset)
- Debug CLI tools
- Docker Compose for firmware build server + web app
- 9 native test suites (149 tests) for firmware
