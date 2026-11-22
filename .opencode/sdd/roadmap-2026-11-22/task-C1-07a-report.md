# Task C1-07a — Single injection-gate authority — report

Status: **COMPLETE** (code + tests + checklist). Real-vehicle acceptance: **UNVERIFIED** (safety-only change; no frame shapes touched).

- Branch: `chore/legacy-tesla-drug-fork-submodules`
- Code commit: `d94184d` — `fix(firmware): harden AP injection gate authority`
- Report commit: see `git log -1` on this file (docs-only follow-up, same pattern as C1-08).
- Staging: exactly 6 code files staged via explicit paths; `git diff --cached --name-only` verified before commit. No `-A`, no unrelated files swept (191+ pre-existing unstaged files untouched).

## 1. Requirement

From brief / plan `roadmap-2026-11-22.md:65`:

1. Gate `dasTick` on `apGateOpen()`.
2. Count only gear 1 as Park (0/5 fail closed).
3. Remove CAN-timeout force-open at `main.cpp:149`.
4. Add bounded freshness (`hasCtrl`, `apGateApActive` timestamps).

Controller ruling (binding): conservative fail-closed rule only; exact gear-bit ground truth is CAPTURE_FIRST → C1-07b. No new injection paths, no frame layout/checksum changes. Keep `das:arm` UNSUPPORTED from C1-10 intact. Walk can-review checklist; frame-shape items N/A.

Acceptance: extend `test_native_ap_gate` with gear 0/5 cases, `dasTick` gate-closed → 0 sends, stale timestamp → no injection.

## 2. Timeout value — justification (not guessed)

Added `#define AP_GATE_FRESHNESS_MS 1000` in `firmware/lib/core/types.h`. Source: the repo's own design analysis `.opencode/analysis/current-state-2026-11-22/firmware.md:242` states "Fix: store timestamps and require age **< 1 s** in every injection gate." 1 s is also >10x the DAS_STATUS / 0x273 periodic rates seen in the codebase, so normal bus traffic cannot spuriously close the gate. Existing `CAN_TIMEOUT_MS` (10 s) was deliberately **not** reused — that is a standby threshold, far too long for a live injection gate.

## 3. Changes

### `firmware/lib/core/types.h`

- Added `AP_GATE_FRESHNESS_MS 1000`.
- Added `lastCtrlMs` (0x273 cache timestamp), `apGateApActiveMs` (DAS AP-active timestamp), `gateNowMs` (loop-cached clock), all initialized to 0 in `State()`.
- `apGateOpen(unsigned long now)` is now the **single injection-gate authority**: gate disabled → open; else AP leg counts only when `apGateApActive && (now - apGateApActiveMs) < 1000`; Park/Summon legs unchanged. Unsigned subtraction gives fail-closed on a future/rolled timestamp.
- No-arg `apGateOpen()` delegates to `apGateOpen(gateNowMs)`, so **every** existing gate caller (ticks, handlers, status/WiFi/BLE output) inherits the same freshness rule. Avoiding `millis()` inside the header keeps header-only native test TUs linkable.
- Added `hasFreshCtrl(now)` for the cached 0x273 frame age bound.
- Added `setGearState(gear)`: stores `gearState` and sets `apGateParked = (gear == 1)`.

### `firmware/lib/vehicle/can/handler/bus/vehicle.h`

- 0x273 cache: `s.lastCtrlMs = millis()` alongside `hasCtrl = true`.
- DAS_STATUS: `s.apGateApActiveMs = millis()` alongside the AP-active decode.
- DI*STATE (0x118): routed through `s.setGearState(decodeGearState(f.data))`. Decode helper, bit positions, DLC guard and all other fields untouched — only the \_gate predicate* changed (old `== 1 || == 0 || == 5` → `== 1`).

### `firmware/lib/vehicle/can/feature/das/das_drive.h`

- `dasTick(now, s)`: added `if (!s.apGateOpen(now)) return;` immediately after the existing disabled check, before dead-man/timers/frame builds. Zero bytes of frame construction changed.

### `firmware/lib/vehicle/can/handler/ticks.h`

- `summonTick`: uses `hasFreshCtrl(now)` and `apGateOpen(now)`; `now = millis()` hoisted above the guards. No frame bytes changed.

### `firmware/src/esp32/main.cpp`

- `loop()`: `state.gateNowMs = now;` each iteration.
- Removed `state.apGateParked = true;` from the CAN-timeout standby block; retained last CAN-derived park value instead. No `sendLog`/standby behavior change.

### `firmware/test/test_native_ap_gate/test_ap_gate.cpp`

- Added fake `Preferences` + `das_drive.h` include to exercise real `dasTick` (not a duplicate helper).
- New tests: gear 0 fail-closed, gear 5 fail-closed, gear 1 opens, dasTick gate-closed → 0 sends, dasTick stale AP → 0 sends, dasTick fresh AP → sends, ctrl-frame freshness bound.
- Reworked standby helper/test: removed its duplicated `apGateParked = true` force-open; assertion now proves standby does **not** open the gate.
- DAS statics reset in setUp/tearDown so tests are isolated.

## 4. TDD evidence

- **RED** (focused `pio test -e native -f test_native_ap_gate`, scaffolding API present but old behavior): **4 failed, 16 succeeded** — `gear 0: Expected FALSE Was TRUE`, `gear 5: Expected FALSE Was TRUE`, `dasTick gate closed: Expected 0 Was 1`, `dasTick stale AP: Expected 0 Was 3`.
- **GREEN** (focused, after fix): **20 test cases: 20 succeeded**.
- **Full native suite** (once, after final code): **1069 test cases: 1069 succeeded** (was 1062 pre-change: +8 new, one standby test renamed/reworked).
- **ESP32 build** `pio run -e esp32_chassis_8mhz`: `SUCCESS` (6.35 s); flash 26.2%, RAM 16.5%. Only pre-existing `-std=c++17` inline-variable warnings.
- `git diff --check` on the 6 files: no whitespace errors.

## 5. can-review checklist walk

Legend: `[x]` satisfied/verified, `[n/a]` not applicable (frame-shape items per controller ruling).

### 1. Frame mutation safety

- `[n/a]` No incoming `frame.data[...]` reads added — `decodeGearState(f.data)` existed; DLC guard `f.dlc >= 3` unchanged.
- `[n/a]` No `frame.data[...]` writes added or changed — no frame builder touched.
- `[n/a]` No shared bit helper changed.
- `[n/a]` No bit writes, so no adjacent-bit leakage possible.
- `[n/a]` No checksum recalculation needed — no transmitted byte changed.

### 2. Bit and field ownership

- `[x]` Exact change documented: gate predicate `gear == 1` in `State::setGearState`; no CAN bit/byte position touched.
- `[x]` Cross-checked against active behavior: old rule at `vehicle.h:393` (`1 || 0 || 5`), analysis `firmware.md:93` (5=SNA, `powertrain.h:27`) and brief ruling. Canonical source = active repo + controller ruling; no legacy sketch used.
- `[n/a]` No legacy-reference disagreement resolved here; C1-07b owns gear-bit ground truth.
- `[n/a]` No mux-specific behavior touched.
- `[x]` RX/TX ownership unchanged: same handlers, same `driverSend` sites. `dasTick` gained a gate check and sends **fewer** frames; no new TX site.

### 3. Variant behavior review

- `[x]` `legacy`, `hw3`, `hw4` considered: gate logic is variant-agnostic; all variant suites green in full run.
- `[n/a]` Unsupported controls unchanged.
- `[n/a]` HW3 speed offset / profile / nag untouched.
- `[n/a]` HW4 ISA mux/checksum untouched.
- `[x]` Legacy not accidentally changed: single shared `setGearState` predicate used by all variants; no variant-specific helper reused.

### 4. Stream and I/O protocol review

- `[n/a]` No transmitted frame changed → nothing to check against `io/serial/usb/esp32/board.h` for frame shape.
- `[n/a]` No client parser change: `frame` envelope (`dir/id/dlc/d`) unchanged.
- `[x]` `boot`/`status`: `apGateOpen` no-arg still reports the gate, now freshness-filtered; no capability flag added/removed. `das:arm` stays UNSUPPORTED (C1-10 untouched).
- `[n/a]` Stream output fields unchanged.
- `[x]` WiFi/BLE identical to serial: all share the same `State::apGateOpen()`; no routing edit.
- `[x]` Command routing unchanged; full native suite (serial/wifi/dispatch suites included) green.

### 5. Required regression tests

- `[x]` Exact predicate assertions: gear 0 → `apGateParked == false`, gear 5 → false, gear 1 → true, plus `apGateOpen()` truth.
- `[n/a]` Short-frame test: no new frame read/write path; existing `f.dlc >= 3` guard on 0x118 unchanged.
- `[x]` Variant-specific regression: all existing HW3/HW4/Legacy suites green.
- `[n/a]` Message-shape assertion: no message shape change by design.

### 6. Manual reviewer questions

- `[x]` Change which CAN IDs are intercepted/transmitted? **No** — same IDs, same TX sites; `dasTick` only suppresses TX.
- `[x]` Change any bit position, checksum, mux rule, profile mapping? **No** — explicitly out of scope (C1-07b/C1-08).
- `[x]` Add a path that can transmit when feature disabled? **No** — new gate check is an additional suppression; `apInjectionGateEnabled == false` keeps prior behavior.
- `[x]` Change browser-visible meaning of streamed frames or status? **No** for frames; status `apGateOpen` now reflects freshness by design (gate is actually closed).
- `[x]` Legacy-sketch reference? **None used.**
- `[x]` Bus routing change? **No** — DAS still `BUS_CHASSIS`; summon still `BUS_VEHICLE`.
- `[x]` Behavior change for 3 MCP2515 buses? **No** bus config/driver change.
- `[x]` WiFi/BLE consistent with serial? **Yes** — shared gate method; full suite green.

### 7. High-risk change types

- `[x]` `das_drive.h` feature-helper change — walked in full; gate added, frame builders untouched.
- `[n/a]` Mux / profile / offset / ISA — none.
- `[n/a]` Checksum — none.
- `[n/a]` Handler filter IDs — none.
- `[n/a]` Stream message shape — none.
- `[x]` Command entry change affecting CAN mutation state — none in dispatch; gate authority lives in `State`.
- `[n/a]` MCP2515 driver / bus routing / WiFi-BLE new mutation endpoints — none.

### 8. Minimum acceptance bar

- `[x]` Intended gate behavior asserted in tests (RED→GREEN shown).
- `[n/a]` Short-frame behavior — no new frame access.
- `[x]` Active variant behavior still explicit — full suite green.
- `[x]` Reviewer can explain the change: gate predicate + freshness only; zero CAN bytes changed.

Checklist signed: **yes**. All frame-shape items marked N/A per controller ruling; no applicable item left unsatisfied.

## 6. Scope discipline

- No code copied from `legacy/`; no new files under `docs/superpowers/`.
- No new injection path, frame layout, DLC, bit, mux, checksum, or command surface change.
- `das:arm`/`das:disarm`/`das:status` remain UNSUPPORTED (C1-10 status untouched).
- No unrelated working-tree changes staged; 191+ pre-existing unstaged files untouched.
- No docs markdown edited outside this report (markdown-lint not triggered).

## 7. Concerns / residual risks

1. **Real-vehicle acceptance UNVERIFIED.** No bench capture. This change only makes injection more conservative; it cannot introduce a new transmitted frame shape, but the exact gear → Park mapping still needs C1-07b capture ground truth.
2. **1 s freshness bound is a policy choice from repo analysis, not a measured vehicle period.** If a vehicle's DAS_STATUS/0x273 rate is slower than 1 s, the gate will close between frames (fail-closed, safe). Tunable via one constant.
3. **Default `apGateParked` is still `true` on a fresh `State`** (pre-existing constructor default, asserted by `test_native_persist`). Enabling the gate before the first 0x118 frame therefore still has a Park-leg opening. Changing the default would break persistence expectations and is outside this brief; flagged for controller/C1-07b.
4. **`hasFreshCtrl` is enforced in `summonTick` only.** The `!s.hasCtrl` guard in `dispatch.h:526` remains a boolean "ever captured" check for one-shot vehicle commands. The timestamp is now recorded so this can be tightened in a follow-up; not changed here to keep scope to injection-gate authority and avoid touching command routing.
5. **DAS cancel burst is suppressed while the gate is closed.** `dasTick`'s new gate check sits before the 5-frame cancel sequencing. Normal `drive:off` happens with the gate open; if the gate closes first (e.g. stale AP signal), no cancel frames are emitted. This is the literal acceptance requirement ("gate closed → 0 sends") but worth a controller safety review.
6. **`gateNowMs` is refreshed in `loop()` only.** No injection runs when `driverReady == false` or in the early-return standby branch, so a stale cached clock cannot authorize a send. Status emitted during `setup()` uses zeroed timestamps and reads fresh by construction.
7. **Unsigned-time math**: `(now - timestamp)` wraps correctly across `millis()` rollover; a timestamp in the future relative to `now` fails closed.
