---
name: legacy-submodule-update
description: Use when updating the TeslaCANModder legacy/ submodule tree to latest upstream, bumping gitlink SHAs, or converting a deleted/404 upstream submodule into a normal tracked folder without losing the local research code. Covers fetch aborts on dead remotes, the .gitmodules staging order required by git rm --cached, Windows PowerShell escaping traps, and the verification that git submodule foreach still works.
license: GPL-3.0
compatibility: opencode
metadata:
    area: research
    priority: medium
---

## What this does

Updates the read-only `legacy/` research tree and preserves local copies when upstream disappears. Two jobs, one sequence:

1. **Bump** submodules that are behind their default branch.
2. **Convert** a dead (404) submodule into an ordinary tracked folder, keeping every working-tree file.

Core principle: the working-tree files are the research data. Never delete them to "clean up" a dead submodule. A broken gitlink in the parent index is worse than an extra untracked folder because it breaks `git submodule foreach` in CI.

## When to use

- "legacy altındakileri güncelle" / "update the legacy repos" / "pull and bump legacy"
- "repo silindiyse yedeğini silme, normal folder ekle" / "keep the local copy as a normal folder"
- A `git submodule foreach` sweep dies with `fatal: repository '...' not found`

Do **not** use for code re-review, impact analysis, or writing `docs/legacy/upstream-review-*.md` — that is `legacy-submodule-review`. Do not copy legacy code into shipping code. Do not edit `docs/legacy/README.md` or `docs/legacy/COMPARISON.md` without asking.

## Ask first

- Committing, pushing, opening a PR (also enforced by `commit-ask-first`).
- Any change that would delete files under `legacy/`.
- Editing `legacy/` files by hand — these are reference snapshots.

Local-only work (fetch, diff, index changes, staging) is fine when the user says "lokalde yap" / "local only".

## Procedure

### 1. Preflight — never skip

```bash
git status --short                 # record pre-existing changes; do not blame them on the sweep
git submodule status               # note '-' (uninitialized) and '+' (ahead/behind) entries
```

Check each submodule for a dirty tree or local commits **before** bumping. Never `--ff-only` over local work:

```bash
git submodule foreach 'git status --porcelain'
```

### 2. Fetch all, tolerating dead remotes

`git submodule foreach` stops at the first non-zero exit. A single 404 aborts the whole sweep, so swallow failures inside the loop:

```bash
git submodule foreach 'git fetch --tags --prune 2>&1 || echo "FETCH_FAIL $name $path"' > "$TMP/fetch.log" 2>&1
```

A moved tag makes plain `--tags` fail even though refs updated (historical case: `ev-open-can-tools`). Re-run that one repo with `git fetch --tags --prune --force` only if the new tags are actually needed.

Classify each failure:

- `Repository not found` / HTTP 404 → dead upstream, candidate for conversion.
- Tag clobber warning → upstream alive, no conversion.
- Network/DNS error → retry later, do not convert.

Never run `git submodule update --init --recursive`, `git submodule deinit -f`, `git clean -fdx legacy/`, or `rm -rf legacy/<path>` during a sweep. Each can destroy the backup.

### 3. Compare to default branch

Resolve default as `origin/HEAD` → `.gitmodules` `branch` → `main` → `master`. This repo has at least one `master` repo (`ColinM-sys-tesla-can-boost`) and one with `branch = main` (`ev-open-can-tools`).

```bash
git submodule foreach '
    default=$(git symbolic-ref --short refs/remotes/origin/HEAD 2>/dev/null | sed "s|^origin/||")
    [ -z "$default" ] && default=$(git config -f "$toplevel/.gitmodules" --get "submodule.$name.branch" 2>/dev/null)
    [ -z "$default" ] && default=main
    git rev-parse --verify -q "origin/$default" >/dev/null || default=master
    echo "$name $path $default +$(git rev-list --count HEAD..origin/$default) -$(git rev-list --count origin/$default..HEAD)"
'
```

Bump only `+N > 0` **and** `-N == 0`. Local commits or a dirty tree mean stop and ask.

### 4. Bump

Detached HEAD at an ancestor fast-forwards fine. Use `--ff-only` so a surprising local state refuses instead of overwriting:

```bash
git -C legacy/<repo> checkout -B <default> origin/<default>   # only when detached
git -C legacy/<repo> pull --ff-only origin <default>          # when already on the branch
git add -- legacy/<repo>                                      # records the new gitlink locally
```

`git checkout -B <default> origin/<default>` is safe only when step 3 proved `origin/<default>..HEAD` is empty. Stage the paths that actually moved; do not blanket-stage every submodule.

### 5. Convert a dead 404 submodule into a normal tracked folder

Order is load-bearing. Do the steps exactly as written.

```bash
P=legacy/<dead-repo>
```

**5a — inventory before touching anything.** Record the pinned SHA and file count; the count is the verification target later.

```bash
git ls-tree HEAD -- "$P"                     # must be 160000 commit <sha>
git -C "$P" rev-parse HEAD                   # record last pinned SHA
git -C "$P" status --porcelain               # must be empty
git -C "$P" ls-files | wc -l                 # expected tracked count after conversion
```

**5b — optional history backup.** Removing `.git/modules/<path>` destroys objects; a compressed copy elsewhere is cheap insurance.

```bash
tar -czf "$TMP/$P-objects.tgz" -C .git/modules "$P"
```

**5c — comment out the `[submodule]` block in `.gitmodules`.** Keep a dated note pointing at the review doc.

```text
# Disabled YYYY-MM-DD: upstream repo deleted (404). Files kept as a normal
# tracked folder so the local copy survives. See docs/legacy/upstream-review-YYYY-MM-DD.md.
#[submodule "legacy/<dead-repo>"]
#	path = legacy/<dead-repo>
#	url = https://github.com/owner/repo
```

**5d — CRITICAL: stage `.gitmodules` before running `git rm --cached`.** Otherwise git refuses with:

```text
fatal: please stage your changes to .gitmodules or stash them to proceed
```

```bash
git add -- .gitmodules
```

`git rm --cached -f` does **not** fix this error. The problem is the unstaged `.gitmodules`, not the path state.

**5e — remove config section and gitlink.** `--cached` never touches working-tree files.

```bash
git config --remove-section "submodule.$P" || true
git rm --cached -- "$P"
```

**5f — remove the dangling gitfile, then the module store.** Reverse order leaves a broken pointer.

```bash
rm -f "$P/.git"
rm -rf ".git/modules/$P"
```

**5g — track the preserved files as ordinary files.**

```bash
git add -A -- "$P"
git status --short -- "$P"       # expect only 'A' lines; ZERO 'D' lines
git ls-files -s -- "$P"          # all 100644/100755, no 160000
```

Nested `.gitignore` files inside the legacy repo become active after conversion. If a must-keep file is ignored, use `git add -f -- "$P/<file>"` for that specific file — do not add the folder to the parent `.gitignore`, which would hide the preserved code from `git status`.

### 6. Verify

```bash
git config -f .gitmodules --list >/dev/null    # .gitmodules still parses
git submodule status                           # exit 0, no "no submodule mapping found"
git submodule foreach 'echo $name' | grep -F "$P"   # no output: no longer iterated
git ls-files -s -- "$P" | grep 160000          # no output: gitlink gone
git ls-files -s -- "$P" | wc -l                # matches 5a count, minus ignored files
git diff --check && git diff --cached --check
```

A `fatal: no submodule mapping found in .gitmodules for path '<path>'` means the gitlink is still in the index — step 5e was missed or run out of order. Fix with `git rm --cached` before doing anything else; CI checkout runs `git submodule foreach --recursive` and will fail until this is clean.

## Windows / PowerShell traps

The repo is developed on Windows with pwsh. Bash snippets above translate, but watch these:

- **Backticks inside double-quoted here-strings are escape characters.** ``"`-f`"`` becomes a form feed and ``"`nag.h`"`` becomes a newline plus `ag.h`. Appending markdown this way silently corrupts commit hashes and code spans. Use a single-quoted here-string (`@' ... '@`) or the editor/write tool instead.
- `Remove-Item -Force "$P\.git"` and `Remove-Item -Recurse -Force ".git\modules\$P"` replace the `rm` lines.
- Prefer `$env:TEMP` over `/tmp` for logs.
- `git add` prints `CRLF will be replaced by LF` warnings on Windows. Expected: the repo enforces LF via `.gitattributes`; staged content is normalized. Not an error.
- Use `git` piping through `Out-String` carefully; `git submodule foreach` output is line-oriented and `Select-Object -First N` hides later failures. Export to a CSV/log and inspect the failure rows instead.

## Quick reference

| Symptom                                                             | Cause                                  | Fix                                                   |
| ------------------------------------------------------------------- | -------------------------------------- | ----------------------------------------------------- |
| `foreach` stops after one repo with `Repository not found`          | non-zero exit aborts the loop          | `git fetch ... \|\| true` inside the loop             |
| `please stage your changes to .gitmodules or stash them to proceed` | `.gitmodules` edited but unstaged      | `git add -- .gitmodules` **before** `git rm --cached` |
| `fatal: no submodule mapping found in .gitmodules`                  | gitlink still in parent index          | `git rm --cached <path>`                              |
| `git add -A <path>` re-adds a gitlink                               | `.git` gitfile still inside the folder | `rm -f <path>/.git` first                             |
| Converted files missing from `git status`                           | path added to `.gitignore`             | remove ignore rule, `git add -f` the must-keep files  |
| Markdown hashes like `` `f82d031` `` come out as control characters | PowerShell backtick escaping           | single-quoted here-string or write tool               |

## What NOT to do

- Do not delete the working-tree files of a dead repo. The whole point is preservation.
- Do not bundle review-doc writing and SHA bumps into one commit unless the user asks for both. Keep them separable.
- Do not commit or push when the user says "lokalde yap" / "pushlama".
- Do not run the sweep on `commaai-openpilot` or `sunnypilot`; they are intentionally absent from the tree.
- Do not touch any folder under `legacy/` that has no active `[submodule]` entry in `.gitmodules`. It is already converted; leave it alone.

## Related skills

- `legacy-submodule-review` — fetch/compare/review/impact docs and the two-repo skip list.
- `commit-ask-first` — explicit permission required before any git mutation that commits or pushes.
- `markdown-lint` — `docs/legacy/**` is ignored by the linter, but keep headings and fences clean anyway.
