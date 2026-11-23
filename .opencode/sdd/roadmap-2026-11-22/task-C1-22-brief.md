# Task C1-22 — resync generated docs bundle and skills after C1-06

Where this fits: Phase 1 follow-up. C1-06 removed OTA TX-pause claims from canonical docs/schema, but the LIVE in-app docs bundle and two project skills still advertise the non-existent gate.

## Requirement (verbatim from plan C1-22)

Regenerate the bundle in a separate commit, skills corrected, claim test extended to catch reintroduction.

- Evidence: `client/src/docs/_generated-docs.ts` is imported by `client/src/docs/catalog.ts:1`; stale claims at generated lines 186, 1273, 4761, 14056, 15463. Skills: `.opencode/skills/can-protocol-safety/SKILL.md:78-87`, `.opencode/skills/firmware-development/SKILL.md:139`. Also stale firmware comment `dispatch.h:423` ("hasCtrl gate").

## Decisions (binding)

- Generate with `node client/scripts/generate-docs-catalog.mjs` (no npm script exists). Commit the generated bundle in its OWN commit, separate from any hand-edited source/skills/test changes, per the repo's auto-generated-file policy.
- Correct the two skills and the stale `dispatch.h:423` comment to the current single-gate API / no-OTA-guard reality.
- Extend `packages/protocol/test/integration/ota-guard-claim.test.ts` so the generated bundle is one of its claim targets (future staleness fails CI). Keep legacy `docs/legacy/**` excluded deliberately.
- Client behavior tests must still pass; no client UI rewrite here.

## Global constraints

- Stage ONLY files you change; 191 unrelated unstaged files exist. Verify staged lists before each commit.
- Two commits: (1) generated bundle only, (2) skills + test + comment. Push both to the current feature branch; never main, never force-push.
- No `legacy/` code copied; no new OTA gate introduced.
