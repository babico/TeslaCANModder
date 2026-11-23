# Task C1-07a — Single injection-gate authority (safety hardening)

Where this fits: Phase 1 safety. Currently `dasTick` can inject without the AP gate, gear 0/5 are treated as Park, a CAN-timeout force-open exists, and gate state has no freshness bound. This task tightens the gate conservatively; it does NOT change frame layouts.

## Requirement (safety-only subset of plan C1-07, lines 65-68)

Gate `dasTick` on `apGateOpen()`; count only gear 1 as Park (0/5 fail closed); remove the CAN-timeout force-open at `main.cpp:149`; add bounded freshness (`hasCtrl`, `apGateApActive` timestamps).

- Acceptance: extend `test_native_ap_gate` with gear 0/5 cases and a `dasTick` case with gate closed → 0 sends; stale timestamp → no injection.
- Evidence: `core/types.h:837-842`, `vehicle.h:392-393,52,135`, `main.cpp:149`, `das_drive.h:414-498` (firmware.md §2.3, §7.1#5, §7.2#13); old P1-008/P9-001.
- Effort: 3–5 h. Risk: High.

## Controller ruling (binding)

- Exact gear-bit ground truth is CAPTURE_FIRST and is deferred to C1-07b. Here only apply the conservative rule: only gear 1 (Park) opens the park condition; 0 (invalid) and 5 (SNA) must NOT open it. Do not change any CAN decode bit positions.
- No new injection paths, no frame layout/checksum changes. Walk `docs/checklists/can-review-checklist.md` in the report, marking frame-shape items N/A.
- Keep any existing user-facing `das:arm` UNSUPPORTED status from C1-10 intact.

## Global constraints

- CAN gating change → checklist + report; no code copied from `legacy/`.
- Stage ONLY files you change; 191 unrelated unstaged files exist. Verify staged list before commit.
- Commit/push on the current feature branch; never main, never force-push.
