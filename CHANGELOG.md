# Changelog

All notable changes to this project will be documented in this file.

## [Unreleased]

### Added

- Root `package.json` with npm workspaces (`packages/*`, `client`, `tools`)
- Shared `@teslacanmodder/protocol` package (commands, decoder, parser, types)
- TypeScript migration for the unified client workspace
- `.editorconfig` for consistent formatting
- `CONTRIBUTING.md` with setup guide and coding standards
- `.github/workflows/ci.yml` CI/CD pipeline
- Unified client redesign with shared design system
- Component and hook tests for the client workspace
- GTW shield mode for `0x398` (`gtw-shield:arm|disarm|reset`) with snapshot-based restore/block logic
- Traffic Light / Stop Sign Control restore feature (`tlssc:on|off`) for `0x399` lower 6-bit enforcement
- HW4 Enhanced Autopilot bit handling (bit 46 on mux 1) and Emergency Vehicle Detection support (bit 59 on mux 0)
- Log ring incremental query helpers (`logRingHead()`, `logRingReadSince()`) for polling only new logs
- Native firmware test coverage for GTW shield, TLSSC restore, HW4 EAP/EVD behavior, and log ring delta reads

### Changed

- Pinned firmware dependency versions (ArduinoJson, NimBLE-Arduino, Unity)
- WiFi defaults now overridable via build flags (`#ifndef` guards)
- Client imports shared protocol package instead of local copies
- Browser support now ships from the Expo client instead of a standalone web workspace
- Flasher downloads prebuilt GitHub Release binaries published by Actions instead of calling a local firmware build server

### Removed

- Legacy standalone native app workspace
- Legacy standalone browser app workspace
- Local firmware build server Docker/service path

## [1.0.0] — 2026-04-10

### Features

- Multi-bus CAN architecture (Chassis, Vehicle, Body) with per-bus build flags
- 5 PlatformIO firmware environments (native, esp32, esp32_wifi, esp32_ble, esp32_wifi_ble)
- Client app with dashboard, vehicle controls, CAN frame monitor, flasher, setup guide, BLE, and serial connectivity
- CAN frame decoder (577 Tesla frames from mikegapinski dataset)
- Debug CLI tools
- Docker Compose for firmware build server + browser client
- 9 native test suites (149 tests) for firmware
