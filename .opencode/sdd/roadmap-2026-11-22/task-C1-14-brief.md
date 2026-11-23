# Task C1-14 — BLE proto test suite (all builders)

Where this fits: Phase 1 correctness. C1-01 already fixed the 5 `RoutableMessage` tags and created `firmware/test/test_native_ble_proto/test_ble_proto.cpp` with 7 tests. This task extends that suite to pin exact wire bytes for ALL BLE builders, not just the envelope.

## Requirement (verbatim from plan)

New `test_native_ble_proto` asserting exact bytes for all builders against SDK proto vectors.

- Acceptance: suite green in `env:native`.
- Evidence: only `test_native_ble_distance` exists (firmware.md §5, §7.2#10).
- Effort: 3–4 h. Risk: Low.

## Interfaces / decisions from earlier tasks

- C1-01 is complete: `msg.h` tags are now 6/10/14/15/51; its test file uses an INDEPENDENT literal tag-byte decoder (do not import production `Proto` helpers into the assertions).
- Controller ruling: because the suite already exists, EXTEND `firmware/test/test_native_ble_proto/test_ble_proto.cpp` rather than creating a duplicate suite. Keep one suite.
- SDK proto reference (read-only under `legacy/`): `legacy/teslamotors-vehicle-command/pkg/protocol/*.proto`. Do not copy code from legacy; implement independently.

## Global constraints

- No code copied from `legacy/` into shipping code.
- Every CAN/BLE wire change needs a test that pins exact bytes.
- Stage ONLY files you change; worktree has 191 unrelated unstaged changes. Verify `git diff --cached --name-only` before committing.
- Commit and push on the current feature branch; never main, never force-push.
