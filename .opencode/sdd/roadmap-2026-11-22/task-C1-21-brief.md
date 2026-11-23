# Task C1-21 — Protocol suite regression from C1-07a

Where this fits: Phase 1 regression repair. After C1-07a introduced the single gate API (`hasFreshCtrl(now)` / `apGateOpen(now)`), `packages/protocol/test/integration/command-dispatch-coverage.test.ts` has 4 failures. C1-06 review confirmed these are not caused by the OTA claim removal.

## Requirement (verbatim from plan C1-21)

Fix the 4 `command-dispatch-coverage.test.ts` failures after the `hasFreshCtrl(now)` dispatch change.

- Acceptance: `npm test -w packages/protocol` green; ordering/guard assertions updated to the new single-gate API.
- Evidence: C1-07a changed `firmware/lib/client/command/dispatch.h` (`!s.hasCtrl` → `!s.hasFreshCtrl(now)`) and `core/types.h` freshness helpers; C1-10 added the grep-guard test file.

## Constraints / decisions

- Update the TEST to the new gate API; do NOT weaken or remove the coverage guard, and do NOT revert the C1-07a firmware change.
- The guard must still catch a command advertised in docs but missing a dispatch branch, and must still reject stale UNSUPPORTED markers.
- If a test assumption is genuinely obsolete (e.g. source-order marker expectations), document why in the report before changing it.

## Global constraints

- Stage ONLY files you change; 191 unrelated unstaged files exist. Verify staged list before commit.
- Commit/push on the current feature branch; never main, never force-push.
- Protocol/test files only — no firmware behavior change.
