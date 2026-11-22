# Task C1-06 report — OTA TX guard decision (removal branch)

Status: **DONE_WITH_CONCERNS**

Branch: `chore/legacy-tesla-drug-fork-submodules`
Commit: `11b21a1 docs(protocol): retire advertised OTA TX guard (C1-06)`
Report commit: separate follow-up commit on same branch.

Controller ruling followed verbatim: no real capture exists and the identity of `0x318`
is disputed, so the **removal** branch of the acceptance criterion was taken. No TX gate,
no frame decode, and no new state field was added.

## Removed claims — complete list

Every removed claim described behavior the firmware does not implement. Locations are
pre-change line numbers.

### `docs/troubleshooting/debug-guide.md`

- `## OTA Paused TX` section (4 bullets): commands stop working during OTA is expected;
  board auto-detects OTA from `0x318` and pauses all TX; TX resumes automatically;
  dashboard shows "OTA in progress". Section deleted.

### `docs/guides/security.md`

- `## OTA Safety` section: "only automatic safety gate is OTA detection", `0x318`,
  "all CAN TX is paused (`txPaused = true`)", claimed to protect the board during updates.
  Replaced with an accurate `## Vehicle Software Updates` note: no automatic TX pause,
  `0x318` identity unresolved (C14), disable injection manually before an update.

### `docs/reference/vehicle-features.md`

- `## OTA Safety Check` section: auto-pauses all injection, monitors `0x318`
  (`GTW_carState`), 3 bullets including "No manual intervention needed — fully automatic".
- Seatbelt Emulation bullet: "Automatically pauses during OTA updates".
- Variant feature matrix row: `| OTA Safety | ✅ | ✅ | ❌ |`.

### `docs/reference/state-fields.md`

- Heading `## OTA & Hardware Detection` → `## Hardware Detection`.
- `otaInProgress` row ("Tesla OTA detected (0x318)").
- `txPaused` row ("All TX paused during OTA").

### `docs/reference/can-protocol.md`

- CAN ID table row 792 `0x318`: "GTW_carState (OTA detection)" → "GTW_carState (raw ID;
  identity unresolved)".
- `## OTA Detection` section: "monitored for OTA update status ... all CAN TX injection
  automatically paused until the update completes".
- Frame Routing item 5: "**OTA Guard:** TX automatically paused when `0x318` indicates
  OTA in progress".

### `docs/reference/can-ids.md`

- Mermaid bus-1 label "BMS / Gateway / OTA" → "BMS / Gateway".
- Bus topology row "Drive, BMS, body controllers, OTA" → "Drive, BMS, body controllers".
- `0x318` ID row Notes "OTA detection" → "Raw ID; identity unresolved".

### `docs/reference/connections.md`

- Validation bullet "You are not in OTA pause state.".

### `docs/architecture/feature-workflows.md`

- `### Transmission Safety` section and its 7-row `txPaused` effects table (handlers pass
  through, `startBurst()` refuses, `burstTick()` cancels, `summonTick()` cancels,
  `preconditionTick()` returns, `nagShouldEcho()` returns false).
- Five `!txPaused` gate lines: nag/0x370 echo, `summonTick`, `preconditionTick`,
  `driveModeTick`, `dasTick`. Gates reduced to their real conditions.
- `## 53. OTA Safety (Read-Only, Auto-Pause)` section: `CAN_ID_GTW_CAR_STATE (0x318)`
  `byte[6] bits[1:0] = GTW_updateInProgress`, `s.otaInProgress` / `s.txPaused` assignment,
  and "txPaused gates ALL transmission".
- Pattern Compliance Matrix row 53 `ota safety`.

### `firmware/lib/io/schemas/io.schema.json`

- `otaInProgress` + `txPaused` properties and `required` entries in all three definitions:
  `StatusState` (2 properties + 2 required), `BootStatusState` (2 + 2),
  `StatusStatePayload` (2 + 2). 12 lines removed; trailing commas fixed; JSON reparsed OK.

### `packages/protocol`

- No change needed. Exact-symbol greps (`otaInProgress`, `txPaused`, `txSuppressed`,
  `'paused'` status) found no OTA state field in `src/` or tests. The old embedded
  removal plan that mentions these names exists only in the generated client docs bundle,
  not in protocol source.

## Deliberately kept (not advertised behavior)

- `firmware/lib/vehicle/can/ids.h:52` — `#define CAN_ID_GTW_CAR_STATE 0x318 // 792 — GTW_carState (RX)` unchanged.
- `0x318` kept in `docs/reference/can-protocol.md` CAN ID table, Frame Routing RX list,
  and `docs/reference/can-ids.md` ID table as a raw, unresolved ID.
- `docs/reference/commands.md:131` and `docs/reference/vehicle-features.md` Ban Shield
  "Unusual OTA/telemetry frame burst" text — threat-pattern detection, not a TX-pause claim.
- `docs/legacy/**` research reports — read-only audit records that _cite_ our prior OTA
  claim as evidence of a docs-vs-code discrepancy; not product advertising.

## Contract test (TDD)

Added `packages/protocol/test/integration/ota-guard-claim.test.ts` following the
`cross-check.test.ts` / `command-dispatch-coverage.test.ts` filesystem-guard pattern.
It walks all non-legacy `docs/**/*.md` plus `io.schema.json` and asserts 12 forbidden
claim patterns are absent (state symbols, named OTA-guard headings, pause/resume prose,
`GTW_updateInProgress`), asserts no OTA state fields in `packages/protocol/src`, and pins
the raw `CAN_ID_GTW_CAR_STATE` define. Legacy docs are excluded because they are research
records, not advertised behavior.

RED (test added, claims still present):

```
Test Suites: 1 failed, 1 total
Tests:       9 failed, 29 passed, 38 total
```

Failing targets: `docs/architecture/feature-workflows.md`, `docs/guides/security.md`,
`docs/reference/can-ids.md`, `docs/reference/can-protocol.md`,
`docs/reference/connections.md`, `docs/reference/state-fields.md`,
`docs/reference/vehicle-features.md`, `docs/troubleshooting/debug-guide.md`,
`firmware/lib/io/schemas/io.schema.json`.

GREEN after removals (also re-run after the lint-staged hook):

```
Test Suites: 1 passed, 1 total
Tests:       38 passed, 38 total
```

## CAPTURE_FIRST rationale and disputed evidence (cross-cutting C14)

Real implementation is **CAPTURE_FIRST**. The frame identity itself is unresolved, so a
decode would either pause TX on the wrong frame or fail to pause during a real update —
both are worse than the current honest absence of a guard.

Binding analysis rows:

- `.opencode/analysis/current-state-2026-11-22/cross-cutting.md:176` — C14: ours claims
  "GTW_carState / OTA guard (`can-protocol.md:75,201`)" vs upstream "RTC timestamp";
  ruling **CAPTURE_FIRST**; evidence `hanswolff:351`, `joshwardell:387`.
- `.opencode/analysis/current-state-2026-11-22.md:201` — same C14 row: "GTW_carState /
  OTA guard" vs "RTC timestamp".
- `.opencode/analysis/current-state-2026-11-22.md:213` — "C14 means the very ID used for
  the advertised OTA guard may not be GTW_carState — the guard decision must be
  capture-first, not doc-first."
- `.opencode/analysis/current-state-2026-11-22/firmware.md:92` — CODE-READ (negative grep):
  no `.h` file in `firmware/lib` references `otaInProgress` or `txPaused`; `State`
  (`core/types.h`) has neither field; no handler for `0x318`. The guard never existed in
  code; only the docs/schema advertised it.

Evidence that `0x318` is **not** GTW_carState on some platforms:

- `docs/legacy/hanswolff-TeslaCanBusInspector.md:148-157,351` — Model 3 `0x318` decoded as
  RTC date/time: `payload[0]`=year−2000, `[1]`=month, `[2]`=second, `[3]`=hour, `[4]`=day,
  `[5]`=minute UTC; guards month≠0, 0<day≤31; validity floor 2000-01-01. Explicitly
  notes "Both meanings cannot be right for the same vehicle era — capture-first".
- `docs/legacy/joshwardell-model3dbc.md:265,387` — `Model3CAN.dbc:2138-2144` names
  `0x318` `SystemTimeUTC`; P47 records "Same ID, different content claim / capture-first"
  against our `CAN_ID_GTW_CAR_STATE` OTA-detect reservation.
- `docs/legacy/bobmorane83-TeslaCam.md:261-263,313` — `0x318` as UTC fields (year offset,
  month, hour, day, minute) in another firmware; CODE-READ.
- `docs/legacy/ColinM-sys-tesla-can-boost.md:78,268,294` — DBC `ID318SystemTimeUTC`;
  upstream's `0x318` speed decode is CONTRADICTED; speed is on `0x155`.
- `docs/legacy/automotive-stuff-Tesla_canbus.md:310` — `0x318` decoded as cabin/AC
  temperatures (`/2 −40 °C`) on another platform; CODE-READ.

Evidence for the guard side is upstream CODE-READ only, not a Tesla capture:

- `docs/legacy/J0811-flipper-tesla-fsd.md:168-178` — `0x318` `buffer[6] & 0x03`,
  nonzero = OTA; "byte/bit position needs capture-first validation".
- `docs/legacy/hypery11-flipper-tesla-fsd.md:181-190,517` — `byte6[1:0]`, raw==2
  installing, debounce 3 assert / 6 clear; "capture-first port (top 2 candidate)".
- `docs/legacy/enstw-tesla-can-mod-guide.md:129-139,410,450,551` — `data[6] bits[1:0]`
  raw==2; labelled "candidate for future PR", "UNVERIFIED", and "best-effort,
  capture-first" because unvalidated decodes could leave the gate false during a real OTA.
- `docs/legacy/canhackers-jupiter.md:380`, `docs/legacy/dongho74s-tesla-open-can-mod.md:325`
  — upstream has no OTA guard at all.

## Capture needed to settle C14

1. Listen-only X179 bench capture of `0x318` (DLC, all 8 bytes, timestamp) on at least one
   modern vehicle per affected model/era — e.g. Model 3/Y HW4 2026.x and one S/X or older
   era — with model, HW gen and gateway SW version recorded alongside.
2. Determine which payload family is actually present: RTC date/time, temperature, or a
   GTW state frame. If a GTW state, identify the exact byte/bit expressing update status
   and enumerate its values against a real, time-boxed OTA install window.
3. Only then: walk `docs/checklists/can-review-checklist.md`, add the decode with DLC
   guard, central TX gate, and a native test feeding the captured frame and asserting
   burst / drive-mode TX count == 0 (the implement branch of C1-06's acceptance).

## Verification (run once before commit)

| Command                                                               | Result                                                                                                        |
| --------------------------------------------------------------------- | ------------------------------------------------------------------------------------------------------------- |
| `npm test -w @teslacanmodder/protocol`                                | 27 suites pass; 1 suite fails — 4 pre-existing failures in `command-dispatch-coverage.test.ts` (see concerns) |
| `npm run lint:all`                                                    | PASS — ESLint clean, Prettier clean, markdownlint 0 issues in 44 files                                        |
| `npm run validate:serial-contract`                                    | PASS — "Validated unified io.schema.json contract."                                                           |
| `npm test -w @teslacanmodder/protocol -- --runInBand ota-guard-claim` | PASS — 38/38                                                                                                  |
| `node -e "JSON.parse(...io.schema.json)"`                             | PASS                                                                                                          |

## Staging discipline

191+ unrelated unstaged files present at start. Staged only the 10 files in commit
`11b21a1` via explicit `git add <paths>`; `git diff --cached --name-only` verified before
commit. Not staged: `packages/protocol/.gitignore` (pre-existing modification),
`docs/superpowers/todo.md` (pre-existing untracked), `client/src/docs/_generated-docs.ts`.

## Concerns

1. **Pre-existing protocol test failures, not caused by this change.** Full protocol run
   has 4 failures in `packages/protocol/test/integration/command-dispatch-coverage.test.ts`
   (`fresh-boot commands must work without 0x273 CAN traffic`: `platform`, `recorder:*`,
   and 2 others report `beforeHasCtrl: false`). Both that test and
   `firmware/lib/client/command/dispatch.h` are clean vs HEAD (`git status --short` shows
   no modification); the failing assertions read dispatch ordering changed by C1-07a
   (`d94184d`). This change touches only docs, the schema, and a new test file. Controller
   should route the dispatch-ordering regression to the C1-07a owner.
2. **Two agent skills still advertise the removed gate (out of enumerated scope, left
   unchanged for controller ruling):**
   `.opencode/skills/can-protocol-safety/SKILL.md:78-87` ("`txPaused` gates ALL
   transmission paths ..." + 6-row effects list) and
   `.opencode/skills/firmware-development/SKILL.md:137-139` ("`txPaused` gates ALL
   transmission paths when OTA update is detected."). They are durable instructions, not
   product docs; recommend a follow-up edit so future agents are not told a non-existent
   safety mechanism exists. The contract test does not scan `.opencode/skills/`.
3. **`client/src/docs/_generated-docs.ts` is a generated mirror of `docs/`** (generated by
   `scripts/generate-docs.ts`) and still contains the old OTA text, including a stale
   embedded removal plan. Per `skip-auto-generated-files`, it was not hand-edited or
   committed; a `generate:docs` regen will clear it. The contract test intentionally
   targets the source `docs/` files, not the generated bundle.
4. **Legacy research docs under `docs/legacy/` still cite the old claim** as evidence of a
   docs-vs-code discrepancy. Deliberately excluded from removal and from the contract
   test to preserve the audit trail; they are not user-facing advertised behavior.
