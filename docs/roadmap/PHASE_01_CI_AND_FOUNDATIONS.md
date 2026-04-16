# Phase 01 - CI And Foundations

## Goal

Remove failing CI noise and establish a stable engineering baseline.

## Scope

- Web test discovery cleanup.
- Workflow dependency ordering and prerequisite builds.
- Workspace-level execution consistency checks.
- Tracking artifacts created for phased execution.

## Tasks

1. Limit web test discovery to active TS/TSX suites.
2. Ensure protocol build runs before web/mobile tests in CI.
3. Validate local parity with CI job commands.
4. Capture known environment caveats in repository notes.
5. Create and maintain planning/tracking markdown set.

## Acceptance Criteria

- Web job no longer fails from empty legacy JS test files.
- Web/mobile CI jobs run with protocol artifacts prepared.
- CI workflow file reflects explicit prerequisite steps.
- Tracker documents are present and actionable.

## Deliverables

- `.github/workflows/ci.yml` updated.
- `web/vite.config.js` updated for test include targeting.
- `docs/roadmap/*` phase and tracker documents.

## Rollback Plan

- Revert workflow and vitest include changes if unexpected regression appears.
- Restore previous file discovery pattern only if legacy JS tests are migrated and needed.
