# Task C1 — Linux TX backend + can-review

Where: first gated TX task. Implements the Pi-side CAN transmit path, which must remain unreachable without the B3 gate artifact.

## Requirement (verbatim from plan)

Files:

- Create: `pi/src/can/tx.ts`, `pi/test/tx.test.ts`, `pi/gate/README.md`
- Modify: `pi/src/index.ts` (feature flag `PI_CAN_TX=0` default)

Steps:

1. Test that TX is unreachable when gate artifact missing/invalid or flag off.
2. Implement backend using the same frame builders/CRC rules from our firmware (independent test oracle), never copying Sentry code.
3. Walk `docs/checklists/can-review-checklist.md` in the report with one-line answers; any CAN mutation needs a listen-only capture reference.
4. GREEN native/vcan tests + lint; commit.

## Binding constraints

- Default `PI_CAN_TX=0`. A valid `pi/gate/rt-result.json` (B3 `isGateOpen()`) AND explicit flag are both required; no artifact exists here.
- No physical Pi, no bench capture, no signed gate artifact → this task delivers a DISABLED backend only. Real injection is a release blocker, not a claim of this task.
- can-review checklist must be walked with honest "capture-first / UNVERIFIED" answers where evidence is missing — do NOT mark items passed without evidence.
- Use B3's v1 artifact schema/HMAC contract via `isGateOpen()`; do not invent a second gate check.
- No Sentry code copied. Stage ONLY changed files (~200 unrelated unstaged exist); verify staged list before commit.
- Feature branch `spike/raspberry-pi-portability`; never main, never force-push.
