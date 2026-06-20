---
name: legacy-submodule-review
description: Workflow for the TeslaCANModder legacy submodule sweep — fetch all 95+ submodules, identify those with new upstream commits, re-review the actual code (not just commit messages) for relevance to TeslaCANModder, skip the two repo-omitted upstreams, and handle 404/dead submodules by disabling them in-place. Codifies the project's preferred format and skip list.
license: GPL-3.0
compatibility: opencode
metadata:
    area: research
    priority: medium
---

## What I do

A repeatable workflow for sweeping the `legacy/` submodule tree and updating the per-repo analysis docs in `docs/legacy/`. Prevents the four most common mistakes:

- Reading commit messages instead of the actual code changes
- Trying to create per-repo docs for every repo including the two that should be skipped
- Deleting a dead submodule's files when the user wants them kept
- Mixing review-doc writing with submodule SHA bumps in one commit

## When to use me

Use this skill when:

- The user says "re-review the legacy repos" / "fetch and review all" / "check legacy for changes"
- Periodic legacy research is requested
- A new bleed-over concern is found in an upstream legacy repo and needs to be cross-referenced

## Hard rules

### Fetch and compare

1. **Fetch every submodule** with `git submodule foreach 'git fetch --tags --prune'`. This is a read-only operation; safe to run without asking.
2. **Compare each submodule to its default branch** with a `git submodule foreach` script that:
    - Resolves the default branch (try `origin/HEAD`, fall back to `main`, then `master`)
    - Counts `git rev-list --count HEAD..origin/<default>` commits
3. **Record the list** of submodules with non-zero ahead counts. The typical candidates every sweep:
    - `teslamotors-vehicle-command`
    - `yoziru-esphome-tesla-ble`
    - `hypery11-flipper-tesla-fsd`
    - `ev-open-can-tools-ev-open-can-tools`
    - `commaai-openpilot` _(review only — no per-repo doc)_
    - `sunnypilot` _(review only — no per-repo doc)_

### Skip list

**Always skip per-repo doc creation for these two repos.** They are covered only in the consolidated review doc, never as standalone `docs/legacy/<repo>.md`:

- `commaai-openpilot`
- `sunnypilot`

Reason: both are large openpilot-family forks where a per-repo analysis would duplicate material already in `docs/reference/`. Reference them from the consolidated review doc instead.

### Re-review the actual code

4. **Do not stop at commit messages.** For each changed repo, `git diff OLD..NEW` the relevant files and read them. Look for:
    - Patterns applicable to our `firmware/lib/vehicle/can/feature/*` and `firmware/lib/core/can/*`
    - Checksum formulas, DLC guards, bit-field layouts, bus-off recovery
    - Ble/VCSEC protocol additions
    - Frame layouts that contradict our assumptions
5. **For each finding, assess TeslaCANModder impact.** A finding is actionable only if it touches code we ship, a CAN frame we mutate, or a protocol we speak. Document the impact as either "candidate for future PR", "adopted", or "no impact".

### Document the sweep

6. **Write a single consolidated review doc** at `docs/legacy/upstream-review-YYYY-MM-DD.md`. Include:
    - All submodules with new commits and the count
    - All submodules with unreachable upstream and the action taken
    - A cross-repo "Impact on TeslaCANModder" section
    - The reproduction commands
7. **Add a concise `## Upstream (date)` section to each per-repo doc** that actually had changes. Bullet points, not a full commit table. Link to the review doc for the full list.
8. **Do NOT edit per-repo docs for `commaai-openpilot` or `sunnypilot`.**

### Handle 404 / dead submodules

When upstream returns 404 or is permanently gone:

1. **Disable the submodule but keep the files.** Steps, in order:
    - Comment out the `[submodule "<path>"]` block in `.gitmodules` with a note pointing at the review doc
    - `git config --remove-section submodule.<path>` (removes the entry from `.git/config`)
    - `Remove-Item -Recurse -Force .git/modules/<path>` (Windows) or `rm -rf .git/modules/<path>` (bash)
    - `Remove-Item -Force legacy/<path>/.git` (the dangling file pointer inside the legacy dir)
    - The working tree files at `legacy/<path>/` stay
2. **Document the disable in the review doc** with: 404 status, action taken, and a recommendation to re-check the URL periodically.
3. **Do NOT `git rm --cached` the submodule.** That would unstage the files and force them to be committed as regular files, which bloats the repo. The gitlink stays in the index; the working tree files are kept; the submodule is unregistered.

### Submodule SHA bumps are a separate concern

- Bumping submodule SHAs to track latest upstream is a _separate_ decision from re-reviewing the code.
- Do **not** bundle a SHA bump into the review-doc commit. The user may want to ship the docs without bumping SHAs, or vice versa.
- If the user says "pull and bump", bump the SHAs in a separate commit after the review doc lands. Use `git submodule foreach` to checkout the default branch (creating a local tracking branch if the submodule is on a detached HEAD) and then `git pull --ff-only origin <default>`. Then `git add` the submodules in the parent to update the gitlinks.

## Acceptable without asking

- Running `git submodule foreach 'git fetch --tags --prune'`
- Running `git log`, `git diff`, `git show` inside submodules
- Writing the review doc and the per-repo upstream sections
- Running `npm run test:all`, `npm run lint`, and firmware build commands

## Ask first

- Pulling/bumping submodule SHAs
- Disabling a dead submodule (the 5 steps above)
- Editing `docs/legacy/README.md` or `docs/legacy/COMPARISON.md`
- Creating a new per-repo doc (other than the two on the skip list)
- Committing, pushing, opening a PR

## Why this exists

The legacy sweep is the project's primary research channel. The previous attempt had four recurring mistakes: relying on commit summaries instead of code, trying to create per-repo docs for the two skip-list repos, deleting dead-submodule files instead of disabling them, and bundling review-doc writes with SHA bumps. This skill encodes the working pattern.
