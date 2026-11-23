# Task C1-01 — BLE `RoutableMessage` field numbers

Where this fits: Phase 1 correctness/security. Our BLE envelope uses wrong protobuf field tags, so vehicle commands and session info cannot be encoded/decoded reliably.

## Requirement (verbatim from plan)

Set `to_destination=6`, `protobuf_message_as_bytes=10`, `session_info_request=14`, parse `session_info=15`, `uuid=51`.

- Acceptance: new `test_native_ble_proto` asserts exact field tags 6/10/14/15/51 against SDK proto vectors.
- Evidence: `msg.h:90-92,131-133,312-314,364-368` vs `universal_message.proto:84-99` (firmware.md §3.2, §7.1#2).
- Effort: 2–4 h. Risk: High.

## Interfaces / decisions from earlier tasks

None (first BLE task). Later tasks C1-02/03/14 consume the corrected tags; do not change add-key nesting or role constants here.

## Global constraints

- No code copied from `legacy/` into shipping code; independent implementation based on proto field numbers.
- Every CAN/BLE wire change needs a test that pins exact bytes.
- Stage ONLY the files you change; the worktree has 191 unrelated unstaged files. Verify `git diff --cached --name-only` before committing.
- Commit and push on the current feature branch; never push to main; never force-push.
