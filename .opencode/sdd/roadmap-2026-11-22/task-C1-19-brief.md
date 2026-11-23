# Task C1-19 — native dispatch harness for fresh-boot commands

Where this fits: Phase 1 test-quality follow-up from C1-10. The fresh-boot test is currently a source-order proxy; make it exercise the real command path.

## Requirement (verbatim from plan C1-19)

Replace the source-order proxy in `command-dispatch-coverage.test.ts:157-178` with a native test that calls `executeCommand` with `hasCtrl=false` and asserts `log` / `platform` / `recorder:status` JSON.

- Acceptance: native suite green; keep ordering assertion as secondary.

## Interfaces / decisions from earlier tasks

- C1-07a introduced `State::hasFreshCtrl(now)` and `apGateOpen(now)` as the single gate API; `dispatch.h` guard is now `!s.hasFreshCtrl(now)` (no `apGateOpen()` on one-shot commands — deliberate).
- C1-21 updated the protocol guard marker to `hasFreshCtrl(now)`; keep that test green.
- C1-10 moved `log` / `platform` / `recorder:*` above the gate; preserve that behavior.

## Constraints

- Native test/framework files only (`firmware/test/**`, optionally a test support shim) + the protocol test file if needed. No production behavior change.
- Do NOT remove the ordering assertion; keep it as a secondary check.
- Stage ONLY files you change; 191 unrelated unstaged files exist. Verify staged list before commit.
- Commit/push on the current feature branch; never main, never force-push.
