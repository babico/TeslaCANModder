# Task C1-02 — BLE add-key rebuild

Where this fits: Phase 1 correctness/security. Our add-key message uses a flat `UnsignedMessage.field 2`; the SDK expects a nested WhitelistOperation path. Broken key provisioning means vehicle authorization cannot be set correctly.

## Requirement (verbatim from plan, lines 45-48)

Replace flat `UnsignedMessage.field 2` with `WhitelistOperation=16 → addKeyToWhitelistAndAddPermissions=5 → PermissionChange{key=1 PublicKeyRaw=1, keyRole=4}`.

- Acceptance: wire-byte test for the nested path plus a bench capture of an accepted add-key.
- Evidence: `msg.h:63-95`; `vcsec.proto:104-108,121-127,43-45` (firmware.md §7.1#3).
- Effort: 4–6 h. Risk: High.

## Interfaces / decisions from earlier tasks

- C1-01 corrected `RoutableMessage` tags (6/10/14/15/51). Do not change them.
- C1-14 created 18 byte-pinning tests in `firmware/test/test_native_ble_proto/test_ble_proto.cpp`. `test_add_key_exact_wire` is marked `CHARACTERIZATION` and carries the SDK-correct literal vector in a comment. **Flip that assertion to the SDK vector in the SAME commit as this production fix** (the recorded vector uses UnsignedMessage field 16 → WhitelistOperation field 5; implementer refinement recorded in the ledger).
- C1-03 (roles: OWNER=2, CHARGING_MANAGER=6, NFC_CARD=1) is a separate follow-up task — do NOT change role constants here; use the existing `keyRole` value for now and note the dependency in your report.
- Bench capture of an accepted add-key cannot be produced in this environment. Report that acceptance criterion as UNVERIFIED; the byte test is the in-repo proof.

## Global constraints

- No code copied from `legacy/` into shipping code; implement independently from proto field numbers.
- Every BLE wire change needs a test that pins exact bytes.
- Stage ONLY files you change; worktree has 191 unrelated unstaged changes. Verify `git diff --cached --name-only` before committing.
- Commit and push on the current feature branch; never main, never force-push.
