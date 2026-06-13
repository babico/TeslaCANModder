---
plan name: ci-repo-cleanup
plan description: Fix CI failures and repo cleanup
plan status: done
---

---

plan name: ci-repo-cleanup
plan description: Fix CI failures and repo cleanup
plan status: active

---

## Idea

Fix CI checkout failures caused by stale submodule references, clean up 48+ stale Dependabot PRs and 50+ remote branches, configure Dependabot to skip firmware/, and remove dead submodule upstreams from .gitmodules while preserving all legacy code files.

## Implementation

- Remove stale submodule tree entry legacy/tesla-fsd-can-mod-2-main from git index
- Remove dead Shayennn/FUCKYOU-TESLA-FSD submodule from .gitmodules and strip its .git dir
- Create dependabot.yml excluding firmware/ (PlatformIO not supported)
- Close all ~48 open Dependabot PRs via GitHub API
- Delete all ~50 Dependabot remote branches
- Delete stale local branches (copilot/fix-all-prs, fix/local-changes-batch)
- Commit and push all changes
- Verify CI passes and Dependabot dashboard shows no errors

## Required Specs

<!-- SPECS_START -->

- ci-repo-cleanup
  <!-- SPECS_END -->
