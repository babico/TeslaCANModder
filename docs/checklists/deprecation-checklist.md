---
title: Client Workspace Maintenance Checklist
description: Ongoing guardrails that keep the repository consolidated on the unified client workspace
category: checklists
folder: checklists
tags: [client, workspace, checklist, maintenance]
order: 15
---

# Client Workspace Maintenance Checklist

Client consolidation is already complete. Keep this document as an ongoing guardrail so the repo does not drift back toward multiple app workspaces.

Run this checklist whenever changes touch docs, Docker, CI, workspace metadata, or client docs asset loading.

## Decision

- Keep this document.
- Remove the one-time migration/sign-off framing.
- Treat the items below as invariants and drift checks, not as a new project phase.

## Workspace Invariants

| Invariant                                                                                  | Why it matters                                               | Verified? |
| ------------------------------------------------------------------------------------------ | ------------------------------------------------------------ | --------- |
| Root `package.json` keeps only `client`, `tools`, and `packages/*` as active workspaces    | Prevents parallel app surfaces from reappearing              | ☐         |
| Root docs describe `client/` as the primary app surface                                    | Keeps setup and release guidance aligned with the codebase   | ☐         |
| CI and Docker use `client/` for browser/native app validation                              | Avoids stale build paths and hidden regressions              | ☐         |
| In-app docs still render the raw `docs/` markdown tree without a parallel generated bundle | Prevents docs drift and avoids a second docs source of truth | ☐         |
| No removed app workspace paths remain in root configs or docs                              | Keeps maintenance and onboarding straightforward             | ☐         |

## When To Run This Checklist

- Root `package.json` workspace changes
- CI workflow changes
- `docker-compose.yml` or release pipeline changes
- Docs rewrites that mention browser/native app entry points
- Client docs asset loading or docs bundling changes

## Maintenance Checks

| Check                                                                                   | Owner    | Done? |
| --------------------------------------------------------------------------------------- | -------- | ----- |
| Verify browser flows still run from `client/` (`npm run web -w @teslacanmodder/client`) | QA       | ☐     |
| Verify native targets still use the unified `client/` app shell                         | QA       | ☐     |
| Verify root docs and CONTRIBUTING mention the unified client workspace                  | Docs     | ☐     |
| Verify Docker and release docs reference `client/` outputs, not removed app paths       | DevOps   | ☐     |
| Verify `npm install` still resolves a clean workspace graph                             | Engineer | ☐     |
| Verify relevant tests still pass after workspace/config changes                         | CI       | ☐     |

## Drift Response Plan

1. Search the repo root for removed app workspace names or stale paths.
2. Update root docs, CI, Docker, and release instructions to point back to `client/`.
3. Verify the client docs screen still loads the raw `docs/` markdown files after the change.
4. Re-run the relevant validation commands: `npm run test:client`, `npm run test:tools`, and any affected release/build steps.
5. If drift is widespread, capture the cleanup as a focused maintenance PR instead of layering unrelated feature work into it.

---

## References

- ADR-001: Canonical Ownership and Migration Policy
- ADR-002: Module Boundary Map and Import Rules
- ADR-003: Migration Compatibility Contract for Legacy Entry Points
- `docs/checklists/release-checklist.md` — release gates for the unified client workspace
