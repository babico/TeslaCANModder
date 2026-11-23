# TeslaCANModder Codebase Review — Todo

Generated 2026-06-20 from a 4-agent parallel review (firmware, client+protocol, tools, cross-cutting).
105 findings total: 25 critical, ~45 medium, ~35 low.

> Run `grep -n "P[0-9]-" .opencode/plans/legacy-review-todo.md` to list by ID.
> Mark done: replace `[ ]` with `[x]`. Keep IDs stable so re-scans can diff.

---

## CRITICAL — Fix before next release

### Firmware

- [ ] **P1-001** — `firmware/lib/vehicle/ble/carserver.h:222-225` — Seat encode uses `fieldEmpty()` (length-delim) but `level` and `position` are varint enums. Wire payload is malformed. **Fix:** switch to `seat.fieldVarint`. **Effort:** 5 min. **Test:** extend `test_native_ble` to assert wire bytes.
- [ ] **P1-002** — `firmware/lib/vehicle/can/feature/misc/single_shot.h:26-30` — `singleshot:on/off` toggles `s.singleShotTx` but never calls `saveSettings(s)`. Lost on reboot. **Fix:** add `saveSettings(s)` in both branches. **Effort:** 5 min. **Test:** add reboot-roundtrip test in `test_native_persist`.
- [ ] **P1-003** — `firmware/lib/vehicle/can/feature/misc/mqtt_bridge.h:26-72` — Same bug for `mqttEnabled/Host/Port/Interval`. All four lost on reboot. **Fix:** add `saveSettings(s)` in all four command branches. **Effort:** 10 min. **Test:** reboot-roundtrip in `test_native_mqtt`.
- [ ] **P1-004** — `firmware/lib/vehicle/can/feature/misc/can_raw.h:24-32` — Same bug for `s.rawCanListen`. **Fix:** add `saveSettings(s)`. **Effort:** 5 min. **Test:** reboot-roundtrip in `test_native_can_raw`.
- [ ] **P1-005** — `firmware/lib/vehicle/can/feature/comfort/wiper.h:120-132` — Same bug for `s.wiperPersistEnabled`. **Fix:** add `saveSettings(s)`. **Effort:** 5 min. **Test:** reboot-roundtrip in `test_native_wiper_persist`.
- [ ] **P1-006** — `firmware/lib/vehicle/can/feature/drive/pedal.h:48` + `drive_mode.h:109` — Both write `0x334` byte[0] bits 5:6. Pedal CHILL=0x20 vs drive-mode CHILL=0x00; SPORT=0x40 vs PERFORMANCE=0x40 (collision). Last-write-wins, last toggled state clobbers the other. **Fix:** allocate distinct bit fields (drive-mode = bits 5:7, pedal = bits 0:2) and rebuild the byte. **Effort:** 30 min. **Test:** add `test_native_multi_feature` case that sets both in sequence and asserts final state.
- [ ] **P1-007** — `firmware/lib/core/can/ring_buffer.h:33` — `volatile uint32_t writeIdx` is NOT atomic for producer/consumer split. `volatile` only prevents compiler reordering, not torn reads. **Fix:** use `std::atomic<uint32_t>` (C++11+) or `portDISABLE_INTERRUPTS`-guarded critical section. **Effort:** 30 min. **Test:** hammer the ring buffer from two threads, assert no torn reads.
- [ ] **P1-008** — `firmware/lib/vehicle/can/handler/bus/vehicle.h:438-449` — Gear 0 (Invalid) and 5 (SNA) treated as "parked" alongside 1 (Park). Per `powertrain.h:30-32`, 5=SNA = sensor-not-available, NOT neutral. Masking real sensor faults. **Fix:** only treat `s.gearState == 1` as parked. Log SNA separately. **Effort:** 15 min. **Test:** add gear=5 case in `test_native_powertrain` asserting it's NOT parked.
- [ ] **P1-009** — `firmware/lib/vehicle/can/feature/body/turn_signal.h:44` — `controlTurnSignal` gated on `s.hasCtrl` (0x273 capture), but 0x3F5 is VCFRONT_lights, not UI_vehicleCtrl. Wrong CAN ID dependency. **Fix:** track a separate `hasLights` flag set on first 0x3F5 RX. **Effort:** 20 min. **Test:** add `test_native_turn_signal` case with no 0x273 traffic but with 0x3F5, assert command works.
- [ ] **P1-010** — `firmware/lib/vehicle/can/feature/comfort/air_recirc.h:31-42` — Builds 0x2AA (HVAC_airRecircStatus) using 5 bytes copied from `s.lastClimate` (0x2F3 = HVAC_request). Different mux layouts. Sends garbage. **Fix:** use a 0x2AA-specific snapshot or send a fresh static payload. **Effort:** 30 min. **Test:** parse returned 0x2AA, assert bits 0/1 of data[0] reflect the requested mode.
- [ ] **P1-011** — `firmware/lib/vehicle/can/feature/body/track_mode.h:23-33` — `controlTrackMode(false)` memsets 8 bytes and sends. No explicit disable bit pattern. Tesla 0x313 expects a specific off encoding. **Fix:** write known-good disable bytes (e.g. data[0] = 0x00, data[1] = 0x00, leave counter bits). **Effort:** 20 min. **Test:** confirm with bench capture.
- [ ] **P1-012** — `firmware/lib/vehicle/ble/handler/dispatch.h:194` — `if (!ecdhOk) break;` skips 4 `mbedtls_*_free()` calls. Resource leak on every ECDH failure. **Fix:** call all 4 free() before break, or refactor to goto-cleanup. **Effort:** 15 min. **Test:** run dispatch under simulated failure, valgrind.
- [ ] **P1-013** — `firmware/lib/client/command/dispatch.h:1080` — `Tesla::executeTeslaCommand(cmd + 6)` calls a template that requires `Board &` as first arg. Won't compile when `BOARD_ENABLE_BLE=1`. **Fix:** pass concrete board adapter or drop the template requirement. **Effort:** 20 min. **Test:** CI matrix should already cover both BLE on/off.

### Client + Protocol

- [ ] **P2-001** — `client/src/state/CommandContext.tsx:100` — `useReducer` destructures `[busState]` only; `dispatch` is dropped. `commandLifecycle` stays `[]` forever. All command-lifecycle UI is dead. **Fix:** `const [busState, busDispatch] = useReducer(...)`; dispatch `COMMAND_PENDING/ACKED/FAILED` around transport send. **Effort:** 30 min. **Test:** add `CommandContext` test that asserts the lifecycle list populates.
- [ ] **P2-002** — `packages/protocol/src/gating.ts:762-767` — `seatbelt` gate self-referential: checks `s.seatbeltEmulation` (the very thing the gate is supposed to enable). User can never enable. **Fix:** drop the `s.seatbeltEmulation` check or replace with a feature-capability bit from firmware. **Effort:** 10 min. **Test:** add `selectSeatbeltGate` test.
- [ ] **P2-003** — `packages/protocol/src/gating.ts:778-783` — Same bug for `wiperPersist`. **Fix:** drop the `s.wiperPersist` check. **Effort:** 10 min. **Test:** add test.
- [ ] **P2-004** — `packages/protocol/src/gating.ts:816-821` — Same bug for `singleShot`. **Fix:** drop the `s.singleShot` check. **Effort:** 10 min. **Test:** add test.
- [ ] **P2-005** — `packages/protocol/src/gating.ts:794-813` — Same bug for `canSimStart/canSimStop` (check `s.canSim`, which they set). **Fix:** drop the `s.canSim` check. **Effort:** 10 min. **Test:** add test.
- [ ] **P2-006** — `packages/protocol/src/featureSettings.ts:556-565` — `nagControl` options list 7 modes but `VALID_NAG_MODES` (commands.ts:18-27) has 8 incl. `feifan`. UI cannot select feifan. **Fix:** add `{value:"feifan",label:"Feifan"}` to options. **Effort:** 5 min. **Test:** add snapshot test.
- [ ] **P2-007** — `client/src/components/controls/ControlsBlocks.tsx:43-67` — `getPresetTheme` reads `PRESET_THEME[preset]` into `_base` then returns only `c.*` values. `preset` arg ignored. **Fix:** merge `_base` with `c` overrides, or drop the constant. **Effort:** 10 min. **Test:** add unit test for getPresetTheme.
- [ ] **P2-008** — `client/src/screens/ControlsScreen.tsx:9` — Cross-workspace import `../../../packages/protocol/src/featureSettings` instead of `@teslacanmodder/protocol`. Breaks in monorepo publish. **Fix:** import from `@teslacanmodder/protocol`. **Effort:** 5 min. **Test:** typecheck.
- [ ] **P2-009** — `client/src/state/board.ts:146` — `raw as unknown as BoardMessage` cast skips runtime validation. Wire payload treated as typed. **Fix:** Zod-parse before passing to reducer. **Effort:** 20 min. **Test:** add bad-payload test that asserts reject.
- [ ] **P2-010** — `client/src/AppExperience.tsx` — 1238-line god component. Duplicates `BoardStateContext/CommandContext/TransportContext` state via local useState/useReducer. **Fix:** extract into the new providers; delete the legacy state. **Effort:** 4-6 hours. **Test:** behaviour tests for each screen.
- [ ] **P2-011** — `client/src/components/controls/VehicleCommandsPanel.tsx:80-90` — Dead ternaries: `trackMode ? "trackMode" : "trackMode"`, `precondition ? "precondition" : "precondition"`. **Fix:** use single constants. **Effort:** 2 min. **Test:** n/a.

### Tools

- [ ] **P3-001** — `tools/serial-http-bridge.js:81-83` — CORS `*` + no auth + default `host: 0.0.0.0` + `/api/command` proxies any `cmd` to firmware. Network caller can `POST {"cmd":"fsd:on"}` and drive the car. **Fix:** bind 127.0.0.1 default, add token auth, restrict CORS. **Effort:** 30 min. **Test:** manual: external POST must 401.
- [ ] **P3-002** — `tools/serial-http-bridge.js:88-94` — `readBody` reads unbounded request body. DoS via huge POST. **Fix:** cap at 1 MB, reject oversized. **Effort:** 10 min. **Test:** POST 2 MB body, assert 413.
- [ ] **P3-003** — `tools/serial-http-bridge.js:111` — No `server.timeout` / `headersTimeout`. Slow-loris DoS. **Fix:** set both (e.g. 30s / 5s). **Effort:** 5 min. **Test:** hold connection open, assert server drops.
- [ ] **P3-004** — `tools/commands/dump.js:12-15,75` — `isSafePathInput` rejects `\0` but passes `..`. Path traversal. **Fix:** resolve, assert inside cwd. **Effort:** 15 min. **Test:** `--output ../../../etc/passwd` must refuse.
- [ ] **P3-005** — `tools/commands/replay.js:16,74` — Same path-traversal weakness on `--input`. **Fix:** same as P3-004. **Effort:** 15 min. **Test:** add test.
- [ ] **P3-006** — `tools/commands/drive-context.js:370,378` — `writeFileSync(abs, ...)` with no path check. **Fix:** assert resolved path inside cwd. **Effort:** 10 min. **Test:** add test.
- [ ] **P3-007** — `tools/commands/dump.js:23,71` — `createWriteStream` + `stream.end()` fire-and-forget. No drain wait, no error handler, no await. **Fix:** `await new Promise(...)` + `stream.on('error', …)`. **Effort:** 15 min. **Test:** write to invalid path, assert error.
- [ ] **P3-008** — `tools/lib/session.js:124` — `this._sp.write(line)` no callback. Disconnect/backpressure silent. **Fix:** pass `(err) => { if (err) ... }` or return Promise. **Effort:** 10 min. **Test:** write to closed port.
- [ ] **P3-009** — `tools/commands/das-drive.js:103-108` — `set-speed` validates `kph >= 1` but no upper bound; `set-cap` requires 1..200. Mismatched. **Fix:** cap at `after.capMax` (200). **Effort:** 5 min. **Test:** `--kph 999999` must refuse.
- [ ] **P3-010** — `tools/commands/flash.js:21-25` — `PLATFORMIO_CORE_DIR` env var honored verbatim to locate `python.exe`. Multi-user env can hijack. **Fix:** ignore env override, use only `~/.platformio`. **Effort:** 10 min. **Test:** set env to bad path, assert ignored.

### Cross-cutting

- [ ] **P4-001** — `scripts/release-matrix-smoke.mjs:117,127` — Script parses for hardcoded `strategy.matrix.env` blocks but `release.yml:90-93` uses `discover-envs` job that dynamically pulls from `firmware/platformio.ini` via jq. Every invocation of `npm run smoke:firmware:release-matrix` fails. **Fix:** rewrite parser to read `platformio.ini` directly, or delete the script. **Effort:** 30 min. **Test:** run script manually.
- [ ] **P4-002** — `docs/reference/commands.md:57` — Mermaid node labels `firmware/lib/interface/dispatch.h`. Tree removed in 1.5.0. **Fix:** replace with `firmware/lib/client/command/dispatch.h`. **Effort:** 5 min. **Test:** render mermaid.
- [ ] **P4-003** — `docs/reference/serial-contract.md:121,130,131` — References removed `lib/interface/common/json.h` and a non-existent `lib/ble/nus/esp32/common.h`. Actual BLE is `lib/io/serial/bluetooth/esp32/`. **Fix:** update paths; delete the "Mirror copy (legacy tree)" paragraph. **Effort:** 10 min. **Test:** grep for stale paths.
- [ ] **P4-004** — `firmware/README.md:9,16-23,41-43` — Lists envs `esp32`, `esp32_wifi`, `esp32_ble`, `esp32_wifi_ble` and shows `.\pio.ps1 run -e esp32_wifi_ble` style commands. None of those envs exist; `platformio.ini` uses `esp32_chassis_8mhz` etc. Every build command 404s. **Fix:** rewrite against actual env list, or replace with a one-liner that references the convention. **Effort:** 30 min. **Test:** manually run a sample command.
- [ ] **P4-005** — `.github/dependabot.yml:27-37` — References removed `/hardware` and `/web` directories. Dependabot scans 0 packages weekly. **Fix:** drop both blocks or replace with current workspaces. **Effort:** 5 min. **Test:** dependabot PR dry-run.
- [ ] **P4-006** — `.github/workflows/OPENCODE.md:40` — Documents `ANTHROPIC_API_KEY` as required secret. All four `opencode*.yml` use `OPENCODE_GO_API_KEY` with `ANTHROPIC_API_KEY` as commented fallback. **Fix:** update doc to use the actual env var. **Effort:** 5 min. **Test:** n/a.
- [ ] **P4-007** — `.github/workflows/OPENCODE.md:46` — Claims "The PR reviewer agent is read-only". `opencode-review.yml:11-15` grants `pull-requests: write` and the agent is not constrained to read-only. **Fix:** either enforce read-only in workflow or fix doc. **Effort:** 15 min. **Test:** n/a.
- [ ] **P4-008** — `.husky/pre-commit` — Runs `npx lint-staged` (prettier + eslint) but NOT markdownlint. Pre-commit accepts MD that CI rejects. **Fix:** add `markdownlint-cli2` step to lint-staged for `*.md`. **Effort:** 5 min. **Test:** commit a MD001 violation locally.
- [ ] **P4-009** — `firmware/scripts/check_firmware_size.py:56-58` — `if env not in THRESHOLDS: return True` passes silently for unknown envs. THRESHOLDS covers 4 envs; platformio.ini has 30+. **Fix:** `sys.exit(2)` for unknown envs. **Effort:** 5 min. **Test:** call with bogus env.
- [ ] **P4-010** — `.github/workflows/ci.yml:42-43` — Size regression checked only on `esp32_chassis_8mhz` (smallest). Release matrix builds 30+ envs incl. larger chassis+vehicle+body combos. **Fix:** add `esp32_wifi_ble_chassis_vehicle_body_8mhz` to size check. **Effort:** 10 min. **Test:** CI run.

---

## MEDIUM — Smell, fragility, type-safety

### Firmware (16)

- [ ] **P5-001** — `firmware/lib/vehicle/can/feature/das/das_drive.h:75-92` — 16 file-scope `static` variables. Per-TU copy risk. **Fix:** gather into struct in anonymous namespace or move to single .cpp. **Effort:** 1 hour.
- [ ] **P5-002** — `firmware/lib/vehicle/can/feature/fsd/nag/math.h:16` — `static uint32_t _nagPrngState` global mutable. Not ISR-safe. **Fix:** pass PRNG state explicitly, or use atomic. **Effort:** 20 min.
- [ ] **P5-003** — `firmware/lib/vehicle/can/feature/fsd/auto_lane_change.h:46` — `static uint8_t _alcStalkCounter` per-TU risk. **Fix:** move to anonymous namespace in .cpp. **Effort:** 10 min.
- [ ] **P5-004** — `firmware/lib/vehicle/can/feature/fsd/nag.h:43-50` — `_nagRandFloat` uses `_nagXorshift() & 0xFFFFFF` (low bits, poor avalanche). **Fix:** shift right by 8 for upper bits. **Effort:** 10 min.
- [ ] **P5-005** — `firmware/lib/vehicle/can/feature/fsd/nag.h:281-294` — `nagOrgApplyGripExcursion` has two near-duplicate magnitude computations. **Fix:** extract helper. **Effort:** 15 min.
- [ ] **P5-006** — `firmware/lib/core/can/health.h:43-71` — `checkCanHealth` only marks `receiving=true` for bus 0. Buses 1, 2 never report. **Fix:** track per-bus `lastRxMs` and `receiving` for all 3 buses. **Effort:** 30 min.
- [ ] **P5-007** — `firmware/lib/core/driver/common.h:235-237` — `_driverTxFailCount++` not atomic, but `driverGetAndResetTxFails` uses `portDISABLE_INTERRUPTS`. Asymmetric protection. **Fix:** atomic or interrupt-guarded. **Effort:** 15 min.
- [ ] **P5-008** — `firmware/lib/core/driver/esp32/board.h:43-45` — `mcpISR_*` non-static `IRAM_ATTR` inline in header. Multi-TU inclusion attaches different function pointers. **Fix:** move ISRs to single .cpp. **Effort:** 30 min.
- [ ] **P5-009** — `firmware/lib/core/log/ring.h:38-46` — `logRingInit` zeros `used` flags but not entry message text. Stale text leaks into dumps. **Fix:** `memset(entries, 0, sizeof(entries))`. **Effort:** 5 min.
- [ ] **P5-010** — `firmware/lib/core/persist/nvs.h:14`, `native.h`, `esp32/board.h` — `static Preferences prefs;` in 3 files. Conflicting definitions. **Fix:** define once in `esp32/board.h`. **Effort:** 20 min.
- [ ] **P5-011** — `firmware/lib/core/persist/native.h` and `native/board.h` — identical `loadSettings/saveSettings` implementations. **Fix:** one should include the other. **Effort:** 10 min.
- [ ] **P5-012** — `firmware/lib/vehicle/can/feature/comfort/seat.h:125-126` — `char lastChar = cmd[strlen(cmd) - 1];` no empty-string guard. **Fix:** explicit length check. **Effort:** 5 min.
- [ ] **P5-013** — `firmware/lib/vehicle/can/feature/das/das_drive.h:11` — `#include <Preferences.h>` unconditional. Native tests get a stub. **Fix:** guard with `#ifndef ARDUINO`. **Effort:** 10 min.
- [ ] **P5-014** — `firmware/lib/vehicle/can/feature/comfort/turn_signal.h:30-31` — `setTurnSignalRequest` writes `f.data[0]` no DLC guard. Currently safe (caller builds 8-byte), but publicly exposed. **Fix:** add `if (f.dlc < 1) return;`. **Effort:** 5 min.
- [ ] **P5-015** — `firmware/lib/vehicle/can/feature/fsd/nag.h:468-480` — Dead clamps (`t.raw < 0 ? 0 : t.raw` and `> 0x0FFF ? 0x0FFF`). State machine never produces out-of-range. **Fix:** remove clamps, add debug assert. **Effort:** 10 min.
- [ ] **P5-016** — `firmware/lib/vehicle/can/handler/bus/vehicle.h:355,438,528` — 3 separate `if (f.id == CAN_ID_GTW_CAR_CFG)` blocks. Same condition evaluated 3× per frame. **Fix:** fold into one block. **Effort:** 20 min.

### Client + Protocol (20)

- [ ] **P6-001** — `client/src/components/docs/MarkdownRenderer.tsx:73-80` — Debug `console.warn` with eslint-disable no-any left in production render path. Fires on every theme change. **Fix:** delete block. **Effort:** 2 min.
- [ ] **P6-002** — `client/src/AppExperience.tsx:573-613` — useEffect for auto-poll has `pollIndex` in deps but never reads it. Re-sets interval whenever `pollIndex` changes (intended?). **Fix:** include `readStatus` or memoize. **Effort:** 15 min.
- [ ] **P6-003** — `client/src/components/controls/FsdPanel.tsx:25-28` — `boardState as unknown as Record<string, unknown>` cast. Bypasses type safety. Same in `DasPanel:15`, `GamepadPanel:19`, `VehicleCommandsPanel:133`, `ConnectionSection:473`, `TelemetryPanel:114,119`. **Fix:** extend `BoardState` with optional fields or use Zod extension. **Effort:** 1 hour.
- [ ] **P6-004** — `client/src/components/controls/DasPanel.tsx:21-22` — useState seeded from `liveLimit || 25`. Controlled `value/onChangeText` overwrites user typing whenever live value differs. **Fix:** drive from `defaultValue` + `onSubmitEditing`, or sync only on initial mount. **Effort:** 20 min.
- [ ] **P6-005** — `client/src/state/board.ts:88-104` — `normalizeFeatures` drops `eap/evd/tlssc`. Feature caps from firmware lost. **Fix:** extend `BoardFeatures`. **Effort:** 15 min.
- [ ] **P6-006** — `client/src/AppView.tsx` + `client/src/AppView.web.tsx` — byte-for-byte identical. Only one resolves via Metro. **Fix:** delete one or split platform-specific code. **Effort:** 5 min.
- [ ] **P6-007** — `client/src/screens/DashboardScreen.tsx:83-152` — mobile branch duplicates overview (lines 99-124 vs desktop 161-188). 3 near-identical copy-pastes. **Fix:** extract `<OverviewSection boardState={boardState} />`. **Effort:** 30 min.
- [ ] **P6-008** — `client/src/hooks/useDiagnostics.ts:91-101` — `savePersistedDiagnosticsState` runs every render. **Fix:** debounce 250-500ms. **Effort:** 15 min.
- [ ] **P6-009** — `client/src/state/monitorCommandExecution.ts:113-198` — `dispatchActions` built but never consumed in `AppExperience`'s wired path. **Fix:** wire `busDispatch(action)` in `AppExperience runCommand`. **Effort:** 20 min.
- [ ] **P6-010** — `client/src/hardware/controller.ts:198-202` — stores inflight Promise for all concurrent commands; second `runCommand` overwrites. Race. **Fix:** use `Map<id, Promise>` or chain. **Effort:** 30 min.
- [ ] **P6-011** — `client/src/state/commandBus.ts:158-171` — `PURGE_SETTLED` filter references `e.finishedAt !== null` but type says `finishedAt?: number`. **Fix:** drop `!== null`. **Effort:** 5 min.
- [ ] **P6-012** — `packages/protocol/src/selectors.ts:96-100` — `selectChargeSnapshot` `fully_charged` branch uses `chargeKw < 0.5`. Duplicates `resolveChargeState.ts:54`. **Fix:** pick one, import. **Effort:** 20 min.
- [ ] **P6-013** — `client/src/screens/ConsoleScreen.tsx:33` — `Object.keys(props.boardState.canHealth ?? {}).length` recomputes every render. **Fix:** useMemo or read `.chassisOnline` directly. **Effort:** 5 min.
- [ ] **P6-014** — `client/src/components/TelemetryPanel.tsx:114-119` — per-render allocation of `(state as unknown as Record<string,number>)[key]`. **Fix:** typed map. **Effort:** 15 min.
- [ ] **P6-015** — `client/src/components/monitor/sections/ConnectionSection.tsx:22-27` — `runnerCommand` defaults via `Object.entries` on `ALL_COMMANDS`. Fragile. **Fix:** pin to specific name or use `commands.ramStyle` fallback. **Effort:** 15 min.
- [ ] **P6-016** — `client/src/state/useBreakpoint.ts:1-59` — hook used in `DashboardScreen` but `DashboardScreen` also reads `width` from `useWindowDimensions` directly. Duplicate reads. **Fix:** rely on `bp.width`. **Effort:** 10 min.
- [ ] **P6-017** — `client/src/state/CommandContext.tsx:60-72` — hooks throw if context is null but never log. **Fix:** add `if (!ctx) console.error(...)`. **Effort:** 5 min.
- [ ] **P6-018** — `client/src/screens/DocsScreen.tsx:228-231` — useMemo depends on `docTree.children` but `docTree` rebuilt whenever catalog loads. **Fix:** add `docTree` reference dep. **Effort:** 5 min.
- [ ] **P6-019** — `client/src/hardware/transport.ts:111-115` — `HttpCommandTransport` takes config but never validates `baseUrl`. **Fix:** `new URL(config.baseUrl)` on construct. **Effort:** 10 min.
- [ ] **P6-020** — `client/src/hardware/controller.ts:107-111` — `setTransport` swallows in-flight rejection with empty `catch{}`. Race when transport swap happens during slow request. **Fix:** re-throw or surface via lastResult. **Effort:** 15 min.

### Tools (15)

- [ ] **P7-001** — `tools/lib/session.js:32-34` — `createInterface` constructed, listener attached, but `rl` never stored/closed. **Fix:** store `this._rl`, close in `close()`. **Effort:** 10 min.
- [ ] **P7-002** — `tools/lib/session.js:9-13` — `cb` can fire synchronously inside `SerialPort` constructor. **Fix:** use `sp.open()` promise pattern. **Effort:** 15 min.
- [ ] **P7-003** — `tools/lib/session.js:115` — silent no-op when port down. **Fix:** throw or `Promise.reject`. **Effort:** 5 min.
- [ ] **P7-004** — `tools/commands/dump.js:47-49` — CSV row built by template-literal interpolation; untrusted `m.dir` / `m.d` not escaped. Excel formula injection. **Fix:** prefix cells starting with `=,+,-,@,\t,\r` with `'`. **Effort:** 15 min.
- [ ] **P7-005** — `tools/commands/dump.js:34-63` — `stream.write` return value ignored. **Fix:** `if (!stream.write(row)) await once(stream, 'drain')`. **Effort:** 15 min.
- [ ] **P7-006** — `tools/commands/smoke.js:71-72,82-83` — `await delay(...)` after `stream:on/off` uses fixed sleeps. **Fix:** wait on actual frame arrival. **Effort:** 20 min.
- [ ] **P7-007** — `tools/commands/das-drive.js:145-147` — `can:raw:on` → `waitForAck` timeout treated as success. **Fix:** hard-fail or warn + reduce soak count. **Effort:** 10 min.
- [ ] **P7-008** — `tools/lib/args.js:34-83` — `Number(x) || default` hides invalid input. `--baud 0`, `--timeout -1`, `--speed abc` all become defaults. **Fix:** validate range, throw on bad numeric. **Effort:** 20 min.
- [ ] **P7-009** — `tools/lib/args.js:75` — `driveCtxDurMs: ... || Number(args.duration) || 30000` — `duration` also `watchDurMs`. Conflicting reuse. **Fix:** split or document precedence. **Effort:** 10 min.
- [ ] **P7-010** — `tools/serial-http-bridge.js:96-104` — `normalizeEntry` returns `{t:"text",raw:""}` for parse errors. **Fix:** preserve original `t` ("parse-error") and include `error: …`. **Effort:** 10 min.
- [ ] **P7-011** — `tools/serial-http-bridge.js:152-157` — body parsed, no Content-Type check. Form posts pass `JSON.parse('{}')` silently. **Fix:** enforce `application/json`. **Effort:** 10 min.
- [ ] **P7-012** — `tools/serial-http-bridge.js:218-222` — `main().catch` logs and exits but no `server.close()` on boot failure. **Fix:** chain close in catch. **Effort:** 10 min.
- [ ] **P7-013** — `tools/commands/replay.js:31-39` — JSON parse errors and CSV-header lines silently skipped. **Fix:** report `skipped: N`. **Effort:** 10 min.
- [ ] **P7-014** — `tools/commands/flash.js:14,113-130` — `FLASH_BAUD = "460800"` hardcoded. **Fix:** add `--flash-baud` flag. **Effort:** 10 min.
- [ ] **P7-015** — `tools/lib/diagnosis.js:7` — `m.raw.includes('"t":"boot"')` substring match. False-positive risk. **Fix:** regex `/^\{[^}]*"t":"boot"/`. **Effort:** 10 min.

### Cross-cutting (5)

- [ ] **P8-001** — `package.json:41` — `engines.node = ">=22"` but README.md:83, AGENTS.md:79, CONTRIBUTING.md:20 say `Node.js >= 18`. **Fix:** bump docs or relax engines. **Effort:** 5 min.
- [ ] **P8-002** — `firmware/scripts/check_firmware_size.py:26-51` — THRESHOLDS only 4 envs. **Fix:** generate from `platformio.ini` at build time. **Effort:** 30 min.
- [ ] **P8-003** — `CHANGELOG.md:7-18 vs :77-80` — `[Unreleased]` "Removed: `lib/interface/` and `lib/transport/`" duplicates `[1.5.0]` "Changed". **Fix:** pick one (Unreleased vs 1.5.0). **Effort:** 5 min.
- [ ] **P8-004** — `AGENTS.md:140-146` and `CONTRIBUTING.md:111-116` — Both list exactly 4 firmware envs. `platformio.ini` defines 30+. **Fix:** replace hand-maintained lists with reference to naming convention. **Effort:** 15 min.
- [ ] **P8-005** — `firmware/README.md:86` — "`hardware/` is reference material only". No `hardware/` dir exists. **Fix:** delete sentence. **Effort:** 1 min.

---

## LOW — Naming, dead code, comment-rot

### Firmware (12)

- [ ] **P9-001** — `firmware/lib/vehicle/can/feature/telemetry/powertrain.h:30-32` — comment "0=invalid, 1=P, 2=R, 3=N, 4=D, 5=SNA" but `vehicle.h:393` treats 5 as Neutral. **Fix:** align docs with code. **Effort:** 5 min.
- [ ] **P9-002** — `firmware/lib/vehicle/can/ids.h:107-122` — `readFollowDistance` and `isFSDSelectedInUI` live in `ids.h` while siblings in `frame_readers.h`. **Fix:** move to `frame_readers.h`. **Effort:** 10 min.
- [ ] **P9-003** — `firmware/lib/vehicle/can/feature/das/das_drive.h:46-48` — `dasCancelCount = DAS_CANCEL_FRAMES;` but no clear of `dasAppliedAngle` on disable. Stale angle on re-enable. **Fix:** zero on disable. **Effort:** 5 min.
- [ ] **P9-004** — `firmware/lib/vehicle/can/feature/das/das_drive.h:127-169` — `buildDasControlFrame` 40+ lines, mixes clamp, scale, pack, checksum. **Fix:** refactor. **Effort:** 30 min.
- [ ] **P9-005** — `firmware/lib/vehicle/can/feature/comfort/light.h:106` — byte 7 of 0x273 is hot (5/8 bits used). Document bit map. **Fix:** add a comment table. **Effort:** 10 min.
- [ ] **P9-006** — `firmware/lib/vehicle/can/feature/telemetry/bms.h` — many `* 0.02f`, `* 0.01f`, `* 0.002f` magic numbers. **Fix:** named constants like `BMS_KWH_SCALE`. **Effort:** 20 min.
- [ ] **P9-007** — `firmware/lib/vehicle/can/handler/bus/vehicle.h:443` — comment "Gear 1=Park, 0=Invalid, 5=Neutral treated as parked" — wrong. **Fix:** correct comment. **Effort:** 2 min.
- [ ] **P9-008** — `firmware/lib/core/can/recorder.h:151-160` — copies 8 bytes per frame regardless of DLC. Wasteful. **Fix:** only copy `f.dlc` bytes. **Effort:** 5 min.
- [ ] **P9-009** — `firmware/lib/vehicle/can/feature/fsd/nag.h:8-33` — 25 lines of architecture doc comment in header. **Fix:** move to `docs/`. **Effort:** 15 min.
- [ ] **P9-010** — `firmware/lib/vehicle/can/feature/fsd/nag.h:73-87` — `NAG_ORG_*` magic numbers without units. **Fix:** add `_MS` / `_RAW` / `_NM` suffixes. **Effort:** 10 min.
- [ ] **P9-011** — `firmware/lib/vehicle/can/feature/misc/mqtt_bridge.h:13-18` — `MQTT_*` constants local but `PKEY_MQTT_*` in `persist/keys.h` reference them. Two definitions not in sync. **Fix:** one source of truth. **Effort:** 15 min.
- [ ] **P9-012** — `firmware/lib/vehicle/can/feature/safety/ban_shield.h:50-58` — resets on enable but not on disarm. **Fix:** symmetric. **Effort:** 5 min.

### Client + Protocol (12)

- [ ] **PA-001** — `packages/protocol/src/commands.ts:666-674` — `pedalStd` returns `pedal:std` but canonical is `pedalStandard`. **Fix:** align. **Effort:** 5 min.
- [ ] **PA-002** — `packages/protocol/src/commands.ts:670` — `regenStandard` returns `regen:standard` but canonical is `regen:std`. **Fix:** align. **Effort:** 5 min.
- [ ] **PA-003** — `client/src/state/CommandContext.tsx:9-15` — file header comment says "Extracted from BoardConnectionContext (Phase B refactor)" but BoardConnectionContext still exists. **Fix:** complete refactor or remove header. **Effort:** 5 min.
- [ ] **PA-004** — `client/src/state/CommandContext.tsx:140` — `transportBlockReason: ... ?? undefined` — `commandBlockReason` is `string | null`, so `?? undefined` always wins. **Fix:** drop `?? undefined`. **Effort:** 2 min.
- [ ] **PA-005** — `client/src/state/monitorDiagnostics.ts:79-87` — `ts: nowMs - idx` synthetic monotonic. **Fix:** use `message.ts` as primary. **Effort:** 15 min.
- [ ] **PA-006** — `client/src/state/monitorExport.ts:1-94` — identical provenance structure repeated 6+ times. **Fix:** extract `serializeExport` helper. **Effort:** 30 min.
- [ ] **PA-007** — `client/src/components/controls/ControlsBlocks.tsx:9` — `ControlsPreset` type includes "service" but no distinct styling. **Fix:** drop or implement. **Effort:** 10 min.
- [ ] **PA-008** — `client/src/state/board.ts:11-13` — `isRecord` uses `Boolean(value) && typeof value === "object"` lets through empty string. **Fix:** drop `Boolean(value)`. **Effort:** 2 min.
- [ ] **PA-009** — `client/src/state/board.ts:39-44` — `busName` falls back to `Bus${normalizedBus}` but `BOARD_REDUCER` only handles 0..2. **Fix:** clamp or extend. **Effort:** 10 min.
- [ ] **PA-010** — `client/src/components/monitor/sections/ConnectionSection.tsx` — 200+ lines, largest stateful UI block. **Fix:** split into sub-panels. **Effort:** 30 min.
- [ ] **PA-011** — `client/src/components/controls/ControlsBlocks.tsx:1221` — 1221-line file, ~half is StyleSheet. **Fix:** extract styles to `ControlsBlocks.styles.ts`. **Effort:** 30 min.
- [ ] **PA-012** — `client/src/hardware/controller.ts:36-41` — `ALL_COMMANDS` sorts alphabetically, palette order never matches domain. **Fix:** group map. **Effort:** 15 min.

### Tools (10)

- [ ] **PB-001** — `tools/debug.js:42-46` — `printUsage` writes to stderr; other help paths use `console.log`. **Fix:** standardize on stdout. **Effort:** 2 min.
- [ ] **PB-002** — `tools/debug.js:106-110` — `session.send(\`variant:${opts.variant}\`)`not validated. **Fix:** validate`hw3|hw4|legacy|auto`. **Effort:** 10 min.
- [ ] **PB-003** — `tools/commands/dump.js:75-77` and `tools/commands/replay.js:74-76` — `isSafePathInput` duplicated verbatim. **Fix:** move to `lib/`. **Effort:** 5 min.
- [ ] **PB-004** — `tools/commands/replay.js:78-84` — `isReplayFrameLike` type guard exported nowhere, tested nowhere. **Fix:** move to `lib/` or add test. **Effort:** 10 min.
- [ ] **PB-005** — `tools/commands/das-drive.js:155-156` — `idRaw = f.msg.id ?? f.msg.canId; ...` clever but obscure. **Fix:** comment + test. **Effort:** 10 min.
- [ ] **PB-006** — `tools/serial-http-bridge.js:19-67` — local `parseArgs` ignores `lib/args.js`. **Fix:** import from `lib/args.js`. **Effort:** 15 min.
- [ ] **PB-007** — `tools/lib/output.js:60-62` — `ts()` uses `toTimeString().slice(0,12)` includes locale-dependent trailing GMT. **Fix:** use ISO format. **Effort:** 5 min.
- [ ] **PB-008** — `tools/lib/args.js:39-83` — mixed `Number(x) || default` vs `Number(x) !== undefined ? Number(x) : null`. **Fix:** consistent style. **Effort:** 10 min.
- [ ] **PB-009** — `tools/commands/export.js:121-141` — standalone `if (process.argv[1].endsWith("export.js"))` entry. **Fix:** move to `bin/export.js`. **Effort:** 15 min.
- [ ] **PB-010** — `tools/commands/das-drive.js:181` — Magic literal `10` for "off-but-seeing-frames threshold". **Fix:** named const + comment. **Effort:** 5 min.

### Cross-cutting (8)

- [ ] **PC-001** — `package.json:18` `lint:md` uses `npx --yes markdownlint-cli2` with no version pin. **Fix:** pin via devDependencies. **Effort:** 5 min.
- [ ] **PC-002** — `firmware/platformio.ini:99-100` — `upload_port = COM4` / `monitor_port = COM4` hardcoded. **Fix:** add a one-liner in CONTRIBUTING.md to set `PLATFORMIO_UPLOAD_PORT`. **Effort:** 5 min.
- [ ] **PC-003** — `docs/architecture/client-guide.md` — exists alongside `unified-client-guide.md`. **Fix:** delete or add "see unified-client-guide.md" banner. **Effort:** 5 min.
- [ ] **PC-004** — `tools/io-schema.json` and `tools/io.schema.json` — both referenced in review brief; both absent. **Fix:** confirm intent (regenerate or remove references). **Effort:** 10 min.
- [ ] **PC-005** — `tools/canExport.js` — review brief expects it at `tools/canExport.js`; actual is `tools/lib/canExport.js`. **Fix:** move or update brief. **Effort:** 5 min.
- [ ] **PC-006** — `CHANGELOG.md [1.0.0]:327` — historical entry references removed envs. Not actionable but worth a `[1.6.0]` footnote. **Effort:** 5 min.
- [ ] **PC-007** — `firmware/scripts/check_firmware_size.py` — import path uses `from pathlib import Path` but no test file. **Fix:** add test. **Effort:** 20 min.
- [ ] **PC-008** — `firmware/README.md:78` — `firmware` directories under `lib/` are mentioned but no longer match real structure post-refactor. **Fix:** regenerate the directory table. **Effort:** 10 min.

---

## Summary

- **Critical (25):** P1 (firmware bugs) 13, P2 (client bugs) 11, P3 (tools security/data) 10, P4 (CI drift) 10 → 25 unique items
- **Medium (45+):** P5-P8 across all areas
- **Low (35+):** P9-PC across all areas
- **Total:** ~105 items, ~25 hours of work estimated

## Recommended order

1. **Week 1 (correctness):** P1-001..013, P2-001..011 → firmware + client bugs
2. **Week 2 (security):** P3-001..010 → tools security gates
3. **Week 3 (CI drift):** P4-001..010 → make CI fail before docs/code drift
4. **Week 4 (debt):** P5-P8 medium items
5. **Backlog:** P9-PC low items

## Tags

- `bug` (P1, P2, P3) — observable user impact
- `security` (P3-001..003, P3-004..006) — exploit potential
- `drift` (P4, P6-006, P8-001) — docs/code mismatch
- `smell` (P5, P6, P7) — code quality
- `cleanup` (P9, PA, PB, PC) — cosmetic / naming
