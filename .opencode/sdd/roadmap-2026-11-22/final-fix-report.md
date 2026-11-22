# Final fix wave — implementation report

Base `a446c55`. Branch `chore/legacy-tesla-drug-fork-submodules`. Single implementer, no subagents.

## Commits

| Commit    | Scope                                        |
| --------- | -------------------------------------------- |
| `cf35cae` | All hand-edit fixes (12 files)               |
| `7b68cbd` | Generated docs bundle only (1 file, 2 lines) |

Generated bundle is its own commit per auto-generated-file policy. Staged list was verified before each
commit; no `git add -A`/`.`; pre-existing working-tree changes were not staged.

## Per-finding status

### Important 1 — `metadataForKey` placement (SDK proto contradiction): FIXED

- `firmware/lib/vehicle/ble/msg.h`: production now writes `whitelist.fieldMsg(6, keyMeta)` — a
  sibling of f5 inside `WhitelistOperation`, per SDK `vcsec.proto:104-108,121-136`. Removed the
  wrong `permission.fieldMsg(6, keyMeta)`. Updated the inline comment to state the SDK authority.
- Four byte pins in `firmware/test/test_native_ble_proto/test_ble_proto.cpp` flipped in the same
  commit: f5 length `0x4B` (75) → `0x47` (71) in all three vectors; metadataForKey `f6` re-anchored
  as sibling of f5 in comments. Outer lengths unchanged (WhitelistOperation still 77,
  UnsignedMessage 80, full wire 105).
- Added SDK-authority test `test_add_key_metadata_is_whitelist_sibling_of_f5`: it parses
  RoutableMessage → UnsignedMessage(f10) → WhitelistOperation(f16) → f5/f6, then asserts f6 exists
  at the WhitelistOperation level and does NOT exist inside PermissionChange (which has only f1/f4).
  This distinguishes nesting that a flat byte pin cannot.
- TDD: RED before fix — native suite reported `test_add_key_metadata_is_whitelist_sibling_of_f5:
Expected Non-NULL [FAILED]` at `test_ble_proto.cpp:406`; GREEN after fix — `test_native_ble_proto
PASSED`.
- Ruling mirrors the amps `DA 02` correction: SDK proto is authority over recorded capture. Bench
  confirmation remains a follow-up, not justification for non-SDK bytes.

### Important 2 — client emits UNSUPPORTED commands: FIXED

- `client/src/components/monitor/sections/ConnectionSection.tsx`: removed the five Integration
  Settings cards that emitted `teslable:*`, `ha:*`, `espnow:*`, `gvret:*`, `smt:*`, `elm327:*`
  (17 wire strings), their dead state vars and status pills. MQTT card kept — firmware accepts
  MQTT config commands (docs: config only, no publish loop), so it is not UNSUPPORTED.
- Added source guard in `packages/protocol/test/integration/command-dispatch-coverage.test.ts`:
  derives UNSUPPORTED prefixes from `commands.md` rows, scans `client/src` (excluding the generated
  docs bundle) for string literals beginning with those prefixes, asserts zero offenders.
- TDD: RED before fix — protocol suite 3 failed / 577 passed; GREEN after — 580 passed.

### Minor 3 — mixed time bases: FIXED

- `firmware/lib/vehicle/can/handler/bus/vehicle.h` 0x39B handler: one handler-local `now = millis()`
  now stamps `apGateApActiveMs` and is passed to `s.apGateOpen(now)`; no mixing with loop-cached
  `gateNowMs`.
- Added source guard in `command-dispatch-coverage.test.ts` pinning both `s.apGateApActiveMs = now;`
  and `s.apGateOpen(now)`. TDD: RED (3 failed suite) → GREEN.
- Note: guard is structural (source-level), not a runtime harness — the vehicle bus handler needs
  the full decoder/driver dependency set not wired into any native TU.

### Minor 4 — stale alias prose: FIXED

- `docs/guides/security.md:109`, `docs/reference/can-ids.md:187`: `lanegraph:on` → `lane-graph:on`.
- Extended `command-dispatch-coverage.test.ts` with a prose-alias guard: scans backtick tokens in
  canonical docs (excluding table rows handled by the existing first-column guard) for deprecated
  `apfirst:` / `lanegraph:` prefixes. TDD: RED → GREEN.
- Generated bundle regenerated in separate commit `7b68cbd`; diff is exactly the two mirrored lines.

### Minor 5 — whitespace: FIXED

- `firmware/test/test_native_cmd_freshness/test_cmd_freshness.cpp`: 17 `space before tab` lines
  cleaned (lines 37–51, 63–64). `git diff --check a446c55..HEAD` is clean.

### Minor 6 — small hygiene: FIXED

- `msg.h`: removed unused `COMPONENT_INFOTAINMENT` / `COMPONENT_VEHICLE_SECURITY`.
- `key.h`: `static roleValueFromName` → `static inline`.
- `tools/serial-http-bridge.js`: `--allow-origin '*'` now throws "wildcard CORS origin is not
  allowed" instead of being emitted verbatim; help text updated. Added test `rejects a wildcard
CORS origin instead of emitting it verbatim` in `tools/test/bridge.test.js` (RED → GREEN).
- `tools/commands/das-drive.js`: soak duration now `Number.isFinite(requested) && requested > 0 ?
requested : 5000` — no falsy `|| 5000` coercion.
- 13 feature helpers still using bare `!s.hasCtrl`: LEFT AS-IS. They are redundant downstream of
  the freshness guard but replacement would be mechanical churn with no behavior change; brief
  permits leaving them.

## Out of scope — untouched

Capture-gated items (C1-07b, C1-12, C1-23, ALC bus routing, real-vehicle acceptance), 99
`docs/legacy/*` research docs, skill files, 191 pre-existing unstaged working-tree changes,
`.opencode/sdd/.../progress.md`.

## Verification (final state, after both commits)

Commands run from repo root unless noted.

| Command                                               | Result                                                                                                           |
| ----------------------------------------------------- | ---------------------------------------------------------------------------------------------------------------- |
| `cd firmware && .\pio.ps1 test -e native`             | `1076 test cases: 1076 succeeded in 00:01:07.693`; all 87 suites PASSED (incl. `test_native_ble_proto`)          |
| `npm test -w @teslacanmodder/protocol -- --runInBand` | Test Suites 28 passed/28; Tests 580 passed/580                                                                   |
| `npm test -w @teslacanmodder/tools -- --runInBand`    | Test Suites 15 passed/15; Tests 164 passed/164                                                                   |
| `npm run test:client`                                 | Test Suites 45 passed/45; Tests 305 passed/305                                                                   |
| `npm run typecheck:client`                            | exit 0                                                                                                           |
| `npm run lint:all`                                    | ESLint clean; Prettier "All matched files use Prettier code style!"; markdownlint "Summary: 0 issues in 0 files" |
| `git diff --check a446c55..HEAD`                      | exit 0, no output                                                                                                |

RED evidence (before fixes):

- native: `1077 test cases: 1 failed, 1075 succeeded`; `test_native_ble_proto ERRORED`; failing test
  `test_add_key_metadata_is_whitelist_sibling_of_f5: Expected Non-NULL [FAILED]`.
- protocol: `Test Suites: 1 failed, 27 passed, 28 total`; `Tests: 3 failed, 577 passed, 580 total`
  (prose alias, client UNSUPPORTED, time-base guards).
- tools: `Test Suites: 1 failed, 14 passed, 15 total`; `Tests: 1 failed, 163 passed, 164 total`
  (wildcard CORS).

## Concerns / follow-ups

- BLE add-key wire shape is now SDK-authoritative but unverified on a real vehicle; bench
  confirmation is a separate capture-gated task.
- Time-base regression guard is source-level, not runtime; a native harness for the vehicle bus
  handler would strengthen it.
- UNSUPPORTED cards were removed rather than hidden/gated — permitted by the brief; protocol
  builders for those commands still exist in `packages/protocol` (out of scope for `client/src`
  emission ban).
