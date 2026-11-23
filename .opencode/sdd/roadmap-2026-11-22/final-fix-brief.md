# Final review fix wave — complete findings list

One implementer, one scoped re-review. Base `a446c55`. Work from repo root.

## Important 1 — `metadataForKey` placement (SDK proto contradiction)

`firmware/lib/vehicle/ble/msg.h:85`: production writes `permission.fieldMsg(6, keyMeta)`. SDK `vcsec.proto:104-108,121-136` defines `metadataForKey` as field **6 of `WhitelistOperation`**, a SIBLING of f5 — not a child of `PermissionChange` (which has fields 1/3/4). Four byte pins in `test_native_ble_proto/test_ble_proto.cpp:261,442,457,486` currently encode the wrong nesting and call it an "SDK vector".
Fix: `whitelist.fieldMsg(6, keyMeta)` and flip all four pins in the SAME commit. Add an SDK-authority test that checks the placement is a sibling of f5 (byte pin alone cannot distinguish). Keep bench confirmation as a follow-up, not as justification for non-SDK bytes. Apply the same SDK-authority ruling used for the amps `DA 02` correction.

## Important 2 — client emits UNSUPPORTED commands

`client/src/components/monitor/sections/ConnectionSection.tsx:297-459`: ~20 active buttons send `teslable:*`, `ha:*`, `espnow:*`, `gvret:*`, `smt:*`, `elm327:*` via bare `run()` with only `disabled={isBusy}`, bypassing `gating.ts:850-974`. Docs now promise `Unknown command`. Fix: gate/hide/disable those cards (or remove) so no UNSUPPORTED wire string is emitted from `client/src`. Add a test asserting that.

## Minor 3 — mixed time bases

`firmware/lib/vehicle/can/handler/bus/vehicle.h:137,142` + `core/types.h:863-865` + `main.cpp:136`: AP-active timestamp uses `millis()`, but the ALC gate call in the same handler uses cached `gateNowMs`; underflow can read gate closed (safe but inconsistent). Fix: pass `now` to that `apGateOpen()` call or stamp `apGateApActiveMs` from `gateNowMs`.

## Minor 4 — stale alias prose

`docs/guides/security.md:109`, `docs/reference/can-ids.md:187` (and generated bundle) still say `lanegraph:on` after C1-10 aliased to `lane-graph`. Fix both canonical docs, regenerate bundle in a SEPARATE commit (auto-generated policy), and extend `command-dispatch-coverage.test.ts` to scan prose alias tokens, not just table first columns.

## Minor 5 — whitespace

`firmware/test/test_native_cmd_freshness/test_cmd_freshness.cpp:37+` — 17 `space before tab` errors; `git diff --check a446c55..HEAD` must be clean after fix.

## Minor 6 — small hygiene

- `msg.h:29-30` unused `COMPONENT_*` constants after `component_id` removal.
- `key.h:28` `static` function in header → `static inline`.
- `tools/serial-http-bridge.js:50,80` `--allow-origin '*'` emitted verbatim though help implies restriction → reject `*` or correct help.
- `tools/commands/das-drive.js:169` `Number(...) || 5000` → use validated value / `??`.
- 13 feature helpers still use bare `!s.hasCtrl` (redundant downstream of the fresh guard) — leave as-is unless trivially replaceable; note in report.

## Out of scope (do not touch)

- Capture-gated: C1-07b, C1-12, C1-23; ALC bus routing; real-vehicle acceptance for BLE/ALC.
- 99 `docs/legacy/*` research docs; skill files except where listed.
- 191 pre-existing unstaged working-tree changes.

## Process

- Stage ONLY files you change with explicit `git add <paths>`; verify staged list before every commit.
- Commit/push on current feature branch; never main, never force-push.
- Generated bundle in its own commit; all other fixes in one or more hand-edit commits.
- Run: full native suite, `npm test -w packages/protocol`, `npm test -w client`, `npm test -w tools`, `npm run lint:all`, `git diff --check a446c55..HEAD`.
- Full report to `.opencode/sdd/roadmap-2026-11-22/final-fix-report.md`; no subagents.
