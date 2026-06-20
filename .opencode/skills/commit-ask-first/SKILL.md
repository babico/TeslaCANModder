---
name: commit-ask-first
description: Enforce asking the user for explicit permission before any git mutation in the TeslaCANModder repo (commit, push, merge, PR create, PR merge, release, force operations). Codifies the project's strict no-surprise git policy.
license: GPL-3.0
compatibility: opencode
metadata:
    area: workflow
    priority: high
---

## What I do

Block every automated git mutation until the user has explicitly approved the exact operation. The TeslaCANModder maintainer wants to see and approve what lands in the tree, on every branch, on every remote, and in every PR — even when the diff looks obvious.

## When to use me

Use this skill before any of the following operations:

- `git commit`
- `git commit --amend`
- `git push` (including `-u`, `--force`, `--force-with-lease`, `--tags`)
- `git fetch` + `git reset --hard origin/<branch>` (anything that rewrites local history)
- `git branch -D` / `git branch -d` (deleting branches, local or remote)
- `git tag -d` / pushing tags
- `git revert` / `git reset` / `git rebase`
- `gh pr create` / `gh pr merge` / `gh release create` / `gh release upload`
- `git submodule update` that bumps a recorded SHA (changes what the parent tracks)
- Removing files with `git rm` that the user did not explicitly ask to remove

## Hard rules

1. **Show first, then ask.** Before invoking the destructive or remote command, present:
    - The exact command(s) you are about to run
    - The `git status --short` and `git diff --stat` of what will be committed/pushed
    - The branch and target remote
    - For PRs: title and body
2. **Wait for a clear yes.** Vague or implicit consent (e.g., "looks good, continue") is not enough for `git push`, `gh pr merge`, branch deletion, or `git push --force`. Get an explicit "yes, push" / "yes, merge" / "yes, delete" / "yes, force-push".
3. **One commit per "yes".** If you intend to make two distinct commits (e.g., "commit the feifan refactor, then a separate commit for the submodule SHA bumps"), ask once per commit and label them clearly so the user can approve or veto individually.
4. **Never batch.** Do not combine "I'll commit and push and open a PR" into a single ask. The user said "ask first" because they want to inspect intermediate state and the PR text.
5. **No amend without a new "yes".** If a hook reformats files and creates a follow-up diff, do not auto-amend. Show the new diff and ask whether to amend or commit as a separate fixup.
6. **`--no-verify` is forbidden.** If a hook rejects a commit, fix the cause and create a new commit. Do not bypass hooks.
7. **No commits to `main`.** Always use a feature branch with `-u origin` tracking on first push. Use `gh pr create` to land it.
8. **Submodule SHA bumps are separate.** If a task involves both firmware code changes AND bumping legacy submodule SHAs, treat them as two independent asks. The user often wants them in different PRs.

## Acceptable without asking

- `git status`
- `git diff` / `git log` / `git show`
- `git fetch` that does not change local history (e.g., a plain `git fetch origin` to inspect remote state)
- `git submodule foreach 'git fetch --tags --prune'` to gather upstream data for a review
- `git submodule update` (no flags) that **resets submodules to the recorded SHA** (no history rewrite, no SHA bump)
- Running tests, linters, builds

## Edge cases

- **The user said "commit it" or "good commit it".** That is an explicit yes for the specific commit you just showed them. Do not chain a push, merge, or PR creation onto that approval.
- **The user gave a plan approval** ("all of them okey" to a multi-step plan). That is an approval to _implement_. It is not an approval to commit, push, or open a PR. Always re-ask at the commit stage.
- **Husky/lint-staged reformats staged files after `git commit`.** This produces a follow-up diff. Show it, ask whether to amend, and wait. Do not auto-amend.
- **Recovery from a bad commit the user didn't like.** `git reset --soft HEAD~1` to un-commit while keeping the working tree is acceptable without asking IF the previous commit was just made in the same session and the user has not pushed it. Otherwise ask first.

## Why this exists

The user explicitly said: _"dont commit everything amk ask me first"_ after a previous agent committed several changes without permission. The cost of an extra question is small; the cost of an unwanted commit, push, or merge on `main` is large.
