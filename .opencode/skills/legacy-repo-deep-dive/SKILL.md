---
name: legacy-repo-deep-dive
description: Use when producing a thorough, code-grounded analysis of a single TeslaCANModder legacy/ repo, when an existing docs/legacy/<repo>.md summary is shallow or suspected wrong, or when deciding whether an upstream mechanism should be adapted into shipping firmware, client, protocol or tools code. Cites source lines, separates evidence grades, records contradictions, and builds a reusable re-verification recipe.
license: GPL-3.0
compatibility: opencode
metadata:
    area: research
    priority: medium
---

## What this does

Turns a legacy research repo into a **trusted, reusable research base**: mechanisms, frame/signal data, constants, failure paths, test gaps, surface inventory, upstream-vs-our parity, and history — all evidence-graded. The goal is not a better summary — it is deciding which solutions to reinterpret for TeslaCANModder, with enough evidence that a future agent can re-check the claims after an upstream bump.

Core principle: **code and tests are evidence; README and `docs/legacy/*.md` are leads.** A claim without a file/symbol reference is not an analysis finding.

Use this for one repo at a time. Use `legacy-submodule-review` for the periodic multi-repo sweep; use `legacy-submodule-update` for fetch/bump/dead-submodule conversion.

## Output contract

The analysis must be a document with these parts, in this order. This is a shape, not a suggestion — a future agent should be able to find each section by number AND name. Use numbered headings exactly: `## 1. Provenance` … `## 17. Open questions and explicitly not investigated`. The order is: evidence first, data tables next, parity before decisions, history near the end, verification last.

1. **Provenance** — submodule path, pinned gitlink SHA, upstream URL, license, analysis date, files read (with line ranges), files explicitly NOT read.
2. **Executive verdict** — one paragraph: what the repo is, which mechanisms matter, which are candidates, which must not be ported.
3. **Evidence map** — table: `source file | lines | what it proves | grade`.
4. **Architecture and build surface** — entry points, shared core vs platform port, build flags, dependency boundaries. Include one `mermaid` data-flow diagram (RX → decode → mutate/gate → TX), or write `None found: no frame pipeline in this repo`.
5. **Mechanism catalogue** — one card per mechanism (template below). A mechanism with 3+ states gets a `mermaid` state diagram inside its card.
6. **Summary vs code contradiction table** — every place the existing `docs/legacy/<repo>.md` or upstream README disagrees with the code. Only actual contradictions belong here; agreements go in the evidence map.
7. **CAN frame and signal inventory** — one row per frame the repo decodes, mutates, or injects: `ID (hex+dec) | bus | period | DLC | byte/bit layout | scale/units | counter position | checksum | read or write | grade`. A vendored DBC/JSON catalog is labeled `STRUCTURAL-PARSE` and summarized in one line — never dumped wholesale into this table.
8. **Constants and tuning table** — every load-bearing magic number: `constant | value | file:line | meaning | grade`. Skip trivial constants (pin masks, formatting widths); keep anything that shapes a frame, a gate, or a timing window.
9. **Failure modes and edge cases** — bus-off handling, watchdog, OTA behavior, queue overflow, brownout/ reboot state, missing-frame timeouts, and which error paths are untested. Each row: `failure | handling code path | tested? | grade`.
10. **Test coverage map** — per test file: what it asserts; then the critical paths with NO test (state the risk in one line).
11. **Parity matrix with TeslaCANModder** — for every signal/frame both sides touch: `signal/frame | upstream decode (cite) | our decode (cite) | delta | action (align / capture-first / ignore)`. A row without a our-side CODE-READ citation is SUMMARY-ONLY and cannot drive a decision.
12. **Surface inventory and security notes** — every transport surface the repo exposes (serial/web/BLE commands or endpoints) with semantics, plus provenance/security findings: hardcoded credentials, keys/certs, untrusted archives, dangerous build flags.
13. **Adaptation assessment** — per mechanism: value / cost / risk / evidence grade / decision.
14. **Do-not-copy list** — with file:line and reason for each item.
15. **History and evolution** — mine the submodule's local `git log` and changelog: what was tried, what was abandoned and why, which approaches the author rolled back. Network-only claims (issues, PRs) stay UNVERIFIED.
16. **Re-verification recipe** — exact commands, grep anchors with expected hits, pinned SHA.
17. **Open questions and explicitly not investigated** — inherited gaps, including whether pre-pin upstream history was reviewed.

If a section has no content, write `None found` — do not silently drop it. The same applies to empty mechanism-card fields (`tests`, `state machine`, `frame spec`): write `None found`, never `n/a`, never merge fields.

## Output location: companion vs replace

Two modes, decided by the user, never by you:

- **Companion (default):** write `docs/legacy/deep/<repo>.md` and add one cross-link line to the existing `docs/legacy/<repo>.md`.
- **Replace:** the user asks to "re-summarize" / overwrite. Write the deep analysis into `docs/legacy/<repo>.md` itself. Then:
    - Preserve the existing YAML frontmatter byte-for-byte and every existing `## Upstream (...)` section verbatim (place them after section 14 or at the end).
    - If the original doc had no frontmatter, say so in Provenance; do not invent one silently.
    - The replaced summary still exists in git history — cite pre-replace claims as `git show HEAD:<doc>#<line>` in the contradiction table, not as a live link.
    - A preserved frontmatter can be factually stale (old summary said ESP-NOW, code says HTTP). Keep the bytes, and record the staleness as a row in the contradiction table instead of editing the frontmatter.

## Evidence grades

Every claim gets one of these, in the evidence map and on the mechanism card:

| Grade        | Meaning                                                                 |
| ------------ | ----------------------------------------------------------------------- |
| CODE-READ    | Read the actual implementation, cited `file:line` + symbol              |
| TEST-READ    | A test asserts the behavior; cite test name and line                    |
| CAPTURE      | Backed by a captured frame / decode tool, with frame count if available |
| SUMMARY-ONLY | Only README or an existing `docs/legacy/` doc says so                   |
| UNVERIFIED   | Suspected or inferred, not checked; must say what would verify it       |

`SUMMARY-ONLY` and `UNVERIFIED` claims cannot be the basis of an adaptation decision. They are backlog items.

Closed vocabulary — only the five grades above. No compound grades, no parenthetical qualifiers like `CODE-READ (grep)` or `CODE+TEST`. If a grade does not apply to the whole repo, say it in prose inside the evidence map: `No TEST-READ evidence exists: this repo ships no tests.` / `No CAPTURE evidence exists in this repo.` Never invent a sixth grade such as `none`.

Reading coverage for large data files (DBC catalogs, vendored JSON databases, generated code) gets its own closed vocabulary in the evidence map: `FULL-READ` (every line), `STRUCTURAL-PARSE` (schema-level, claims limited to structure), `RANGE-CITED` (only the cited line ranges were read, stated per file). `docs/legacy/**` is lint-ignored, so nothing catches a 4000-line file skimmed as "read in full" — label honestly.

## Procedure

### 1. Anchor the snapshot

```bash
git ls-files -s legacy/<repo>                       # parent gitlink: 160000 <sha>
git -C legacy/<repo> rev-parse HEAD                 # must match
git -C legacy/<repo> status --porcelain             # must be empty
git -C legacy/<repo> log -1 --format='%H %s'
git -C legacy/<repo> remote -v
```

Record the SHA in the doc. If it does not match the gitlink, stop and report — the analysis would describe the wrong version.

### 2. Inventory before reading

```bash
git -C legacy/<repo> ls-files
```

Include hidden and platform subdirectories. Real gap seen in past analyses: `esp32/.firmware/` was missed because a glob skipped dot-directories, and the platform claims ended up SUMMARY-ONLY. If a directory cannot be read, list it under "not read" rather than quietly omitting it.

Read the load-bearing files in full: CAN handlers, frame builders, checksum/counter helpers, state machines, drivers, tests, protocol contracts between components. README and `docs/legacy/` are read **after** the code, to compare, not to learn from.

### 3. Mechanism card template

Repeat per mechanism. Keep byte-level detail; that is the whole point.

```markdown
### <mechanism name>

- **Claim (README / summary):** <what the docs say>
- **Code path:** `file:line` — `symbol()` → `symbol2()` → TX
- **Frame spec:** ID, DLC, byte/bit layout, counter position, checksum formula
- **Gates and timing:** preconditions, DAS/AP state, intervals, jitter, pause windows
- **State machine:** states, transitions, re-arm conditions
- **Tests:** `test_name` at `file:line` — what it actually asserts
- **Evidence grade:** CODE-READ / TEST-READ / CAPTURE / SUMMARY-ONLY
- **Verdict:** CONFIRMED / PARTIAL / CONTRADICTED / UNVERIFIED
- **Adapt to TeslaCANModder?** candidate / already present / no impact / never
```

Be precise about the things that make a frame work or fail: which nibble holds the counter, whether a hands-on flag is forced or left at zero, whether the checksum is `sum(bytes)+id+constant`, whether an injected frame is built from a cached real frame or from zeros. Record constants exactly (`1870..2230`, `1000/1500 ms`, `0x73`).

### 4. Hunt contradictions deliberately

For every number and protocol claim in the existing `docs/legacy/<repo>.md`, find the line of code that proves or disproves it. Real examples of what this catches:

- A summary says binary framing `0xAA` + XOR; the code is line-based ASCII `EVT `/`CMD ` with no `0xAA`.
- A header comment says a range `1..15`; the web slider and clamp make it `0..7`.
- A summary cites CAN ID `0x238`; the code uses `0x3FD` mux0 with a different formula.
- A summary quotes a torque walk range while the code clamps every path to a narrower band.

Put each in the contradiction table with both citations, and fix the doc in a separate step (do not bury the correction in prose). A contradiction means the summary is not trustworthy — treat the rest of it as SUMMARY-ONLY until re-checked.

### 5. Decide adaptation per mechanism, not per repo

Ask, in order:

1. **Evidence strength** — CAPTURE + TEST-READ + CODE-READ > CODE-READ only > SUMMARY-ONLY. Never port on SUMMARY-ONLY.
2. **Delta from what we ship** — read our counterpart first (`firmware/lib/vehicle/can/`, `firmware/lib/core/can/`, `firmware/lib/vehicle/ble/`, `packages/protocol/`). If we already have the mechanism, record "already present" and look for the _actual_ gap (freshness gate, fault guard, decode fallback) instead of re-porting the core.
3. **Safety** — any frame mutation, routing, checksum or gate change requires `docs/checklists/can-review-checklist.md` and a listen-only bench capture before it lands. Frame-shape claims need independent validation, not agreement between two docs.
4. **Cost** — prefer extending an existing pipeline in our code over adding a parallel mutation helper. Note the exact our-side symbol that would change.
5. **License** — record upstream license. GPL-3.0 / MIT compatibility does not excuse line-copying; see below.

Output a decision for every mechanism: `candidate for future PR`, `already present`, `no impact`, or `never port` (with reason).

### 6. Never copy code verbatim

- Port **behavior, constants and byte layout**, not source text.
- Use our names, types and style (`Frame`, `State`, our tab/Allman C++ conventions), not upstream identifiers (`CANFRAME`, `buffer`, `fsd_*`).
- Attribute the idea in a comment with the upstream symbol / issue / tool reference.
- Keep an independent test oracle: recompute the checksum or expected frame in the test instead of snapshotting our own output. A passing snapshot test does not prove independent implementation.
- Write `derived idea, independent implementation` where a port is eventually made.
- Never import legacy files into shipping directories (`firmware/`, `client/`, `packages/`, `tools/`).

### 7. Leave a re-verification recipe

The recipe must work after an upstream bump, when line numbers have drifted:

```bash
OLD=<pinned sha>
git -C legacy/<repo> fetch --tags --prune
git -C legacy/<repo> log --oneline $OLD..origin/main
git -C legacy/<repo> diff --stat $OLD..origin/main
git -C legacy/<repo> diff $OLD..origin/main -- src/ include/ test/
git ls-files -s legacy/<repo>          # compare gitlink to rev-parse HEAD
```

Then give grep anchors with **expected hits** for the load-bearing claims. Rules that came out of the pilot audits:

- Use `grep -nE` (extended regex). Plain `grep -n 'a|b|c'` is BRE, where `|` matches a literal pipe and the anchor never hits — a shipped anchor that cannot match is worse than no anchor.
- **Demonstrate every anchor at write time**: run it, confirm the expected hit, paste the expectation. An anchor you did not run is UNVERIFIED.
- Include **negative anchors** for absence claims: `grep -nE '0xAA|XOR' include/uart_bridge.h -> no output`.
- Include **our-side drift anchors** too: the `already present` decisions rest on our files, so those citations need anchors and expected hits just as much as upstream's. Pin our side by symbol, not by line number alone.
- Pin the gitlink in the canonical form from `git ls-files -s legacy/<repo>`: `160000 <full-sha> 0`. One style, everywhere.

```text
grep -nE '<symbol_or_constant>' <file>   -> expect <value/pattern>
```

Add: "If line numbers drift more than ~10, re-read the whole function; do not trust this document's numbers." State whether upstream history before the pinned SHA was reviewed (default: it was not — only the pinned snapshot). List everything UNVERIFIED at the end so the next agent inherits the gaps, not just the conclusions.

## Markdown hygiene (self-check required)

`docs/legacy/**` is ignored by `markdownlint-cli2`, so CI will NOT catch your formatting. Before declaring the doc done:

```bash
grep -nP ' +$' docs/legacy/<repo>.md        # -> no output (no trailing whitespace)
```

And check by eye: blank line above AND below every heading — including `###` mechanism-card headings, the most common miss in the pilot (14 violations in one doc) — plus blank lines around every fence. LF endings only.

## Batch runs (scaling beyond one repo)

When asked to deep-dive many repos:

1. **One agent per repo**, each writing exactly one doc — no shared files. Give each agent: repo path, doc path, mode (replace vs companion), and the preserve list.
2. **Audit pass after the batch**, not during: a separate read-only agent checks each doc against the output contract (17 numbered sections, frontmatter/Upstream preserved, closed grade vocabulary, pinned gitlink, demonstrated anchors, hygiene) and spot-verifies 3 `file:line` claims per doc against source - include at least one claim from the data tables (sections 7-12), not only the mechanism cards. Pilot evidence: 30/30 spot-checks verified, yet 3 docs still shipped defects (14 missing blank lines, 1 dead BRE anchor, 1 inherited trailing space) — structure checks alone are not enough.
3. **Fix defects before committing docs**, keep the audit findings, and fold any new loophole into this skill.

## Quick reference

| Trap                                                    | Counter                                                          |
| ------------------------------------------------------- | ---------------------------------------------------------------- |
| Expanding the old summary instead of reading code       | Read code first; put docs in the contradiction table             |
| Hidden platform dirs skipped by globs                   | `git ls-files` inventory; list unread dirs explicitly            |
| Claim with no citation                                  | Evidence grade required; downgrade to SUMMARY-ONLY/UNVERIFIED    |
| "We should port this" with no check of our code         | Read our counterpart; mark `already present` when true           |
| Copying upstream identifiers or checksum code           | Independent implementation + independent test oracle             |
| Line numbers treated as permanent                       | Pin SHA + grep anchors with expected hits                        |
| Unverified tuning constants presented as fact           | Label UNVERIFIED and name the capture/test that would settle it  |
| Anchor written but never run                            | `grep -nE`, run it, paste the expectation                        |
| BRE alternation anchor (`grep -n` with `\|`) never hits | Use `grep -nE`                                                   |
| Invented grades (`none`, `CODE+TEST`)                   | Closed five-grade vocabulary; absence stated in prose            |
| Skimmed 4000-line DBC labeled "read in full"            | FULL-READ / STRUCTURAL-PARSE / RANGE-CITED, stated per file      |
| Missing blank line after `###` card headings            | Hygiene self-check; CI will not catch it in `docs/legacy/**`     |
| `already present` decision has no our-side anchor       | Our-side drift anchors with expected hits, same as upstream      |
| Frontmatter preserved but factually stale               | Keep bytes; record the staleness in the contradiction table      |
| Silent scope creep over upstream history                | State in section 17 whether pre-pin history was reviewed         |
| Wholesale DBC/JSON dump pasted as "inventory"           | One row per code-referenced frame; catalog gets STRUCTURAL-PARSE |
| Parity row citing our side from memory                  | Our-side CODE-READ citation per row, or the row is SUMMARY-ONLY  |
| Mermaid fence missing `mermaid` tag or blanks           | Tagged fence + blank lines; lint ignores `docs/legacy/**`        |
| History mined only from network                         | Use the submodule's local `git log`; network claims UNVERIFIED   |
| Failure modes guessed from naming                       | Enumerate actual error branches with file:line; untested marked  |

## Related skills and gates

- `legacy-submodule-review` — periodic multi-repo sweep and the two-repo skip list.
- `legacy-submodule-update` — fetch / bump / dead-submodule conversion.
- `commit-ask-first` — no commit, push or PR without explicit approval.
- `markdown-lint` — `docs/legacy/**` is lint-ignored, but keep headings `##`-nested and fences tagged.
- `docs/checklists/can-review-checklist.md` — required for any CAN frame mutation, routing, checksum or transport-visible shape change.
