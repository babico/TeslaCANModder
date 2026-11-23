# Task C1 — Linux TX backend + can-review — report

Branch: `spike/raspberry-pi-portability` (never main, no force-push).
Worktree: `C:\Users\boran\Masaüstü\KL\Kodlama\TeslaCANModder`.

## Status

DONE — a **DISABLED-by-default** gated CAN TX backend. `PI_CAN_TX=0` (anything
other than literal `"1"`) and B3's `isGateOpen()` are both required; no gate
artifact exists here, no physical Pi or bench capture was available, and no
frame was sent on a real bus. TDD RED/GREEN confirmed, typecheck/tests/lint
clean, committed and pushed, self-reviewed. Real injection remains a release
blocker, not a claim of this task. No Sentry code was read, copied or adapted.

## TDD evidence

**RED** — added only `pi/test/tx.test.ts`, ran `npm test -w @teslacanmodder/pi`:

```
FAIL test/tx.test.ts
  TS2305: Module '"../src/index.js"' has no exported member 'isCanTxEnabled'.
  TS2307: Cannot find module '../src/can/tx.js' or its corresponding type declarations.
Test Suites: 1 failed, 6 passed, 7 total
Tests:       55 passed, 55 total
```

**GREEN** — after implementing `pi/src/can/tx.ts` and wiring the flag in
`pi/src/index.ts` (one test-oracle bug fixed along the way: my first expectation
omitted the firmware's `& 0xff` checksum wrap; the implementation was right):

```
PASS test/bridge.test.ts
PASS test/socketcan.test.ts
PASS test/rt.test.ts
PASS test/smoke.test.ts
PASS test/record.test.ts
PASS test/deploy.test.ts
PASS test/tx.test.ts
Test Suites: 7 passed, 7 total
Tests:       74 passed, 74 total
```

New tests (19): flag off/unset/non-literal/no-transport/missing artifact;
tampered measurements, lying verdict, wrong key, short cached frame — all prove
`send()` is a no-op returning `false` with zero transport calls; reachability
with a real signed artifact + fake transport and with an injected fake gate
(both open and shut); bit-for-bit frame comparison against an **independent
oracle** written from the firmware spec (not importing the builder); SocketCAN
16-byte wire encoder oracle; additive checksum wrap; DLC guard.

Validations (all clean): `npm run typecheck:pi`, `npm test -w @teslacanmodder/pi`
(74/74), `npm run lint:all` (exit 0: ESLint + Prettier + 48 markdown files,
0 issues), `npx markdownlint-cli2 pi/gate/README.md` (0 issues),
`npx prettier --check` on all four changed files.

## Files changed (only these four)

1. `pi/src/can/tx.ts` (new) — gated backend, `buildDriveModeFrame()` /
   `driveChecksum()` / `encodeCanFrame()` / `readGateArtifact()` /
   `createGatedTx()`. Pure at import: no fs, no socket, no clock.
2. `pi/test/tx.test.ts` (new) — 19 tests with independent frame/wire oracles.
3. `pi/gate/README.md` (new) — two-condition gate contract, DISABLED status,
   release blockers, no-Sentry statement.
4. `pi/src/index.ts` (modified) — added exported `isCanTxEnabled(env)` flag
   helper; `/health` response unchanged, so existing smoke tests stay valid.

No other file staged or committed. ~200 pre-existing unrelated unstaged files
left untouched (count was 202 before and after). Staged list verified before
commit. The report file itself is untracked, matching prior tasks.

## can-review checklist — honest walk

Scope note: every bus-level answer is **capture-first / UNVERIFIED** because no
listen-only capture, board or gate artifact was available. "PASS (structural)"
means repo-source or unit-test evidence only, not vehicle acceptance.

### 1. Frame mutation safety

- DLC read guard — PASS (structural): `buildDriveModeFrame` rejects
  `cachedFrame.length < 8` before `subarray(0, 8)`; `encodeCanFrame` validates id
  and DLC before writing.
- DLC write guard — PASS (structural): writes occur only on the guarded 8-byte
  copy and the 16-byte local wire buffer.
- Shared helpers reject invalid indexes/out-of-DLC writes — PASS (structural):
  mode index is a keyed `Record<TxMode, number>` (no arbitrary bit index API);
  `encodeCanFrame` rejects id outside 0..0x7ff and DLC outside 0..8.
- Bit writes isolated — PASS (test): oracle asserts all three modes change only
  byte[0] bits[6:5] and preserve every other cached byte. Vehicle effect UNVERIFIED.
- Checksum recalculated — PASS (test): byte[7] rebuilt via firmware additive
  `driveChecksum()` and asserted against the independent oracle. On-bus acceptance
  UNVERIFIED (no capture).

### 2. Bit and field ownership

- Exact bits documented — PASS: byte[0] bits[6:5] = 2-bit drive mode, byte[7] =
  additive checksum, frame 0x334; recorded in code comments, gate README and here.
- Cross-checked against active behavior — PASS (source): ported from active
  `firmware/lib/vehicle/can/feature/drive/drive_mode.h` and `ids.h`, not a stale doc.
- Legacy source disagreements — N/A: no legacy sketch consulted; no Sentry code used.
- Mux-specific behavior — N/A: 0x334 drive-config is not muxed in our firmware.
- RX/TX ownership — PASS (structural): dedicated TX module; the A3 HTTP bridge's
  `ALLOWED_COMMANDS` remains read-only (`status`, `ping`), so no HTTP path reaches TX.

### 3. Variant behavior

- legacy/hw3/hw4 considered — UNVERIFIED: firmware drive-mode override is
  variant-agnostic (vehicle bus); no variant gate added. No board evidence.
- Unsupported controls stay unsupported — PASS: bridge still rejects fsd/summon/drive.
- hw3 offset vs profile/nag — N/A: not touched.
- hw4 ISA mux/checksum — N/A: not touched.
- legacy accidental change — PASS: no legacy helper reused.

### 4. Stream and I/O protocol review

- serial `board.h` — N/A: no firmware wire-shape change.
- client transport/parsing — N/A: no wire-shape change.
- boot/status capability — PASS: Pi `/health` payload unchanged; flag is a pure export.
- stream `dir`/`id`/`dlc`/`d` — PASS: A2 read path untouched.
- WiFi/BLE parity — N/A: no command added or routed.
- command routing vs handlers — PASS: no new command reaches any dispatch path.

### 5. Required regression tests

- exact bit-set assertion — PASS: `it.each` over chill/standard/performance vs oracle.
- short-frame no-send — PASS: builder returns `undefined`; `send()` verified no-op.
- variant-specific regression — N/A (no variant behavior in this port); UNVERIFIED on vehicle.
- stream/message-shape assertion — PASS: 16-byte SocketCAN encoding compared byte-for-byte
  to an independent encoder.

### 6. Manual reviewer questions

- Which CAN IDs intercepted/transmitted? Adds a potential 0x334 TX path only;
  no RX change; gated off here. Bus effect UNVERIFIED.
- Bit position/checksum/mux/profile changed? Only the ported 0x334 mode bits and
  additive checksum; no mux/profile change. Vehicle result UNVERIFIED.
- Any path transmitting while disabled? None found: flag + `isGateOpen()` +
  transport all required at construction; `send()` returns early when shut. Tested.
- Browser-visible stream/status meaning? No change.
- Legacy sketch correct reference? None used; active firmware is canonical.
- Bus routing (0/1/2)? Unchanged; firmware targets 0x334 on vehicle bus (index 1),
  but no real transport is wired here, so UNVERIFIED.
- 3 MCP2515 buses? Untouched.
- WiFi/BLE vs serial consistency? Unchanged.

### 7. High-risk change types

- firmware helper changes — none (Pi-only).
- mux/profile/offset/ISA/nag — none.
- checksum changes — new additive-checksum port, not a firmware modification;
  capture-first UNVERIFIED for vehicle acceptance.
- handler filter IDs — none.
- stream message shape — TX encoder added and asserted; RX stream untouched.
- command entry changes — only a pure flag export; no command routed.
- MCP2515 driver — none.
- bus routing — none.
- WiFi/BLE endpoints — none.

### 8. Minimum acceptance bar

- Bit-level behavior asserted — PASS (unit test).
- Short-frame behavior asserted — PASS.
- Active variant explicit — N/A in this port; no vehicle evidence, so UNVERIFIED.
- Reviewer can explain IDs/bits — 0x334, byte[0] bits[6:5], byte[7] additive sum.
  **Full acceptance bar NOT met for real injection**: listen-only capture and a
  signed checklist review are still required (see concerns).

## Self-review

- Confirmed `import` of `node:fs` is lazy in effect: `readFileSync` is only called
  by `readGateArtifact()` when `createGatedTx()` evaluates an open flag; no I/O at
  import. No default socket, runner or writer exists.
- Gate is evaluated once at construction; `enabled` is a readonly snapshot and
  `send()` re-checks it, so a gate cannot be bypassed by a later call.
- `isGateOpen()` is reused verbatim — no second gate implementation.
- Gate key comes from `options.gateKey ?? env.PI_RT_GATE_KEY`; never logged or stored.
- `readGateArtifact()` swallows all read/parse errors and returns `undefined`
  (fail-closed); no partial object can reach `isGateOpen()`.
- `encodeCanFrame()` uses `DataView.setUint32(..., true)` matching the reader's
  native little-endian `struct can_frame` layout; oracle test proves equality.
- `send()` lets a transport error propagate rather than silently reporting
  success; no retry storm is introduced.
- gate README states DISABLED status and release blockers up front; markdownlint clean.
- No Sentry code, no hardware claim, no artifact and no CAN TX activation in committed files.

## Concerns / deferred

1. **Vehicle acceptance is absent by design.** No listen-only capture and no
   bench hardware → every CAN item above is structural/test-only. Real injection
   needs a signed `docs/checklists/can-review-checklist.md` walk against a capture.
2. **No gate artifact.** `pi/gate/rt-result.json` does not exist; the unit test
   asserts its absence. C1/D1 must delete/replace that assertion when a real
   measured artifact is generated on the board.
3. **Artifact contract remains B3's self-defined v1** (HMAC over canonical JSON).
   C1 consumes it as-is via `isGateOpen()`; if the controller chooses a different
   contract, the change is local to B3's `measure.ts` and this consumer call.
4. **Frame scope is deliberately minimal** — only the 0x334 drive-mode builder.
   CRC-8/OPENSAFETY frames (0x229/0x249/0x370) were NOT ported: their magic tables
   are capture-derived and there is no bench data here to validate an injected frame.
5. **No real transport or on-board entrypoint.** A later task must wire a
   SocketCAN gateway/native binding and the systemd service before `PI_CAN_TX=1`
   can do anything off a test bench.
6. **Transient test flake (pre-existing)** — one run hit a `bridge.test.ts` port
   race; the next full run passed 74/74. Same flake noted in B3's report; unrelated
   to this change.
