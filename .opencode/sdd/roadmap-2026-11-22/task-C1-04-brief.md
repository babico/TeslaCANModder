# Task C1-04 — BLE scan filter

Where this fits: Phase 1 correctness. Our advertisement filter uses a last-8-VIN suffix, but the observed Tesla Local Name is `S + sha1(VIN)[:8] + C`. The filter therefore cannot match a real vehicle advert.

## Requirement (verbatim from plan, lines 54-55)

Replace last-8-VIN suffix with the observed `S+sha1(VIN)[:8]C` Local Name.

- Acceptance: advertisement-filter unit test against recorded advert names.
- Evidence: `dispatch.h:36-43`, `ble_client.h:116-120`, repo-state `:28` (firmware.md §7.2#9).
- Effort: 1–2 h. Risk: Medium.

## Interfaces / decisions from earlier tasks

- C1-01..03 changed BLE wire tags/add-key/roles only; do not touch them.
- The formula and its VIN input are from community observation (SUMMARY-ONLY upstream evidence) — no vehicle capture exists here. Implement the observed formula; mark "real vehicle acceptance UNVERIFIED" in the report.
- Test must pin literal expected Local Names for at least two known VINs (compute SHA-1 independently in the test or use precomputed vectors with source noted). Do not derive the expected value from production code.

## Global constraints

- No code copied from `legacy/` into shipping code.
- BLE behavior change needs a unit test with literal expected values.
- Stage ONLY files you change; worktree has 191 unrelated unstaged changes. Verify `git diff --cached --name-only` before committing.
- Commit and push on the current feature branch; never main, never force-push.
