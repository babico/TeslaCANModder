# Task B3 — RT measurement gate — report

Branch: `spike/raspberry-pi-portability` (never main, no force-push).
Worktree: `C:\Users\boran\Masaüstü\KL\Kodlama\TeslaCANModder`.

## Status

DONE. Pure gate decision + signed-artifact contract + hardware-free measurement
command skeleton + bench runbook. TDD RED confirmed, GREEN, lint/typecheck clean,
committed and pushed, self-reviewed. No gate artifact produced, no hardware
claims, TX remains disabled.

## Artifact format — design decision, needs controller/C1 sign-off

The brief/plan/ADR all say "signed gate artifact `pi/gate/rt-result.json` with
measured numbers" but **none specifies a schema or signing scheme**. I asked
myself the brief's "ASK NOW" question; no messaging channel to the controller
exists for a subagent, so I made an explicit, documented v1 choice rather than
stall. **This is the main open interface for C1.**

Choice (documented in `pi/deploy/rt-gate.md`):

- `schemaVersion: 1`, `generatedAt`, `board`, `kernel`, `method{}`,
  `measurements{}` (µs), `thresholds{}`, `verdict{pass,failures}`, `signature{}`.
- `signature.alg = "HMAC-SHA256"` over canonical JSON (recursively sorted keys)
  of every field except `signature`; key from `PI_RT_GATE_KEY`, `keyId` records
  only the key source (`env:PI_RT_GATE_KEY`), never the secret.
- C1 must call `isGateOpen(artifact, key)` — verifies signature, schema version
  and binding thresholds, requires `verdict.pass`, and re-decides from raw
  measurements. Any failure → TX off; `PI_CAN_TX` default `0`.

If the controller has a different artifact contract in mind, the change is
localised to `RtGateArtifactPayload`, `signPayload`, `verifyGateArtifact`,
`isGateOpen` and the runbook's artifact-contract section.

## TDD evidence

**RED** — `npm test -w @teslacanmodder/pi` with only `pi/test/rt.test.ts` added:

```
FAIL test/rt.test.ts
  TS2307: Cannot find module '../src/rt/measure.js' or its corresponding type declarations.
Test Suites: 1 failed, 5 passed, 6 total
Tests:       36 passed, 36 total
```

**GREEN** — after implementing `pi/src/rt/measure.ts`:

```
PASS test/bridge.test.ts
PASS test/socketcan.test.ts
PASS test/record.test.ts
PASS test/rt.test.ts
PASS test/smoke.test.ts
PASS test/deploy.test.ts
Test Suites: 6 passed, 6 total
Tests:       55 passed, 55 total
```

Focused coverage: 99/100/101 µs cyclictest; 1900/2000/2100 µs (1.9/2.0/2.1 ms)
p99.99; 4900/5000/5100 µs (4.9/5.0/5.1 ms) worst case — both sides of each
strict `<` bound. Plus fail-closed on NaN/∞/negative, missing measurements,
tampered/wrong-key/old-schema artifacts, lying verdict, missing opt-ins, and
one signed artifact written through injected fake runner/writer.

Validations (all clean): `npm run typecheck:pi`, `npm test -w @teslacanmodder/pi`,
`npm run lint:all` (exit 0), `npx markdownlint-cli2 pi/deploy/rt-gate.md`
(0 issues), `npx prettier --check` on all three files.

## Files changed (only these three, plus the fix touched two of them)

1. `pi/src/rt/measure.ts` (new, 317 lines) — `decideRtGate()` pure decision,
   threshold constants, HMAC sign/verify/`isGateOpen`, cyclictest/probe parsers,
   and `runRtMeasurement()` skeleton gated on `enable=true` + `PI_RT_MEASURE=1` +
   `PI_RT_GATE_KEY` + injected runner/writer.
2. `pi/test/rt.test.ts` (new, 225 lines) — boundary + signature + guard tests.
3. `pi/deploy/rt-gate.md` (new, 149 lines) — UNVERIFIED bench runbook, exact
   thresholds, artifact contract, C1 consumer contract.

No other file staged or committed. ~200 pre-existing unrelated unstaged files
left untouched (count was 202 before and after). Staged lists verified before
each commit. The sdd report itself is untracked, matching prior tasks.

## Commits

| Commit    | Subject                                              | Files |
| --------- | ---------------------------------------------------- | ----- |
| `14d3478` | `feat(pi): RT measurement gate (TX still disabled)`  | 3     |
| `5eda9f8` | `fix(pi): fail closed on malformed RT gate artifact` | 2     |

Push result: `d581e67..14d3478`, then `14d3478..5eda9f8` to
`spike/raspberry-pi-portability`.

## Self-review

- Confirmed imports of `node:crypto` / `node:buffer` are deterministic; no
  clock, fs or process execution at import time.
- `decideRtGate()` returns copies and is pure; invalid inputs fail closed via
  `Number.isFinite`, non-negative and strict `<` checks.
- `verifyGateArtifact()` is timing-safe on the HMAC compare, checks schema and
  binding thresholds; `isGateOpen()` additionally re-decides measurements so a
  signed-but-lying verdict cannot unlock TX.
- Self-review finding fixed in `5eda9f8`: a signed artifact with missing
  `measurements` previously threw a `TypeError`; now returns `false`.
- `runRtMeasurement()` default runner/writer are absent (not no-ops), so
  calling it on a dev machine cannot run `cyclictest` or write a file.
- Runbook avoids ordered lists (Prettier two-space vs markdownlint MD030
  conflict) and keeps code fences unindented; Prettier output is idempotent.
- No Sentry code, no CAN TX implementation, no hardware claims in committed
  files; every board claim marked UNVERIFIED.

## Concerns / deferred

1. **Artifact contract unconfirmed** — see design-decision section. Highest-priority
   item for controller + C1.
2. **No hardware, no artifact** — by design. `pi/gate/rt-result.json` does not
   exist; a unit test asserts its absence. That test is intentionally brittle:
   C1/D1 must delete it when a real measured artifact is generated on the board.
3. **Runner/writer not implemented** — skeleton only; wiring `node:child_process`
   and an fs writer is a later task's job. No binary was run here.
4. **Transient test flake (pre-existing)** — first GREEN run hit `TypeError: fetch
failed / bad port` in `bridge.test.ts` and `smoke.test.ts` (ephemeral port
   race between parallel Jest workers). Three subsequent full Pi runs passed
   55/55. Unrelated to this change; noting for the controller's QA ledger.
5. **`buildCyclictestArgs()` includes `--smp`** — inherited from the cited
   methodology (`can-hardware.md` §3.3). Runbook additionally pins with
   `taskset -c <isolated-core>`; confirm on the board that both together still
   exercise the intended isolated-core path.
6. **Failing runs still write an artifact** with `verdict.pass: false`. This is a
   record, not an unlock; C1's `isGateOpen()` returns false for it.
