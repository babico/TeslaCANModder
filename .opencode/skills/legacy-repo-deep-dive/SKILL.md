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

Turns a legacy research repo into a **trusted, reusable mechanism catalogue**. The goal is not a better summary — it is deciding which solutions to reinterpret for TeslaCANModder, with enough evidence that a future agent can re-check the claims after an upstream bump.

Core principle: **code and tests are evidence; README and `docs/legacy/*.md` are leads.** A claim without a file/symbol reference is not an analysis finding.

Use this for one repo at a time. Use `legacy-submodule-review` for the periodic multi-repo sweep; use `legacy-submodule-update` for fetch/bump/dead-submodule conversion.

## Output contract

The analysis must be a document with these parts, in this order. This is a shape, not a suggestion — a future agent should be able to find each section by name.

1. **Provenance** — submodule path, pinned gitlink SHA, upstream URL, license, analysis date, files read (with line ranges), files explicitly NOT read.
2. **Executive verdict** — one paragraph: what the repo is, which mechanisms matter, which are candidates, which must not be ported.
3. **Evidence map** — table: `source file | lines | what it proves | grade`.
4. **Architecture and build surface** — entry points, shared core vs platform port, build flags, dependency boundaries.
5. **Mechanism catalogue** — one card per mechanism (template below).
6. **Summary vs code contradiction table** — every place the existing `docs/legacy/<repo>.md` or upstream README disagrees with the code.
7. **Adaptation assessment** — per mechanism: value / cost / risk / evidence grade / decision.
8. **Do-not-copy list** — with file:line and reason for each item.
9. **Re-verification recipe** — exact commands, grep anchors with expected hits, pinned SHA.
10. **Open questions and explicitly not investigated.**

If a section has no content, write `None found` — do not silently drop it.

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

Then give grep anchors with **expected hits** for the load-bearing claims:

```text
grep -n '<symbol_or_constant>' <file>   -> expect <value/pattern>
```

Add: "If line numbers drift more than ~10, re-read the whole function; do not trust this document's numbers." List everything UNVERIFIED at the end so the next agent inherits the gaps, not just the conclusions.

## Quick reference

| Trap                                              | Counter                                                         |
| ------------------------------------------------- | --------------------------------------------------------------- |
| Expanding the old summary instead of reading code | Read code first; put docs in the contradiction table            |
| Hidden platform dirs skipped by globs             | `git ls-files` inventory; list unread dirs explicitly           |
| Claim with no citation                            | Evidence grade required; downgrade to SUMMARY-ONLY/UNVERIFIED   |
| "We should port this" with no check of our code   | Read our counterpart; mark `already present` when true          |
| Copying upstream identifiers or checksum code     | Independent implementation + independent test oracle            |
| Line numbers treated as permanent                 | Pin SHA + grep anchors with expected hits                       |
| Unverified tuning constants presented as fact     | Label UNVERIFIED and name the capture/test that would settle it |
| `docs/legacy/` edited as part of the sweep docs   | Keep this deep-dive doc separate; cross-link instead            |

## Related skills and gates

- `legacy-submodule-review` — periodic multi-repo sweep and the two-repo skip list.
- `legacy-submodule-update` — fetch / bump / dead-submodule conversion.
- `commit-ask-first` — no commit, push or PR without explicit approval.
- `markdown-lint` — `docs/legacy/**` is lint-ignored, but keep headings `##`-nested and fences tagged.
- `docs/checklists/can-review-checklist.md` — required for any CAN frame mutation, routing, checksum or transport-visible shape change.
