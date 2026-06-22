# Changelog

All notable changes to this project will be documented in this file.

## [1.5.0] — 2026-06-20

### Added

- **Selectable BLE key distance estimator** — three modes (threshold,
  formula, Kalman) wired through new `blekey:distance:<off|threshold|formula|kalman>`
  commands, `:factor:<N>` path-loss exponent, and `:calibrate:<M>` one-shot
  offset. Default TX power `-59 dBm` baseline. Status payload includes
  `bleDistanceMode`, `bleDistanceMeters`, `bleRssi`, `bleDistanceFactor`,
  `bleDistanceCalibrated`. Client `GamepadPanel` exposes all three modes
  with a live RSSI bar.
- **`nag:mode:feifan`** — eighth nag mode mirroring in-the-wild V4.1.00
  captures. `handsOnLevel` left at 0, signed torque random-walks around
  zero (~-0.05..+0.02 Nm), gated on DAS hands-on request. Designed for
  Tesla 2026.14.x preflight. Implemented via the unified nag pipeline
  (`nagEchoShouldEcho` / `nagEchoCompute` / `nagEchoApply`).
- **3 new legacy reference submodules** — `bogosj/tesla`, `mveplus/tesla-model3-resources`,
  `teslamotors/vehicle-command`. Per-repo analysis docs under
  `docs/legacy/`. `docs/legacy/README.md` and `COMPARISON.md` updated.
- **Per-repo mermaid diagrams** — 134 markdown files (53 non-legacy + 81
  legacy per-repo analyses) now have frontmatter, mermaid overview, and
  cross-links. Two non-overlapping PRs (wave 1 = 54 files, wave 2 = 73 files).
- **2026-06-20 legacy upstream review** — `docs/legacy/upstream-review-2026-06-20.md`
  walks 91 active submodules, identifies 6 with new upstream commits, and
  re-evaluates them against the shipping codebase. `## Upstream` sections
  appended to 6 affected per-repo analyses.
- **Markdown lint skill** — `.opencode/skills/markdown-lint/SKILL.md`
  formalises the CI parity checks, pre-commit verification command, and
  config layout to prevent recurring `MD001` heading-increment failures.
- **Project policy skills** — `commit-ask-first` (no surprise git
  mutations), `legacy-submodule-review` (skip list + 5-step disable/enable
  procedure), `refactor-over-duplication` (4th-copy-paste-apply anti-pattern),
  `skip-auto-generated-files` (dashboard.h, \_generated-docs.ts).
- **Serial contract reference** — new `docs/reference/serial-contract.md`
  documents the JSON-RPC wire format, response envelope, transport parity,
  and toolchain helpers.
- **Wire-format TS docblock** in `packages/protocol/src/commands.ts` —
  inline JSDoc on the wire format requirement so every consumer of the
  protocol package sees the contract.

### Changed

- **JSON-RPC wire format is now documented and required** — every command
  on USB, BLE, and WiFi is `{"cmd":"<name>"}\n`. The serial parser has
  always required this since v1.3.0; the documentation now matches the
  behaviour. Plain-text commands like `ping` are no longer advertised.
- **Tools auto-wrap plain commands** — `tools/lib/session.js` `send(cmd)`
  now wraps `cmd` in `{"cmd":"..."}` automatically when the input does
  not already start with `{`. Pass `{raw: true}` to push bytes verbatim.
  This unblocks `session.send("status")`, `session.send("ping")`, etc.
  that previously failed against current firmware.
- **Worktree version alignment** — root + workspaces bumped from 1.4.0 / 2.0.0
  mismatch to a unified `1.5.0` for the monorepo release line. Internal
  workspace versions now follow the root release tag.
- **3 dead `comm.peer` fields removed from `BoardState`** — legacy OTA
  detection, tx-paused gating, and a stale 3rd-party feature flag no
  longer shipped in `status` JSON.
- **`firmware/lib/interface/` and `lib/transport/` legacy alias tree** —
  infrastructure audit and `firmware/README.md` cross-link now reflect
  the current shipping locations (`lib/client/`, `lib/vehicle/can/`,
  `lib/vehicle/ble/`).
- **Native test count: 1000 → 1028** — all passing in `pio test -e native`.
  New suites: `test_native_ble_distance`, `test_native_nag_organic`,
  `test_native_nag_unified` (49 nag tests covering gate, apply
  byte-equivalence, feifan, command dispatch).
- **Two legacy submodules removed** — `legacy/commaai-openpilot` and
  `legacy/sunnypilot` (~852 MB freed, 91 active submodules remain).
  Disabled submodule `legacy/mikegapinski-tesla-can-explorer` (upstream 404) via `git rm --cached` to clear the CI `git submodule foreach`
  loop.

### Fixed

- **`docs/reference/can-export.md` MD001 heading-increment** — `###`
  jumped from `#`, fixed. Markdown lint CI green again.
- **`ble-adapter.md` cross-link** — repaired broken reference to a
  renamed sub-doc.
- **Unified nag pipeline refactor** — replaced 5 copy-paste `nagApply*`
  functions with `nagEchoShouldEcho` / `nagEchoCompute` / `nagEchoApply`
  single-pass pipeline. Avoids the 4th-copy-paste anti-pattern when
  adding new modes.
- **`fsd:on` / `nag:mode:feifan` round-trip on real hardware** — flashed
  and verified on ESP32-S DevKit via CP210x (COM4), boot JSON valid,
  `{"cmd":"fsd:on"}` → ack + state.fsd:1 + NVS persistence, full
  `{"cmd":"nag:mode:feifan"}` → state.nagMode:"feifan".

### Documentation

- **`docs/reference/commands.md`** — wire-format section added; required
  JSON-RPC envelope now called out at the top of the document.
- **`docs/guides/full-setup.md`, `quickstart-checklist.md`** — quickstart
  commands now use `{"cmd":"..."}` form.
- **`docs/checklists/testing-plan.md`** — BLE round-trip steps updated to
  JSON-RPC envelopes.
- **`docs/reference/ble-adapter.md`** — RX characteristic write example
  updated to JSON-RPC.
- **All `docs/legacy/*` analyses** — added Mermaid overview, frontmatter,
  and `## Upstream` notes where applicable.
- **`docs/superpowers/reports/2026-06-20-ci-markdown-lint-fix.md`** —
  postmortem of the `MD001` CI fix.

## [1.4.0] — 2026-06-15

### Added

- **3 new native test suites** — `test_native_multi_feature`,
  `test_native_drive_context`, `test_native_serial_common` covering combined
  feature dispatch, drive context initialization, and shared JSON output
  helpers
- **Cross-check filter coverage** — driver-assist and Tesla BLE command
  families now included in the cross-check integration test
- **9 new protocol wire fields** — `nagOrgBypass`, `rawCan`, and 9
  `canDiag.*` fields (tx failures, bus-off events, EW limit hits) wired
  end-to-end from firmware boot/status to client state
- **6 new commands** — driver-assist toggles (`lhd`, `assist-dev`, `assist-nav`,
  `assist-hof`, `assist-tel`, `ap-first`, `lane-graph`) plus `eap:on/off` and
  `evd:on/off` for HW4 advanced settings
- **nag unified command** — `nag:mode:<off|bit19|legacy|safe|natural|organic|full>`
  plus `nag:bypass:on/off` replaces the old `nag:on/off` knob; `nagKillerMode`
  renamed to `nagMode` in state types

### Changed

- **Protocol refactor: 62 boolean wire fields → `WireBool` type** — replaces
  ad-hoc `boolean` definitions with a named branded type, eliminating a class
  of generator bugs and clarifying the JSON contract
- **Firmware lib inlining pass** — `inline constexpr` applied to per-TU
  globals (`kBusName`, `kBusActive`, `mcpCsPins`, `mcpIntPins`, `mcpAvailable`,
  ISR function pointers) so multi-TU native tests no longer drift
- **Bit-position constants extracted** — FSD/DAS frame bit numbers lifted to
  named constants in `vehicle/can/handler/variant/bits.h`; magic numbers
  removed from HW3/HW4/legacy handlers
- **Chassis filter and CAN ID size constants** — `CHASSIS_FILTER_MAX` and
  `CAN_ID_MAX` extracted; `SERIAL_CMD` buffer split sizing fixed
- **CHASSIS lane constant** — magic bus `0` replaced with `BUS_CHASSIS` in
  all handlers; `sizeof()` used for body filter table
- **Client redesign** — uses `formatUptime` and `ApClusterState` from the
  shared protocol package; nativewind + shadcn theme with 11 component
  primitives; tab connectors removed — screens consume contexts directly
- **Native test count: 880 → 1000** — all passing in `pio test -e native`

### Fixed

- **Binary size regression** — non-const `inline` on `mcpBus`, `mcpFrameReady`,
  `mcpISRs`, and the `mcpISR_*` functions actually _increased_ size under
  the single-TU build (C++17 `inline` semantics differ for non-const data).
  Reverted to plain `static` for those four, kept `inline constexpr` for the
  const arrays
- **BLE sticky overflow flag** — `mcpISR_*` overflow latch no longer
  clobbers the frame-ready signal; counters wrapped correctly
- **TX-fail counters now atomic** — `canDiag.txFailCount` and
  `busOffCount` updated from ISR context with `std::atomic` to prevent
  torn reads on the bus-error poll path
- **9 dead `BootMessage`/`StatusMessage` fields removed** — features that
  were never wired (legacy OTA detection, tx-paused gating) are no longer
  advertised
- **10 broken doc paths fixed** — references to `docs/reference/...` and
  `docs/guides/...` that pointed at renamed or moved files
- **NVS version bumped** — persistence layer now rejects stale magic/version
  tuples from firmware < 1.4.0
- **Legacy command names corrected** in `docs/reference/commands.md` —
  `fsd:on/off`, `tlssc:on/off`, `gtwshield:arm/disarm`, etc. previously
  referenced internal codenames
- **6 missing wire fields** now emitted in `boot` and `status` payloads
  (matches `BootMessage`/`StatusMessage` schemas in protocol package)
- **brakePedalState** doc and field mapping corrected across the three
  layers (C++ State, JSON wire, TypeScript boardState)
- **7 bus assignments corrected** in `docs/reference/can-ids.md` (e.g.
  `0x370` EPAS_sysStatus was on Vehicle, now correctly on Chassis)
- **18 missing CAN IDs added** to the reference table
- **14 missing commands documented** in `docs/reference/commands.md`

### Documentation

- **Infrastructure audit completed** — orphan files, gitignore gaps, dual-tree
  firmware lib state (`lib/transport/` ↔ `lib/vehicle/can/`, `lib/interface/`
  ↔ `lib/client/`), and stale doc references catalogued
- **`firmware/README.md` Layout table updated** to reflect the current
  `lib/{core,core/can,vehicle/can/feature,vehicle/can/handler,vehicle/ble,io,client}`
    - legacy alias tree; AGENTS.md note about `lib/infra|feature|handler`
      applies only to that file
- **All `docs/guides/*.md` cross-links** walked; 10 previously broken
  `docs/...` references fixed
- **Markdown lint clean** — 41 files, 0 errors

## [1.3.0] — 2026-05-15

### Added

- **ESP32 hardware test suites** — 6 PlatformIO test environments for physical
  ESP32 DevKit validation: `test_esp32_can_driver` (MCP2515 SPI),
  `test_esp32_can_loopback` (CAN TX/RX), `test_esp32_can_recorder` (frame
  capture), `test_esp32_nvs_persist` (NVS persistence), `test_esp32_wifi_api`
  (WiFi AP/STA + HTTP), `test_esp32_led_gpio` (GPIO I/O + PWM)
- **13 new native test suites** — `test_native_id_filter`,
  `test_native_ring_buffer`, `test_native_checksum`, `test_native_burst`,
  `test_native_recorder`, `test_native_parse`, `test_native_types`,
  `test_native_ids`, `test_native_motor_temps`, `test_native_wheel_speeds`,
  `test_native_frame_readers`, `test_native_nag_math`, `test_native_ban_detect`
- **Firmware library reorganization** — new `lib/interface/` and
  `lib/transport/` directories, `lib/core/config/esp32.h`,
  `lib/core/persist/{native.h,nvs.h}`, BLE feature headers
  `lib/vehicle/ble/feature/{carserver.h,vcsec.h}`
- **Legacy reference submodules** — 7 external repos added under `legacy/` for
  research: vehicle-command, tesla-local-control, teslamotors-vehicle-command
- **Legacy analysis docs** — new reference docs under `docs/legacy/` for
  vehicle-command, tesla-local-control, and related projects

## [1.2.0] — 2026-05-05

### Added

- **DAS Drive (openpilot-style gamepad CAN injection)** — `0x2B9` DAS_control,
  `0x488` DAS_steeringControl, `0x209` APS_eacMonitor with Tesla-style 4-bit
  CRC-8 checksums, speed-aware steer cap, rate limiter, standstill brake hold,
  cancel-burst on disable, NVS-persisted enable/cap/limit
- **Gamepad (BLE HID)** — NimBLE central scanner/pairing, 16-button bindings
  (tap + hold), 6 analog axes feeding DAS Drive sticks/triggers, per-axis
  deadzone/expo/inversion tuning, NVS round-trip
- New gamepad command family: `gamepad:scan|pair|unpair|on|off|status|cancel`,
  `gamepad:bind:<n>:<cmd>`, `gamepad:hold:<n>:<cmd>`,
  `gamepad:axis:<n>:<dz|expo|inv>:<v>`
- Client `DasDrivePanel` and `GamepadPanel` in Controls screen with live status
  (ARMED/CONNECTED/RSSI/battery), discovered-device pairing list, speed/cap
  inputs bounded by safety envelope
- IndexedDB persistence for monitor diagnostics + live CAN frames with
  localStorage fallback (`monitorDiagnosticsPersistence`,
  `monitorLiveCanPersistence`, `indexedDbStore`)
- Native test suites: `test_native_das_drive` (22 tests — frame builders,
  safety envelope, NVS), `test_native_gamepad_axis` (18 tests — deadzone,
  expo curve, inversion, trigger range)
- Protocol command tests: `gamepad-commands.test.ts` (20 tests covering MAC
  validation, button/axis range checks, axis-kind enum)
- Cross-check coverage extended for parameterized command families
  (`prefix:<...>:<...>` doc-prefix matcher)
- Client tests: `monitorDiagnosticsPersistence` (6) +
  `monitorLiveCanPersistence` (5) + `ControlsScreen` Gamepad panel (5)
- Docs: `docs/guides/das-drive.md`, expanded `vehicle-features.md`
  (DAS Drive + Gamepad sections), Gamepad section in `commands.md`,
  `openpilot-tesla-harness-topology.md` reference
- Native test infrastructure: `Preferences.h` shim, `fake_preferences` extended
  with `bool`/`uint8`/`uint16` getters/setters
- Tools: `tools/commands/das-drive.js` debug helper

### Changed

- Refactored gamepad axis math into pure header `client/gamepad/axis_math.h`
  for native testability without NimBLE dependencies
- Promoted client/io serial + WiFi logic to top-level `lib/`; renamed
  `io/wifi/web/esp32` → `io/wifi/esp32`; extracted `wifi_api.h` and
  `client/common/api_fwd.h` to decouple includes
- Variant gating split into `lib/vehicle/can/handler/variant/{hw3,hw4,legacy}.h`
  with shared `helpers.h`/`filters.h`/`ticks.h`
- DAS Drive section in `commands.md` now references the chassis CAN bus
  consistently (legacy "autopilot party CAN" wording removed)

### Removed

- `ap_party` legacy build paths and references (fully cleared)
- `BUS_DAS_OUT` build flag (DAS Drive now writes on the chassis bus)
- `apmode` legacy AP toggle (replaced by WiFi soft-AP managed via `wifi:*`)
- Stale `ble_config.h`, `wifi_api.h`, `hw3.h`/`hw4.h`/`legacy.h` shims under
  `lib/io/` and `lib/vehicle/can/handler/` after refactor

### Fixed

- Cross-check doc-prefix matcher now walks all ancestor prefixes so
  `prefix:<a>:<b>` style commands resolve correctly
- Stick saturation expectation in axis math (`raw=255` → `v=127`, asymmetric
  with `raw=0` → `v=-128`) documented and tested

## [1.1.0] — 2026-05-02

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
