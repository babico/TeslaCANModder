# Task C1-03 — BLE roles and form factor

Where this fits: Phase 1 correctness/security. Role constants and NFC form-factor value are wrong, so provisioned keys get the wrong permissions and add-key wire bytes do not match the SDK.

## Requirement (verbatim from plan, lines 49-52)

OWNER=2, CHARGING_MANAGER=6, NFC_CARD=1, and align `roleValue()` mapping.

- Acceptance: role-value test asserts `"owner"`→2, `"charging_manager"`→6; encoded bytes match proto.
- Evidence: `key.h:22-23`, `msg.h:25`, `tesla_board.h:124-127`; `keys.proto` values quoted in `docs/legacy/tesla-local-control-vehicle-command.md:56,235` (firmware.md §7.1#4).
- Effort: 4–6 h (shared with C1-02). Risk: High.

## Interfaces / decisions from earlier tasks

- C1-01 fixed RoutableMessage tags; C1-02 rebuilt add-key nesting 16→5→PermissionChange{key=1, keyRole=4}. Do not change either.
- C1-02 residual (carry into this task): production dispatch still passes `KEY_FORM_FACTOR_NFC_CARD = 3`; SDK says 1. C1-02's byte test used a test-only formFactor=1 constant. Align production and re-pin any affected assertion in `firmware/test/test_native_ble_proto/test_ble_proto.cpp`.
- No bench capture is possible here; report wire-level proof from byte tests and mark bench verification UNVERIFIED.
- `metadataForKey` placement discrepancy from C1-02 is a separate bench follow-up — do not change it here.

## Global constraints

- No code copied from `legacy/` into shipping code; independent implementation from proto values.
- Every BLE wire change needs a byte-pinning test.
- Stage ONLY files you change; worktree has 191 unrelated unstaged changes. Verify `git diff --cached --name-only` before committing.
- Commit and push on the current feature branch; never main, never force-push.
