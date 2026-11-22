# Task C1-10 — Advertised-but-unimplemented command decision

Status: COMPLETE

## Scope

Phase 1 correctness: every command advertised in `docs/reference/commands.md` /
`packages/protocol/src/commands.ts` with no real firmware effect must get an explicit
implement / alias / delete decision. Commands kept for wire compatibility are marked
`UNSUPPORTED` in docs and enforced by a new grep-based CI test.

Evidence read: `task-C1-10-brief.md`, `cross-cutting.md` §1 D13/D14, §2.1/2.2/2.3,
`client-protocol.md` §5, §7 N1/N2, `dispatch.h:1-130,380-499,660-739,820-919,920-1086`,
`commands.ts`, `gating.ts`, `cross-check.test.ts`. Direct negative greps over `firmware/`
confirmed every NO_HANDLER wire string in the table below is absent from all firmware
`.h`/`.cpp` files (including `das:*`, which `cross-cutting.md` §2.2 missed).

## Decision table

Legend: **ALIAS** = repoint builder to canonical firmware spelling. **UNSUPPORTED** =
keep builder + wire string for compatibility but mark docs row explicitly unsupported
(gated off by existing `COMMAND_GATES` flags, all default false). **IMPLEMENT** = make an
existing handler reachable. **KEEP** = already implemented; no behavior change.

| #   | Builder(s)                                                                                                            | Wire strings                                      | Decision                        | One-line justification                                                                                                                                       |
| --- | --------------------------------------------------------------------------------------------------------------------- | ------------------------------------------------- | ------------------------------- | ------------------------------------------------------------------------------------------------------------------------------------------------------------ |
| 1   | `apFirstToggle`, `apFirstOn/Off`                                                                                      | `apfirst:on/off`                                  | **ALIAS → `ap-first:on/off`**   | Firmware already handles `ap-first` (`dispatch.h:619,628`); pure spelling drift, no new injection.                                                           |
| 2   | `laneGraph`, `laneGraphOn/Off`                                                                                        | `lanegraph:on/off`                                | **ALIAS → `lane-graph:on/off`** | Firmware already handles `lane-graph` (`dispatch.h:579,588`); pure spelling drift.                                                                           |
| 3   | `btnMap`, `btnMapQuery`, `btnMapReset`                                                                                | `btnmap:*` (8 forms)                              | **UNSUPPORTED**                 | Physical button remap needs a real handler + NVS bindings; client status fields retained for compatibility.                                                  |
| 4   | `speedAlert`                                                                                                          | `speedalert:on/off`                               | **UNSUPPORTED**                 | BLE speed-camera service does not exist; implementing it is a new BLE service, out of scope.                                                                 |
| 5   | `gvret`, `gvretPort`                                                                                                  | `gvret:on/off`, `gvret:port:N`                    | **UNSUPPORTED**                 | GVRET TCP stack absent; would require a new network subsystem (`docs/legacy` says docs/strings only).                                                        |
| 6   | `espNow`, `espNowChannel`                                                                                             | `espnow:on/off`, `espnow:channel:N`               | **UNSUPPORTED**                 | ESP-NOW radio stack absent; new radio feature, not a minimal handler.                                                                                        |
| 7   | `scanMyTesla`                                                                                                         | `smt:on/off`                                      | **UNSUPPORTED**                 | ScanMyTesla SPP bridge absent; new BLE SPP profile required.                                                                                                 |
| 8   | `elm327`                                                                                                              | `elm327:on/off`                                   | **UNSUPPORTED**                 | ELM327 AT-command emulator absent; new protocol emulator required.                                                                                           |
| 9   | `teslaBle`, `teslaBleAuth`, `teslaBleForget`                                                                          | `teslable:on/off/auth/forget`                     | **UNSUPPORTED**                 | Distinct from the implemented `tesla:*` BLE key protocol; no VCSEC dispatcher branch exists.                                                                 |
| 10  | `homeAssistant`, `haDiscovery`, `haInterval`                                                                          | `ha:on/off`, `ha:discovery`, `ha:interval:N`      | **UNSUPPORTED**                 | HA discovery/MQTT publish loop absent (cross-cutting §2.2); not a minimal handler.                                                                           |
| 11  | `bleEncrypt`, `blePair`, `bleUnpair`                                                                                  | `bleencrypt:on/off/pair/unpair`                   | **UNSUPPORTED**                 | AES-256-GCM multi-device BLE pairing absent; new security subsystem required.                                                                                |
| 12  | `dasArm`, `dasDisarm`, `dasStatus`                                                                                    | `das:arm/disarm/status`                           | **UNSUPPORTED**                 | No `das:` literal anywhere in firmware (direct negative grep); DAS gate is driven by `gamepad:*` / `drive:*`.                                                |
| 13  | `log`, `platform`, `recorder`, `recorderClear`, `recorderStatus`                                                      | `log`, `platform`, `recorder:on/off/clear/status` | **IMPLEMENT (gate fix)**        | Handlers exist (`dispatch.h:679,836,872`) but sit below the `!s.hasCtrl` early return (`:422-427`); move above it — no CAN mutation.                         |
| 14  | `ble`, `bleStatus`, `bleDistance*`, `gamepad*`, `teslaKey*`, `teslaWake`, `teslaCharge*`, `teslaClimate*`, `teslaVin` | ~29 BLE strings                                   | **KEEP (build-gated)**          | Real handlers exist inside `#if BOARD_ENABLE_BLE` (`dispatch.h:909-1083`); non-BLE builds reject them by design and docs already frame them as BLE features. |

Safety: no CAN-frame mutation, checksum, routing, or transport-shape change. Decision 13
only moves read-only/telemetry handlers above a state gate. Can-review checklist not required.

## Implementation plan

1. Write grep-based coverage test + fresh-boot ordering test TDD (RED expected).
2. Alias two builder families (protocol + standalone builders + docs + cross-check list; delete the `apfirst`/`lanegraph` whitelist).
3. Mark the 11 UNSUPPORTED families' doc rows with an explicit `**UNSUPPORTED**` marker + a legend section.
4. Move `log`, `platform`, `recorder:*` above `!s.hasCtrl` in `dispatch.h`.
5. Run suites; commit/push; self-review.

## TDD log

**RED** — `node --experimental-vm-modules ../../node_modules/jest/bin/jest.js --config
jest.config.js --runInBand test/integration/command-dispatch-coverage.test.ts` before any
source change: 4 failed / 3 passed. Coverage test listed 40 undocumented-without-dispatch
rows (the 4 `apfirst`/`lanegraph` spellings plus 36 UNSUPPORTED-family rows; direct grep also
caught `das:arm/disarm/status`, missing from `cross-cutting.md` §2.2). Fresh-boot test failed
for `log`, `platform`, `recorder:*` (`beforeHasCtrl: false`).

**GREEN** — same command after implementation: 7/7 passed.

Full verification (each run once, all green):

- `npm run test:protocol` — 27 suites, 501 tests passed (includes updated `cross-check.test.ts`).
- `npm run test:client` — 45 suites, 305 tests passed.
- `npm run test:firmware` / `.\firmware\pio.ps1 test -e native` — 1053 native cases passed, exit 0.
- `npm run lint:all` — ESLint clean, Prettier clean, markdownlint 0 issues / 44 files.

## Files changed

- `.opencode/sdd/roadmap-2026-11-22/task-C1-10-report.md` (new — this report)
- `packages/protocol/test/integration/command-dispatch-coverage.test.ts` (new — grep guard + fresh-boot ordering)
- `packages/protocol/src/commands.ts` (alias builders + standalone builders)
- `packages/protocol/test/integration/cross-check.test.ts` (canonical spellings; removed `apfirst`/`lanegraph` whitelist)
- `docs/reference/commands.md` (alias rows, `## Unsupported commands` legend, 35 UNSUPPORTED row markers)
- `firmware/lib/client/command/dispatch.h` (moved `log`, `platform`, `recorder:*` above `!s.hasCtrl`)

Staged explicitly by path; no `git add -A`/`.`; the 199+ pre-existing unstaged files untouched.

## Self-review

- Decision table covers every advertised command with no firmware effect; no command left undecided.
- No CAN frame mutation, checksum, routing, or transport-shape change → can-review checklist not applicable.
- Gate move is a pure relocation of three read-only/telemetry branches; all three sit at line 425/448/484, before the `!s.hasCtrl` return at 526.
- Alias change is protocol/docs only; firmware canonical spellings already existed and are unchanged.
- Grep guard parses firmware source at test time (not a hardcoded list) and now has no whitelist bypass for the known alias drift.
- `cross-check.test.ts` hardcoded list updated in lockstep with builders/docs; all four layers agree.
- Verified `git diff --cached --name-only` contains only the six paths above before commit.

## Concerns

1. UNSUPPORTED families keep their builders, gating entries, and status field definitions for wire compatibility. Client panels still display those status fields, and `ConnectionSection.tsx` still sends raw `gvret:*` / `espnow:*` strings that hit `Unknown command`. Docs are now honest; UI cleanup is a separate task (no new behavior introduced here).
2. `client/src/docs/_generated-docs.ts` is an auto-generated snapshot of `docs/`; it was not regenerated, so the in-app docs still show pre-change rows until the generator is run. Generated diffs were deliberately excluded.
3. The fresh-boot test is a deterministic source-order guard rather than a compiled `executeCommand()` call. A native dispatch harness needs an `Arduino.h` stub plus broad driver/output stubs (no existing native test includes `dispatch.h`); the ordering assertion directly enforces the D14 regression without that scaffolding.
4. `das:arm/disarm/status` is a real gap not recorded in `cross-cutting.md` §2.2; included here on direct negative grep evidence.
5. Build-artifact/temp files from verification live under `firmware/.pio*`, `$TEMP` only; nothing scratch left in the repo tree.
