---
name: markdown-lint
description: Markdown linting for non-legacy TeslaCANModder docs. Triggers on any .md edit in docs/, *.md at root, or markdown-touching commits. Prevents CI markdown-lint failures (MD001 heading-increment etc.) by enforcing pre-commit lint, common gotchas, and config awareness.
---

# Markdown Lint Skill

CI fails `markdown-lint` job when any .md in non-ignored paths violates rules. Past failures: `MD001` heading-increment (h1 → h3 skip), `MD040` fenced code language, `MD009` trailing spaces, `MD031` blanks around fences, `MD025` single-H1. Avoid repeat.

## When to Use

- Creating or editing any `.md` file in: `docs/**`, repo root, `client/**`, `firmware/**`, `tools/**`, `packages/**`.
- NOT for `legacy/**` or `docs/legacy/**` (excluded by `.markdownlint-cli2.jsonc`).

## Config (authoritative)

`/`.markdownlint-cli2.jsonc``(used by CI) +`/.markdownlint.json` (used by local CLI without --config).

```jsonc
{
    "config": {
        "MD007": false, // ul-indent
        "MD010": { "code_blocks": false }, // no-hard-emphasis (allow underscores in code)
    },
    "ignores": [
        "legacy/**",
        "docs/legacy/**",
        "**/node_modules/**",
        "**/dist/**",
        "**/.pio/**",
        "**/.pio-home/**",
        "client/.expo/**",
        ".venv/**",
        ".opencode/**",
        "MCP_SERVERS.md",
    ],
}
```

`MD013` (line-length), `MD024` (duplicate-heading), `MD025` (single-h1), `MD040` (fenced-code-language) are off. `MD007` (ul-indent) is off. Everything else ON.

## Gotchas (rules that bite)

| Rule                            | Trap                                        | Fix                                    |
| ------------------------------- | ------------------------------------------- | -------------------------------------- |
| `MD001` heading-increment       | `#` then `###` (skips h2)                   | Use `##` between h1 and h3             |
| `MD022` blanks-around-headings  | Heading directly touching paragraph/HTML    | Blank line above + below               |
| `MD031` blanks-around-fences    | ` ` ``` directly touching text              | Blank line above + below               |
| `MD032` blanks-around-lists     | List directly after paragraph without blank | Blank line above list                  |
| `MD009` no-trailing-spaces      | Trailing whitespace on line                 | Strip                                  |
| `MD012` no-multiple-blanks      | Two+ consecutive blank lines                | One blank line max                     |
| `MD035` hr-style                | Mixed `---` and `***` for hr                | Pick one, use consistently             |
| `MD026` no-trailing-punctuation | `Heading?` `Heading!`                       | Rephrase or end with period            |
| `MD041` first-line-h1           | File starts with anything but `#`           | First non-frontmatter line must be `#` |
| `MD047` file-end-newline        | No trailing newline                         | End file with `\n`                     |
| `MD034` no-bare-urls            | Bare `https://...` in body                  | Wrap in `<>` or `[text](url)`          |

## Heading Rules (the one that bit us)

- Frontmatter (`---` block) does NOT count as h1. First `#` after frontmatter is h1.
- `#` → `##` → `###` only. No skipping. Mermaid code blocks don't count.
- One h1 per file (`MD025` off in config but the workflow uses default; safer to keep one).
- Don't nest deeper than h4 in prose (h6 max but readability dies).

## Code Fence Rules

- Always specify language: ` ```ts ` not bare ` ``` `.
- Blank line above + below every fence.
- Mermaid uses ` ```mermaid ` language tag.

## Pre-Commit Verification (REQUIRED)

Before declaring any markdown work done, run from repo root:

```bash
# Lint only the files you touched (fast)
npx markdownlint-cli2 path/to/file.md

# Lint all non-ignored files (matches CI exactly)
npx markdownlint-cli2 "**/*.md" "#legacy/**" "#docs/legacy/**" "#**/node_modules/**" "#**/dist/**" "#**/.pio/**" "#**/.pio-home/**" "#client/.expo/**" "#.venv/**" "#.opencode/**" "#MCP_SERVERS.md"
```

CI command (from `.github/workflows/ci.yml`):

```yaml
- uses: DavidAnson/markdownlint-cli2-action@v23
  with:
    globs: **/*.md
```

No `args:` line means default invocation; ignores come from `.markdownlint-cli2.jsonc`.

## Common Workflow

1. Edit `.md` file.
2. Run `npx markdownlint-cli2 <file>` to verify.
3. Fix any errors before committing.
4. If rule is wrong, change config (`.markdownlint-cli2.jsonc`) AND `.markdownlint.json` (both files must match for local + CI parity).
5. If file belongs in `docs/legacy/**`, it is already ignored — but do not move non-legacy files there to escape the linter.

## Husky Hook

`lint-staged` runs Prettier on staged `*.md` files. Prettier auto-fixes some lint rules (whitespace, blank lines). It does NOT fix `MD001`, `MD040`, `MD031`, or any structural rule. Manual fix required.

## Emergency Disable (DO NOT USE)

Adding a file to `.markdownlint-cli2.jsonc` `ignores:` is a CODE SMELL. The only legitimate use is generated/vendor content. If you must ignore a real doc, justify it in commit message + mention in PR description.

## Failure Recovery

If CI `markdown-lint` fails after push:

```bash
gh run view <run-id> --log-failed
```

Output is one line per error: `path:line MDxxx/rule-name message`. Read line, fix, commit, push. No exceptions, no "I'll fix it in a follow-up" — same PR or revert.

## Checklist Before PR

- [ ] Ran `npx markdownlint-cli2 "**/*.md" "#legacy/**" "#docs/legacy/**" "#**/node_modules/**" "#**/dist/**" "#**/.pio/**" "#**/.pio-home/**" "#client/.expo/**" "#.venv/**" "#.opencode/**" "#MCP_SERVERS.md"` locally → 0 errors.
- [ ] No new files added to `ignores:` (or justified in commit).
- [ ] If config changed: both `.markdownlint-cli2.jsonc` AND `.markdownlint.json` updated.
- [ ] No `MD001` skips (the recurring one).
- [ ] All fences have language tags.
