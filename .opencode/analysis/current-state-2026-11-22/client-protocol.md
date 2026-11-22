# P0-00 Current-State Extraction — Client + Protocol

Date: 2026-11-22. Subagent task: read-only current-state extraction for the `client/` + `packages/protocol/` subsystem.

## Scope, method, evidence grades

Method: `git ls-files` for authoritative file lists; `read` for line-numbered source; `grep`/`Select-String` for cross-file symbol and literal searches; `git diff --no-index` for the two `AppView` files. No test runner, build, server, or runtime was invoked.

Evidence grades used below:

- **CODE-READ** — file body read with line numbers and cited as `file:line`.
- **TEST-READ** — test file bodies read (three files); test-case counts from shell `Select-String` are **CODE-READ (grep-counted)** and approximate.
- **CAPTURE** — runtime/wire capture. None in this report.
- **SUMMARY-ONLY** — claim taken from `docs/superpowers/todo.md` (lines 31-41, 94-113, 162-172) and re-graded against current code; the todo itself is **not** trusted as current truth.
- **UNVERIFIED** — not read or not confirmed; listed explicitly at the end.

Counts confirmed from `git ls-files`: `packages/protocol/src` = 11 files, `client/src` = 102 files, `client/tests` = 45 files, `packages/protocol/test` = 27 files. The four `client/src/assets/can-decoder/*.json` blobs are 548k-759k lines each and are generated data, not code.

## 1. `packages/protocol/**` — what actually exists now

### Inventory (all CODE-READ unless noted)

| File                                          | Lines | Role (evidence)                                                                                                                                                                                                                                                                           |
| --------------------------------------------- | ----- | ----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| `packages/protocol/src/commands.ts`           | 584   | Command registry object `commands` at `commands.ts:108-540`; standalone builders and aliases at `commands.ts:558-674`; validators `commands.ts:68-106`; numeric ranges `commands.ts:56-66`.                                                                                               |
| `packages/protocol/src/gating.ts`             | 951   | `COMMAND_GATES` table `gating.ts:33-980`; `getCommandGate` at `gating.ts:989-1006` returns `{available, reason}`; gate shape `gating.ts:7-26`; `CommandName = keyof typeof commands` at `gating.ts:17`.                                                                                   |
| `packages/protocol/src/featureSettings.ts`    | 1190  | Spec types `featureSettings.ts:5-64`; internal spec map starts `featureSettings.ts:66`; `nagControl` spec `featureSettings.ts:546-573`; `speedOffset` spec `featureSettings.ts:574-589`. Most of the 1190 lines are spec data (not fully read — UNVERIFIED).                              |
| `packages/protocol/src/types.ts`              | 969   | `BoardFeatures` at `types.ts:5-16` (optional `eap`/`evd`/`tlssc` at `types.ts:13-15`); status payload sub-types `types.ts:24-94`; `BootMessage` union member `types.ts:96-130`. Rest of 969 lines not fully read (UNVERIFIED).                                                            |
| `packages/protocol/src/parser.ts`             | 328   | JSON-line parser; normalization helpers `parser.ts:28-94`, state-section field whitelist `parser.ts:96-120`; exports `parseSerialLine`/`parseSerialChunk` via `index.ts:167`.                                                                                                             |
| `packages/protocol/src/reducer.ts`            | 1275  | `BUS_NAMES` `reducer.ts:25`; `detectBoard` `reducer.ts:29`; `initialBoardState` `reducer.ts:106`; state flags `reducer.ts:164-165`; `busName` fallback `reducer.ts:903`; `reduceBoardMessage` `reducer.ts:1235`.                                                                          |
| `packages/protocol/src/selectors.ts`          | 158   | `selectChargeSnapshot` fully-charged branch `selectors.ts:96-104`.                                                                                                                                                                                                                        |
| `packages/protocol/src/decoder.ts`            | 90    | `DecoderDataset`/`DecoderIndex` types `decoder.ts:20-32`; `buildDecoderIndex` `decoder.ts:44-52`; `KNOWN_CAN_IDS` `decoder.ts:72-94`.                                                                                                                                                     |
| `packages/protocol/src/format.ts`             | 130   | Pure display formatters, e.g. `formatAutopilotTier` `format.ts:3-18`, `formatDriveMode` `format.ts:51-60`.                                                                                                                                                                                |
| `packages/protocol/src/transportLifecycle.ts` | 94    | Transport kinds `transportLifecycle.ts:1`; `resolveTransportSelection` `transportLifecycle.ts:38-66`; `reduceTransportLifecycle` `transportLifecycle.ts:68-108`.                                                                                                                          |
| `packages/protocol/src/index.ts`              | 157   | Barrel re-exports types `index.ts:4-33`, commands `index.ts:36-76`, specs `index.ts:79-92`, gating `index.ts:95-96`, decoder `index.ts:99-111`, reducer `index.ts:114-120`, format `index.ts:123-134`, selectors `index.ts:137-150`, lifecycle `index.ts:153-164`, parser `index.ts:167`. |

The package is a real shared library: client code imports from `@teslacanmodder/protocol` (e.g. `controller.ts:1`, `board.ts:1-9`), with one deliberate exception at `ControlsScreen.tsx:9` (see P2-008).

### Protocol facts relevant to the rewrite

- Wire contract documented in-code at `commands.ts:3-13`: single-line `{"cmd":"..."}\n` JSON-RPC; plain text is rejected by firmware. Protocol builders emit bare command _names_, not envelopes; the client transport wraps them (see `controller.ts:192-199` and `transport.ts`).
- Gating is capability/state driven: chassis-gated `fsd` `gating.ts:38-43`, body-gated mirror commands `gating.ts:292-331`, feature-gated `profile` `gating.ts:60-69`, hardware-gated `bms` `gating.ts:224-229`.
- Feature-setting specs are typed by `CommandBuilderName` (`featureSettings.ts:3`), so spec-to-builder drift is caught at typecheck time — but spec-to-_firmware_ drift is not (see section 5).

## 2. `client/src/**` — real component boundaries and god components

### Provider / data-flow boundary (CODE-READ)

Mount tree: `AppView.tsx:8-10` and `AppView.web.tsx:8-10` wrap `AppExperience` in `BoardConnectionProvider`; `BoardConnectionContext.tsx:78-90` composes `TransportProvider` → `BoardStateProvider` → `CommandProviderComposer` → `CommandProvider`. `CommandProviderComposer` at `BoardConnectionContext.tsx:93-115` supplies `boardState`, transport type, `applyBoardPayload` and `setLastResult` to `CommandProvider`. The compat hook `useBoardConnection` at `BoardConnectionContext.tsx:119-173` flattens all four contexts.

Concrete boundaries that exist today:

- **Transport** — `hardware/transport.ts:103-115` (`HttpCommandTransport`), plus BLE/serial/Bluetooth-SPP implementations and factory exports imported at `controller.ts:8-26`; transport presentation/state at `hardware/transportPresentation.ts` and `hardware/monitorPollingPolicy.ts` (line counts 105 and 61).
- **Controller** — `hardware/controller.ts:93-260`: `CommandName = keyof typeof commands` (`controller.ts:29`), `ALL_COMMANDS` derived from registry (`controller.ts:36-41`), `runCommand` builds/serializes via registry (`controller.ts:186-219`), `readStatus` (`controller.ts:252-259`).
- **State** — `BoardStateContext.tsx:61-136`, `CommandContext.tsx:85-193`, `TransportContext.tsx:94+` (partial read), plus pure helpers `board.ts`, `commandBus.ts`, `frameIngestion.ts`, `monitorCommandExecution.ts`, `monitor*.ts`.
- **Screens** — `screens/ConsoleScreen.tsx`, `ControlsScreen.tsx`, `DashboardScreen.tsx`, `DocsScreen.tsx`, `DriveScreen.tsx`, `FlasherScreen.tsx`. Console is a thin 66-line shell delegating to `MonitorSection`/`ConnectionSection`/`DiagnosticsSection` (`ConsoleScreen.tsx:25-57`).
- **UI** — small shadcn-style primitives under `ui/shadcn/` (5-73 lines each, from line-count scan) and design tokens `design/tokens.ts` (463 lines).
- **Docs** — `docs/catalog.ts:28-43` builds routes/tree from generated bundle; `docs/_generated-docs.ts:1-3` is AUTO-GENERATED (`scripts/generate-docs-catalog.mjs`), 11754 lines; renderer `components/docs/MarkdownRenderer.tsx:62-93`; tree item `DocTreeItem.tsx` (50 lines).
- **Flasher** — `hardware/webEspFlasher.ts:38-103` (`supportsBrowserEspFlash`, `flashMergedEspReleaseImage` via Web Serial + `esptool-js`); `screens/FlasherScreen.tsx:113-128` owns local flash connection/bus/clock/log state.

### God components (line counts from `git ls-files` + `Measure-Object -Line`, CODE-READ)

| File                                                           | Lines | Note                                                                                                     |
| -------------------------------------------------------------- | ----- | -------------------------------------------------------------------------------------------------------- |
| `client/src/components/controls/ControlsBlocks.tsx`            | 1189  | ~half is StyleSheet (PA-011); preset theme helper at `ControlsBlocks.tsx:55-67`.                         |
| `client/src/AppExperience.tsx`                                 | 1148  | Local state duplication at `AppExperience.tsx:125-152`; own `runCommand` at `AppExperience.tsx:385-444`. |
| `client/src/screens/DriveScreen.tsx`                           | 855   | Only 1 test case (`client/tests/screens/DriveScreen.test.tsx`).                                          |
| `client/src/components/monitor/sections/ConnectionSection.tsx` | 608   | Largest stateful UI block; runner command default at `ConnectionSection.tsx:22-24`.                      |
| `client/src/design/tokens.ts`                                  | 463   | Token dictionary; not a component.                                                                       |
| `client/src/screens/FlasherScreen.tsx`                         | 439   | Flash UI + local state.                                                                                  |
| `client/src/components/monitor/sections/MonitorSection.tsx`    | 431   | CAN frame view.                                                                                          |
| `client/src/hardware/transport.ts`                             | 379   | Multiple transport classes.                                                                              |
| `client/src/hardware/runtimeAdapters.ts`                       | 313   | Platform adapters.                                                                                       |
| `client/src/screens/ControlsScreen.tsx`                        | 324   | Screen + gating + palette wiring.                                                                        |
| `client/src/screens/DocsScreen.tsx`                            | 297   | Docs browser.                                                                                            |

The genuinely clean boundaries are the provider tree, the transport/controller layer, the pure `state/monitor*` helpers, the shadcn primitives, and the docs catalog. The remaining complexity is concentrated in `AppExperience` (two parallel state paths) and the three large UI files.

## 3. Tests — what is covered and what is not

Counts are grep-counted `it(`/`test(` occurrences (CODE-READ via shell), approximate:

- `client/tests/**`: 45 files, ≈302 test cases. Heaviest: `hardware/transport.test.ts` (29), `state/monitorSessionHistory.test.ts` (29), `state/monitorConnectionPresets.test.ts` (28), `state/monitorCommandExecution.test.ts` (24), `state/monitorExportComposition.test.ts` (23), `state/commandBus.test.ts` (21), `state/frameIngestion.test.ts` (19), `screens/ControlsScreen.test.tsx` (10).
- `packages/protocol/test/**`: 27 files, ≈455 test cases. Heaviest: `commands/commands.test.ts` (106), `commands/new-commands.test.ts` (61), `reducer/variant-features.test.ts` (32), `state`-like reducer tests (5-12 each).

Bodies read (TEST-READ):

- `client/tests/state/CommandContext.test.tsx:30-67` — only two smoke tests: initial history empty, `runCommand` is a function. It does **not** assert lifecycle population, so P2-001 is untested.
- `packages/protocol/test/gating/command-gating.test.ts:4-66` — covers no-gate, chassis, feature, hardware, and multi-rule ordering. The `canSimStart` case at `command-gating.test.ts:57-66` fails on the chassis gate first, so the self-referential `canSim` gate is never exercised; `seatbelt`/`wiperPersist`/`singleShot` gates have no test.
- `packages/protocol/test/integration/cross-check.test.ts:1-716` — the most important structural gap: its `FIRMWARE_WIRE_COMMANDS` array at `cross-check.test.ts:35-361` is a **hardcoded hand-maintained list**, not parsed from firmware. Tests at `cross-check.test.ts:655-715` therefore verify docs ↔ protocol ↔ a literal list, and never open a firmware file. Line `cross-check.test.ts:708` explicitly whitelists `apfirst`/`lanegraph` prefixes, suppressing the real mismatch (section 5).

Files with no matching test file (test-file-list comparison, CODE-READ): `AppView.tsx`, all six `components/controls/*Panel.tsx` except `ControlsBlocks`, `components/monitor/sections/*` (3), `components/transport/*` (3), `ThemeSwitch.tsx`, `hardware/runtimeAdapters.ts`, `hardware/webEspFlasher.ts`, hooks `useBleConfig`/`useDecoderDatasets`/`useDiagnostics`, `state/board.ts`, `state/commandGating.ts`, `state/indexedDbStore.ts`, `state/resolveChargeState.ts`, and all `state/use*` presentation hooks. Note `board.ts` holds `normalizeFeatures` and the unchecked cast (P2-009/P6-005) with no direct test.

## 4. Old todo re-check (P2, P6, PA)

Statuses from current code. `SUMMARY-ONLY` denotes the original claim; evidence is current CODE-READ.

### P2 — client/protocol critical (11 items)

| ID     | Status        | Current evidence                                                                                                                                                                          |
| ------ | ------------- | ----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| P2-001 | STILL_PRESENT | `CommandContext.tsx:100` still `const [busState] = useReducer(...)`; `commandLifecycle` from `busState.entries` at `:101`; `runCommand` at `:132-180` never dispatches lifecycle actions. |
| P2-002 | STILL_PRESENT | `gating.ts:762-767` `seatbelt` gate checks `s.seatbeltEmulation`.                                                                                                                         |
| P2-003 | STILL_PRESENT | `gating.ts:778-783` `wiperPersist` gate checks `s.wiperPersist`.                                                                                                                          |
| P2-004 | STILL_PRESENT | `gating.ts:816-821` `singleShot` gate checks `s.singleShot`.                                                                                                                              |
| P2-005 | STILL_PRESENT | `gating.ts:794-813` `canSimStart`/`canSimStop` check `s.canSim`.                                                                                                                          |
| P2-006 | STILL_PRESENT | `featureSettings.ts:556-564` lists 7 nag modes, no `feifan`; `FsdPanel.tsx:13-22` does include `feifan`, so the two UIs disagree.                                                         |
| P2-007 | STILL_PRESENT | `ControlsBlocks.tsx:55-67` assigns `_base = PRESET_THEME[preset]` then ignores it.                                                                                                        |
| P2-008 | STILL_PRESENT | `ControlsScreen.tsx:9` imports `../../../packages/protocol/src/featureSettings`.                                                                                                          |
| P2-009 | STILL_PRESENT | `board.ts:145-146` still `raw as unknown as BoardMessage`.                                                                                                                                |
| P2-010 | STILL_PRESENT | `AppExperience.tsx` is 1148 lines (todo said 1238); local `boardState`/`history`/`pollIndex` at `:134-144` duplicate provider state.                                                      |
| P2-011 | STILL_PRESENT | `VehicleCommandsPanel.tsx:80` and `:87` dead ternaries `cond ? "trackMode" : "trackMode"` / `cond ? "precondition" : "precondition"`.                                                     |

### P6 — client/protocol medium (20 items)

| ID     | Status        | Current evidence                                                                                                                                                                                |
| ------ | ------------- | ----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| P6-001 | STILL_PRESENT | `MarkdownRenderer.tsx:73-80` production `console.warn` + `no-explicit-any` disable.                                                                                                             |
| P6-002 | FIXED         | `AppExperience.tsx:581` now reads `POLL_SECONDS[pollIndex]` inside the effect at `:573-613`. (`readStatus` still absent from deps — different concern, not the reported bug.)                   |
| P6-003 | STILL_PRESENT | `FsdPanel.tsx:25`, `DasPanel.tsx:15`, `GamepadPanel.tsx:19`, `VehicleCommandsPanel.tsx:133`, `TelemetryPanel.tsx:114-119`, plus `ConnectionSection.tsx:473` in old claim (last one UNVERIFIED). |
| P6-004 | FIXED         | `DasPanel.tsx:70-78` uses `value={limitInput}` / `onChangeText={setLimitInput}`; live value now only `placeholder`/label (`:76`, `:80`), so typing is not overwritten.                          |
| P6-005 | STILL_PRESENT | `board.ts:94-103` copies only fsd/fsdForce/profile/nag/offset/isaSpeedChime/summon; `types.ts:13-15` declares `eap`/`evd`/`tlssc`.                                                              |
| P6-006 | STILL_PRESENT | `git diff --no-index`: `AppView.tsx` and `AppView.web.tsx` differ only by export function name (`AppView` vs `AppViewWeb`), body still duplicated.                                              |
| P6-007 | STILL_PRESENT | `DashboardScreen.tsx:102-123` (mobile) vs `:166-187` (desktop) duplicate overview cards.                                                                                                        |
| P6-008 | STILL_PRESENT | `useDiagnostics.ts:91-101` persists on every history/snapshot/archive change, no debounce.                                                                                                      |
| P6-009 | FIXED         | `AppExperience.tsx:418-421` now loops `executionResult.dispatchActions` into `busDispatch`. Caveat: the provider path from P2-001 still drops them.                                             |
| P6-010 | STILL_PRESENT | `controller.ts:95` single `inflight` pointer; `:198` overwrites on concurrent commands.                                                                                                         |
| P6-011 | STILL_PRESENT | `commandBus.ts:166-168` still tests `e.finishedAt !== null` (now alongside `!== undefined`); type is `finishedAt?: number` at `:27`.                                                            |
| P6-012 | STILL_PRESENT | `selectors.ts:96` and `resolveChargeState.ts:54` both use `chargeKw < 0.5`.                                                                                                                     |
| P6-013 | STILL_PRESENT | `ConsoleScreen.tsx:33` `Object.keys(props.boardState.canHealth ?? {}).length`.                                                                                                                  |
| P6-014 | STILL_PRESENT | `TelemetryPanel.tsx:114-119` per-render `as unknown as Record<string, number>` lookups.                                                                                                         |
| P6-015 | STILL_PRESENT | `ConnectionSection.tsx:22-24` seeds from `props.availableCommands[0] ?? "status"` (different but still fragile default).                                                                        |
| P6-016 | FIXED         | `DashboardScreen.tsx:8` imports only `useBreakpoint`; used at `:21`. No direct `useWindowDimensions` read.                                                                                      |
| P6-017 | STILL_PRESENT | `CommandContext.tsx:60-71` hooks throw without logging.                                                                                                                                         |
| P6-018 | STILL_PRESENT | `DocsScreen.tsx:228-231` useMemo depends on `docTree.children`, not `docTree`.                                                                                                                  |
| P6-019 | STILL_PRESENT | `transport.ts:90-92` `normalizeBaseUrl` only strips trailing slash; constructor `:110-115` does no `new URL()` validation.                                                                      |
| P6-020 | STILL_PRESENT | `controller.ts:105-114` `setTransport` swallows rejection with empty `catch` at `:109-111`.                                                                                                     |

### PA — client/protocol low (12 items)

| ID     | Status        | Current evidence                                                                                                                                                                             |
| ------ | ------------- | -------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- | ------------------------------------------------------------------------------------------------------------------------------- |
| PA-001 | STALE         | `commands.ts:666-668` still emits `pedal:std`, but firmware literal scan finds both `pedal:std` and `pedal:standard`, so both are accepted; `commands.md` documents both. No defect.         |
| PA-002 | STALE         | `commands.ts:669-671` emits `regen:standard`; firmware literal scan finds both `regen:standard` and `regen:std`. No defect.                                                                  |
| PA-003 | STILL_PRESENT | `CommandContext.tsx:5` header still says "Extracted from BoardConnectionContext (Phase B refactor)"; `BoardConnectionContext.tsx:78-90` still exists as compat layer.                        |
| PA-004 | STALE         | `CommandContext.tsx:137` `commandBlockReason ?? undefined`; source is `string                                                                                                                | null`at`monitorPollingPolicy.ts:16`, and the target field is optional (`monitorCommandExecution.ts:57`) — intentional coercion. |
| PA-005 | STILL_PRESENT | `monitorDiagnostics.ts:79-87` `ts: nowMs - idx` synthetic timestamps.                                                                                                                        |
| PA-006 | FIXED         | `monitorExport.ts:62-94` has a single `buildExportProvenance`; the 6× repetition is gone. (Composition helper `monitorExportComposition.ts` exists, 165 lines, not fully read — UNVERIFIED.) |
| PA-007 | STILL_PRESENT | `ControlsBlocks.tsx:9` includes `"service"`; theme defined at `:43-52` but `getPresetTheme` at `:55-67` ignores it.                                                                          |
| PA-008 | STALE         | `board.ts:11-13` now has `!Array.isArray(value)`; `Boolean("")` is already `false`, so the claimed empty-string leak does not occur.                                                         |
| PA-009 | STILL_PRESENT | `board.ts:43` and `reducer.ts:903` both fall back to `Bus${bus}` beyond 0..2.                                                                                                                |
| PA-010 | STILL_PRESENT | `ConnectionSection.tsx` still 608 lines.                                                                                                                                                     |
| PA-011 | STILL_PRESENT | `ControlsBlocks.tsx` still 1189 lines (todo said 1221).                                                                                                                                      |
| PA-012 | STILL_PRESENT | `controller.ts:36-41` `ALL_COMMANDS` sorted with `localeCompare`.                                                                                                                            |

Totals: **34 STILL_PRESENT, 5 FIXED, 4 STALE** across 43 re-checked items (P2 11 + P6 20 + PA 12).

## 5. Advertised vs implemented command surface

Method (all CODE-READ): extracted every `strcmp/strncmp(cmd, "...")` literal from all firmware `.h`/`.cpp` files under `firmware/lib` via `git ls-files firmware/lib` + `Select-String`; then compared with the builders in `commands.ts:108-540`, the `FIRMWARE_WIRE_COMMANDS` list in `cross-check.test.ts:35-361`, and `docs/reference/commands.md`. Firmware dispatch entry point is `firmware/lib/client/command/dispatch.h:25` (`executeCommand`), ending in `sendError(F("Unknown command"))` at `dispatch.h:1085`. Only `firmware/lib/client/command/` was read in depth; other feature handlers were searched by literal, not fully read (UNVERIFIED for their internal behavior).

### Advertised in `commands.ts` / docs with NO firmware handler

Each was confirmed absent by an all-`firmware/lib` literal scan (no match):

| Builder(s)                                   | Wire string(s)                      | Evidence                                                                              |
| -------------------------------------------- | ----------------------------------- | ------------------------------------------------------------------------------------- |
| `apFirstToggle`                              | `apfirst:on` / `apfirst:off`        | `commands.ts:191`; firmware has `ap-first:on`/`ap-first:off` at `dispatch.h:619`.     |
| `laneGraph`                                  | `lanegraph:on` / `lanegraph:off`    | `commands.ts:192`; firmware has `lane-graph:on`/`lane-graph:off` at `dispatch.h:579`. |
| `btnMap`, `btnMapQuery`, `btnMapReset`       | `btnmap:*`                          | `commands.ts:408-415`; no `btnmap:` in firmware.                                      |
| `speedAlert`                                 | `speedalert:on`/`off`               | `commands.ts:418`; no match.                                                          |
| `gvret`, `gvretPort`                         | `gvret:*`                           | `commands.ts:421-426`; no match.                                                      |
| `espNow`, `espNowChannel`                    | `espnow:*`                          | `commands.ts:429-434`; no match.                                                      |
| `scanMyTesla`                                | `smt:on`/`off`                      | `commands.ts:437`; no match.                                                          |
| `elm327`                                     | `elm327:on`/`off`                   | `commands.ts:440`; no match.                                                          |
| `teslaBle`, `teslaBleAuth`, `teslaBleForget` | `teslable:*`                        | `commands.ts:443-445`; no match.                                                      |
| `homeAssistant`, `haDiscovery`, `haInterval` | `ha:*`                              | `commands.ts:457-463`; no match.                                                      |
| `bleEncrypt`, `blePair`, `bleUnpair`         | `bleencrypt:*`                      | `commands.ts:466-468`; no match.                                                      |
| `dasArm`, `dasDisarm`, `dasStatus`           | `das:arm`/`das:disarm`/`das:status` | `commands.ts:522-524`; no `das:` literal in firmware.                                 |

These are also present verbatim in the "canonical wire commands" test list: `cross-check.test.ts:220-223`, `:236-238`, `:269-298`, `:305-336`. `docs/reference/commands.md` documents them all. Result: roughly **29 advertised command strings across 12 feature groups return `{"t":"error","msg":"Unknown command"}`** from the shipping dispatcher.

Confirmed backed (spot checks): `gamepad:` `dispatch.h:983`, `tesla:` `dispatch.h:1078-1082`, `blekey:distance` `dispatch.h:963`, `recorder:status` comment `dispatch.h:871`, `ble:status` comment `dispatch.h:910`, `can:raw:` `can_raw.h:14`, `canclock:auto` `can_clock.h:13`, `nag:mode:feifan` via `parseNagMode` (`firmware/lib/core/types.h:291`) with test `firmware/test/test_native_nag_organic/test_nag_organic.cpp:579-583`.

Why tests do not catch it: `cross-check.test.ts:30-33` labels its list the "single source of truth" but never reads firmware; `cross-check.test.ts:708` whitelists `apfirst`/`lanegraph` explicitly. Any firmware rename or removal is invisible to the suite.

## 6. Web UI rewrite assessment

### Keep (reuse as-is or with small fixes)

| Area                                           | Evidence                                                                                                                | Effort               | Risk   |
| ---------------------------------------------- | ----------------------------------------------------------------------------------------------------------------------- | -------------------- | ------ |
| `packages/protocol/**` (11 files, ~5.6k lines) | barrel `index.ts:1-167`; types/gating/parsers/reducers are pure and well tested (section 3)                             | 0-2 h                | LOW    |
| Transport + controller                         | `transport.ts:103-115`, `controller.ts:93-260`; only P6-010/P6-019/P6-020 fixes needed                                  | 4-6 h                | MEDIUM |
| Provider tree                                  | `BoardConnectionContext.tsx:78-90`, `BoardStateContext.tsx:61-136`, `CommandContext.tsx:85-193`, `TransportContext.tsx` | 4-6 h (+ P2-001 fix) | MEDIUM |
| Docs screen + catalog + generated bundle       | `catalog.ts:28-43`, `_generated-docs.ts:1-3`, `MarkdownRenderer.tsx:62-93` (delete debug log P6-001)                    | 2-4 h                | LOW    |
| shadcn primitives + tokens                     | `ui/shadcn/*` (5-73 lines ea.), `design/tokens.ts`                                                                      | 0-2 h                | LOW    |
| Web flasher                                    | `webEspFlasher.ts:38-103` is cleanly separated from UI                                                                  | 2 h                  | LOW    |

### Replace / split

| Area                                                       | Current                                                                                                                   | Target                                                                           | Effort  | Risk   |
| ---------------------------------------------------------- | ------------------------------------------------------------------------------------------------------------------------- | -------------------------------------------------------------------------------- | ------- | ------ |
| `AppExperience.tsx` (1148)                                 | dual state (`@:125-152`) + own `runCommand` (`@:385-444`)                                                                 | thin shell; all state through providers; delete local copies                     | 12-16 h | HIGH   |
| `ControlsBlocks.tsx` (1189)                                | command palette + styles + panels                                                                                         | `ControlsBlocks.styles.ts`, `CommandPalette`, `CommandGroupCard`, `BusStatusBar` | 8-12 h  | MEDIUM |
| `DriveScreen.tsx` (855)                                    | monolith, 1 test                                                                                                          | `DriveStatus`, `DasControls`, `GamepadControls`, `SpeedLimits`                   | 6-8 h   | MEDIUM |
| `ConnectionSection.tsx` (608) + `MonitorSection.tsx` (431) | monitor UI                                                                                                                | `ConnectionForm`, `CommandRunner`, `FrameTable`, `DiagnosticsPanel`              | 8-10 h  | MEDIUM |
| `FlasherScreen.tsx` (439)                                  | UI + local state                                                                                                          | `FlasherConfig`, `FlasherLog`, keep `webEspFlasher.ts`                           | 4-6 h   | MEDIUM |
| Control panels (raw casts)                                 | `FsdPanel.tsx:25`, `DasPanel.tsx:15`, `GamepadPanel.tsx:19`, `VehicleCommandsPanel.tsx:133`, `TelemetryPanel.tsx:114-119` | typed `BoardState` view fields or a decoder adapter                              | 4-6 h   | MEDIUM |
| Command-contract reconciliation                            | 12 ghost groups (section 5)                                                                                               | firmware-backed truth + generated builders                                       | 6-10 h  | HIGH   |
| Gating fixes                                               | P2-002..005 + dead `nag` gate (`gating.ts:52`)                                                                            | capability-based gates + tests                                                   | 2-3 h   | LOW    |

**Total estimate: ~60-90 hours** (~8-12 working days), of which **12-16 h is high-risk contract work**.

### Order that avoids chasing a moving protocol contract

1. Freeze truth: change `cross-check.test.ts:35-361` to parse firmware `strcmp/strncmp(cmd, ...)` literals at test time; delete the hardcoded list and the `:708` whitelist. (2-3 h)
2. Resolve the 12 ghost groups: either add firmware handlers or remove/alias the builders, specs, and docs together; fix `apfirst`→`ap-first`, `lanegraph`→`lane-graph`. (6-10 h)
3. Stabilize `types.ts`/`reducer.ts` wire types and `board.ts` validation (P2-009, P6-005, PA-009); add `board.ts` tests. (4-6 h)
4. Wire `busDispatch` into `CommandProvider` (P2-001), then delete `AppExperience`'s duplicated state. (6-8 h)
5. Extract styles/panels from `ControlsBlocks`, `DriveScreen`, monitor sections. (20-26 h)
6. Rebuild screens against providers only; last touch docs/flasher, which do not depend on the command contract. (12-16 h)

Steps 3-6 are then stable because steps 1-2 fix the builder → wire → firmware chain before UI is rewritten against it.

## 7. New findings (not in old todo)

Severity: HIGH = user-visible failure or false safety net; MEDIUM = correctness/type debt; LOW = cosmetic.

| #   | Severity | Finding (evidence)                                                                                                                                                         | One-line fix                                                                        |
| --- | -------- | -------------------------------------------------------------------------------------------------------------------------------------------------------------------------- | ----------------------------------------------------------------------------------- |
| N1  | HIGH     | 12 advertised command groups (~29 wire strings) have no firmware handler; they hit `Unknown command` (`dispatch.h:1085`) — see section 5.                                  | Implement firmware handlers or delete builders/specs/docs together.                 |
| N2  | HIGH     | `cross-check.test.ts:35-361` pretends a hardcoded list is firmware truth; `:708` whitelists the `apfirst`/`lanegraph` mismatch.                                            | Parse `dispatch.h` literals in the test and remove the whitelist.                   |
| N3  | HIGH     | `CommandContext.tsx:132-180` ignores `result.dispatchActions`, so provider-driven screens have permanently empty command lifecycle (P2-001).                               | Destructure `busDispatch` and dispatch actions as `AppExperience.tsx:419-421` does. |
| N4  | MEDIUM   | `gating.ts:52-57` defines a `nag` gate, but registry keys are `nagMode`/`nagBypass` (`commands.ts:129,133`), so the rule is unreachable and both nag commands are ungated. | Rename the gate to `nagMode` and add a `nagBypass` gate.                            |
| N5  | MEDIUM   | Self-referential gates (P2-002..005) block enabling `seatbelt`, `wiperpersist`, `singleShot`, `simu` until already enabled.                                                | Gate on firmware capability bits, not live state flags.                             |
| N6  | MEDIUM   | `board.ts:88-104` drops `eap`/`evd`/`tlssc` declared optional at `types.ts:13-15`.                                                                                         | Copy the three flags in `normalizeFeatures`.                                        |
| N7  | MEDIUM   | `AppExperience.tsx:125-152` keeps local `boardState`/`history` alongside providers — two sources of truth.                                                                 | Remove local copies; read solely from `useBoardConnection`.                         |
| N8  | MEDIUM   | `MarkdownRenderer.tsx:73-80` debug `console.warn` in every theme render.                                                                                                   | Delete the block.                                                                   |
| N9  | MEDIUM   | `board.ts:145-146` casts wire payload to `BoardMessage` with no validation; `parser.ts:28-120` already has normalizers that could be reused.                               | Validate with a type guard before `reduceBoardMessage`.                             |
| N10 | LOW      | `controller.ts:95,198` single `inflight` pointer races concurrent `runCommand` calls; `setTransport` swallows failures (P6-010/P6-020).                                    | Use `Map<id, Promise>` and surface swap errors.                                     |
| N11 | LOW      | `transport.ts:90-92` accepts any `baseUrl` string.                                                                                                                         | Validate with `new URL()` in the constructor.                                       |
| N12 | LOW      | `AppView.tsx` vs `AppView.web.tsx` duplicate body (P6-006).                                                                                                                | Keep one component; use a platform-specific entry only if behavior diverges.        |

Effort/risk roll-up is in section 6; N1/N2/N3 are prerequisites for any UI rewrite estimate to hold.

## Explicitly NOT read / UNVERIFIED

- **CAPTURE: none.** No tests were executed, no builds, no device/wire traces, no console output; all statuses are static-code judgments.
- `tools/**`, `legacy/**`, CI workflows, `docs/superpowers/` artifacts other than `todo.md`, and all sibling extraction reports were not read.
- Firmware was read only at `firmware/lib/client/command/dispatch.h` (lines 1-120, 975-1086) plus literal greps across `firmware/lib`; feature-handler internals, CAN mutation, checksums, and transport internals were **not** reviewed. `features.h`/`messages.h` in `firmware/lib/client/command/` were not read.
- Client source bodies not fully read: `DriveScreen.tsx` (855), `FlasherScreen.tsx` (439), `ConnectionSection.tsx` (608, only lines 1-35 and the old claim's `:473`), `MonitorSection.tsx` (431), `runtimeAdapters.ts` (313), `TransportContext.tsx` (164), all `ui/shadcn/*`, the four `can-decoder/*.json` assets, and the 11754-line `_generated-docs.ts` body (header only).
- Protocol source not fully read: `featureSettings.ts` beyond `1-120` and `540-589`, `types.ts` beyond `1-130`, `parser.ts` beyond `1-120`, `reducer.ts` beyond targeted greps, `format.ts` beyond `1-60`, `selectors.ts` beyond `85-114`.
- Test bodies read: only `CommandContext.test.tsx`, `command-gating.test.ts`, `cross-check.test.ts`. All other test counts are grep-counted, not executed; **test pass/fail status is UNVERIFIED**.
- Precise count of "29" ghost wire strings is derived from builder arity enumeration in `cross-check.test.ts:417-647`; an exact deduplicated count was not machine-verified. Treat as approximate.
- `monitorExportComposition.ts` repetition (PA-006 follow-up) and `ConnectionSection.tsx:473` cast (P6-003) remain UNVERIFIED.
