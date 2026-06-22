# CI Failure Postmortem — 2026-06-20

## Summary

`main` push for PR #102 (mermaid wave 2) failed `markdown-lint` job. Single `MD001` violation in `docs/reference/can-export.md`. Fixed, pushed `54a09cc`, CI green.

## Timeline

| Time (UTC) | Event                                                                                                       |
| ---------- | ----------------------------------------------------------------------------------------------------------- |
| 18:40      | PR #102 merged into `main` (commit `b171452`)                                                               |
| 18:40      | CI triggered, `markdown-lint` job runs                                                                      |
| 18:45      | `markdown-lint` fails: `docs/reference/can-export.md:25 MD001 heading-increment [Expected: h2; Actual: h3]` |
| 18:45      | Run 27880332017 reports `failure`                                                                           |
| 18:59      | Fix committed: `54a09cc docs(reference): fix MD001 heading-increment in can-export.md`                      |
| 18:59      | Pushed to `origin/main`                                                                                     |
| ~19:10     | CI run 27880811448 completes `success`                                                                      |

## Root Cause

`docs/reference/can-export.md` jumped from `# CAN Dataset Export` (h1) directly to `### DBC (.dbc)` and `### CSV (.csv)` (h3). MD001 requires h1 → h2 → h3 with no skips.

## Fix

Changed both `###` headings to `##` to match the existing h2 in mermaid code blocks. Diff:

```diff
-### DBC (`.dbc`)
+## DBC (`.dbc`)
 ...
-### CSV (`.csv`)
+## CSV (`.csv`)
```

2 insertions, 2 deletions. No other content changes.

## Prevention

Wrote `.opencode/skills/markdown-lint/SKILL.md` covering:

- `.markdownlint-cli2.jsonc` config + ignore list
- All common gotchas in a table (MD001, MD022, MD031, MD032, MD009, MD012, MD035, MD026, MD041, MD047, MD034)
- Local pre-commit verification command
- CI parity check
- Husky hook scope (Prettier fixes some, not structural)
- Emergency-disable policy
- Failure recovery command
- Pre-PR checklist

Skill triggers on any `.md` edit in non-ignored paths.

## Action Items

- [x] Fix `can-export.md` heading-increment
- [x] Push fix
- [x] Verify CI green
- [x] Write `markdown-lint` skill
- [ ] Commit skill (deferred — needs user permission per `commit-ask-first`)
- [ ] Pre-PR: run `npx markdownlint-cli2 "**/*.md" "#legacy/**" ...` on all future .md changes

## Lessons

1. Wave 1 (PR #101) had no markdown-lint errors; wave 2 (PR #102) did. Difference: wave 2 wrote many new legacy per-repo docs, but the failing file was in `docs/reference/` — same authoring pass. Catch: the local `npm run lint:all` script does not include markdown-lint by default.
2. The pre-commit lint pass on the local dev machine did not catch this — `lint:all` runs ESLint + Prettier, not `markdownlint-cli2`. `markdown-lint` only runs in CI.
3. Recommend: add `npx markdownlint-cli2 ...` to `npm run lint:all` or a dedicated `npm run lint:md` script so CI failures become local-preventable.

## Stats

- 1 file changed
- 2 lines changed
- 0 commits squashed
- 1 CI run to detect
- 1 CI run to fix
- 1 new skill created
- ~0 engineer-hours spent
