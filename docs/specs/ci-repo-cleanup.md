# Spec: ci-repo-cleanup

Scope: repo

# CI Fix & Repo Cleanup

## Problem

CI workflows (GitHub Actions) fail on `git checkout` with:

```
fatal: No url found for submodule path 'legacy/tesla-fsd-can-mod-2-main' in .gitmodules
```

Dependabot update jobs fail because:

1. `Shayennn/FUCKYOU-TESLA-FSD` upstream repo returns 404
2. Dependabot cannot find dependency files in `firmware/` (PlatformIO project, not npm)

## Root Causes

1. **Stale tree entry** `legacy/tesla-fsd-can-mod-2-main` — exists in git tree (160000 commit) but has no `.gitmodules` entry. Duplicate of `legacy/iubns-tesla-fsd-can-mod` (same commit SHA).
2. **Dead upstream** `Shayennn/FUCKYOU-TESLA-FSD` — in `.gitmodules` but deleted from GitHub. Causes Dependabot clone failure.
3. **Misconfigured Dependabot** — tries to scan `firmware/` for package files that don't exist (PlatformIO uses `platformio.ini`).

## Changes

### 1. Remove stale submodule tree entry

- `git rm --cached legacy/tesla-fsd-can-mod-2-main` — removes from index only
- Does not touch `.gitmodules` (no entry exists)
- Does not delete the legacy submodule data (already covered by `iubns-tesla-fsd-can-mod`)

### 2. Remove dead submodule from .gitmodules

- Remove `[submodule "legacy/Shayennn-FUCKYOU-TESLA-FSD"]` block from `.gitmodules`
- Remove `legacy/Shayennn-FUCKYOU-TESLA-FSD/.git` directory to convert it from submodule to plain files
- Keep all other code files intact

### 3. Close stale Dependabot PRs and delete branches

- Close all open PRs authored by `dependabot[bot]` (approx 48)
- Delete all remote branches with prefix `dependabot/`

### 4. Delete stale local branches

- `copilot/fix-all-prs`
- `fix/local-changes-batch`

### 5. Configure Dependabot

- Add `dependabot.yml` to skip `firmware/` directory (platformio package-ecosystem not supported)
- Or: set existing config to exclude `firmware/` path

## What We Do NOT Touch

- `.git/config` local entries (left as-is)
- Any files under `legacy/` (except stripping `.git` from Shayennn)
- The existing `Remove-OTA` plan
- Any source code

## Verification

1. `git submodule status` — no errors
2. CI `actions/checkout` succeeds on next push
3. Zero open Dependabot PRs
4. Zero stale remote/local branches
5. Dependabot dashboard shows no errors for this repo
