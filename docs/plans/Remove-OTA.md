---
plan name: Remove-OTA
plan description: Remove OTA detection
plan status: active
---

## Idea

Remove all OTA detection, txPaused gating, and related state fields from firmware, protocol package, client tests, and docs. LCD is already absent from the codebase (only exists in legacy repos).

## Implementation

-   1. Remove `otaInProgress` and `txPaused` fields from `firmware/lib/core/types.h` State struct (lines 533-535) and their constructor initialization (line 742). Remove `s.txPaused` checks from `firmware/lib/vehicle/can/handler/ticks.h` (summonTick, preconditionTick, burstTick) — replace with `false` so TX gates always pass. Update doc comments referencing OTA.
-   2. Delete the OTA detection block in `firmware/lib/vehicle/can/handler/bus/vehicle.h` (lines 466-498). Update the file doc comment (line 40) to remove 'OTA detection'. Remove `s.otaInProgress` and `s.txPaused` references in `handleGtwShield` and `handleTlssc` calls — replace `!s.txPaused` with `true`.
-   3. Remove `s.otaInProgress` check from `firmware/lib/vehicle/can/feature/safety/ban_shield.h` (line 101-103) and update the doc comment (line 91). Remove OTA comments from variant handler files (`legacy.h`, `hw3.h`, `hw4.h`) under `firmware/lib/vehicle/can/handler/variant/`.
-   4. Remove `otaInProgress` and `txPaused` from `packages/protocol/src/types.ts`: StatusStatePayload (lines 52-53), BootMessage (lines 114-115), StatusMessage (lines 319-320), StatusStateMessage (lines 421-422), and BoardState (lines 679-680).
-   5. Remove `otaInProgress` and `txPaused` from `packages/protocol/src/reducer.ts`: initialState (lines 136-137), applyBoot (lines 434-436), applyStatus (lines 655-657), and applyStatusState (lines 1017-1028).
-   6. Remove `otaInProgress` and `txPaused` from `packages/protocol/src/parser.ts` normalizeStateSection (lines 91-92). Update `packages/protocol/src/selectors.ts` selectConnectionSummary — remove `state.otaInProgress` from txSuppressed calculation (line 46), keep only `state.txPaused` which will be removed in next step.
-   7. Since txPaused is also being removed, update `packages/protocol/src/selectors.ts` selectConnectionSummary: remove `txSuppressed` variable entirely, remove `status = 'paused'` branch, update return object to remove `txSuppressed`. Update ConnectionSummary interface to remove `txSuppressed` and remove `'paused'` from status union.
-   8. Update `packages/protocol/src/decoder.ts` line 83: change `'792: "GTW_carState (OTA)"'` to `'792: "GTW_carState"'`.
-   9. Update test fixtures: remove `otaInProgress: false` and `txPaused: false` from `packages/protocol/test/parser/parser.test.ts` (line 145), `packages/protocol/test/reducer/config-reducer.test.ts` (line 102), `packages/protocol/test/reducer/body-feature-reducer.test.ts` (line 92), `packages/protocol/test/integration/cross-check.test.ts` (search for otaInProgress), `client/tests/screens/screens.test.tsx` (line 54), `client/tests/screens/ControlsScreen.test.tsx` (line 58).
-   10. Run verification: `npm run test:protocol`, `npm run test:client`, `npm run typecheck:protocol`, `npm run typecheck:client`, `npm run lint:all`. Fix any compilation errors from removed fields.

## Required Specs

<!-- SPECS_START -->
<!-- SPECS_END -->
