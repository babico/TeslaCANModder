---
title: codethaumaturge-911-tesla-gauges
description: A project to digitally control stock instrument panel gauges on a classic Porsche 911 using Tesla drive unit CAN data, d
category: legacy
folder: legacy
tags: [legacy, community, external]
author: codethaumaturge
repo: 911-tesla-gauges
---

# codethaumaturge-911-tesla-gauges

## 1. Provenance

- **Submodule path:** `legacy/codethaumaturge-911-tesla-gauges`
- **Pinned gitlink (canonical form):** `160000 150182ad493a169102d2a1a84264639ee043985c 0`
- **Snapshot check:** `git -C legacy/codethaumaturge-911-tesla-gauges rev-parse HEAD` returns `150182ad493a169102d2a1a84264639ee043985c` — matches gitlink. `status --porcelain` is empty; tree is clean.
- **Upstream URL:** `origin` = `https://github.com/codethaumaturge/911-tesla-gauges` (fetch + push). Local `main`, `origin/main`, `origin/HEAD` all at the pinned SHA.
- **License:** MIT — `LICENSE:1` (`MIT License`), `LICENSE:3` (`Copyright (c) 2021 codethaumaturge`).
- **Analysis date:** 2026-11-21.
- **Files read:** `README.md` lines 1-2 (FULL-READ, 2 lines), `LICENSE` lines 1-21 (FULL-READ, 21 lines). Pre-replace summary read via `git show HEAD:docs/legacy/codethaumaturge-911-tesla-gauges.md` for the contradiction pass.
- **Files explicitly NOT read:** None — the repo contains exactly two tracked files and both were read in full. A recursive listing (including hidden entries) found only `.git` metadata beyond those two; no hidden platform directories, no vendored DBC/JSON catalogs, no generated code.
- **Frontmatter handling:** preserved byte-for-byte in values by replace mode. The `description` value is truncated mid-token (`... CAN data, d`); it carries **no** trailing whitespace (verified hex: `... 64 0D` = `d` + CR), so it was left unchanged. No `## Upstream (...)` sections existed in the pre-replace doc, so none were preserved.
- **Line-ending note:** source doc was CRLF; this replacement uses LF per hygiene requirement.

## 2. Executive verdict

`codethaumaturge/911-tesla-gauges` is a documentation-only concept stub: a single-line README stating that a Raspberry Pi digitally drives stock instrument-panel gauges of a classic Porsche 911 from Tesla drive-unit CAN data, plus an MIT LICENSE. There is no firmware, client, protocol, tooling, schematic, DBC, or capture artifact — therefore no CAN frame pipeline, no decode/mutate/gate/checksum mechanism, and no surface to port. Mechanism catalogue, frame/signal inventory, parity matrix, and do-not-copy list are all empty by construction. Verdict: **archival concept only — no impact on TeslaCANModder; nothing is an adaptation candidate.** The sole adjacency is thematic (reading Tesla drive-unit CAN data), which is SUMMARY-ONLY and cannot drive any decision.

## 3. Evidence map

| Source file | Lines | What it proves | Grade |
| --- | --- | --- | --- |
| `README.md` | 1-2 | Project title and the entire concept statement: "Digital control by Raspberry Pi of stock instrument panel gauges for a classic Porsche 911 using Tesla drive unit CAN data." No implementation detail follows. | SUMMARY-ONLY |
| `LICENSE` | 1, 3 | MIT license grant text; copyright `2021 codethaumaturge`. License compatible with GPL-3.0 distribution of notes, but no code exists to copy. | CODE-READ |

Gitlink pin, clean tree, and grafted single-commit history are recorded in §1 and §15; they are provenance facts, not behavior claims. No TEST-READ evidence exists: this repo ships no tests. No CAPTURE evidence exists: no frames, logs, or decode artifacts are present. No behavior claim rises above SUMMARY-ONLY because there is no implementation to read.

## 4. Architecture and build surface

- **Entry points:** None found — no source files of any kind.
- **Shared core vs platform port:** None found.
- **Build flags / feature gating:** None found.
- **Dependency boundaries:** None found.
- **Data-flow diagram:** None found: no frame pipeline in this repo.

The only described data flow is the README's prose intent (Tesla drive-unit CAN data → Raspberry Pi → Porsche 911 stock gauges); it is not backed by code or a diagram in the upstream repo.

## 5. Mechanism catalogue

None found. The repo contains no implementation, so no mechanism card can be populated — no frame builder, counter/checksum helper, state machine, gate, driver, or protocol contract exists to cite.

## 6. Summary vs code contradiction table

None found. The two tracked files contain no code-level claims to contradict; the pre-replace doc (`git show HEAD:docs/legacy/codethaumaturge-911-tesla-gauges.md`) and the README agree on the doc-only status, Raspberry Pi concept, and MIT license. No frontmatter staleness: the description still matches the README. The mid-token description truncation is a data-quality defect, not a contradiction, and was preserved per replace mode.

## 7. CAN frame and signal inventory

None found. No frame is decoded, mutated, injected, or vendored. The README mentions "Tesla drive unit CAN data" only as a concept (SUMMARY-ONLY) — no CAN ID, bus, DLC, bit layout, scale, counter, or checksum is stated anywhere in the repo. No DBC/JSON catalog exists to label STRUCTURAL-PARSE.

## 8. Constants and tuning table

None found. No load-bearing magic number shapes a frame, gate, or timing window. (`2021` in `LICENSE:3` is a copyright year, not a tuning constant.)

## 9. Failure modes and edge cases

None found. There is no bus-off handling, watchdog, OTA behavior, queue overflow, brownout/reboot state, or missing-frame timeout because no runtime code exists. No error path exists, tested or untested.

## 10. Test coverage map

None found. No test files and no test runner are present; no critical code paths exist to be untested.

## 11. Parity matrix with TeslaCANModder

None found. Upstream defines no frame or signal, so there is no row that could carry both an upstream citation and a our-side CODE-READ citation. No mechanism qualifies as "already present" — that decision requires a code-level mechanism on both sides, and none exists upstream. Concept-level adjacency (our firmware also reads Tesla CAN data) is SUMMARY-ONLY and is recorded here as a non-actionable observation, not a parity row.

## 12. Surface inventory and security notes

None found. No serial, web, BLE, or other command/endpoint surface is exposed. No hardcoded credentials, keys, or certificates; no untrusted archives; no dangerous build flags. The repo is two static text files with no executable content.

## 13. Adaptation assessment

None found — no mechanisms to score for value / cost / risk / evidence grade / decision. Repo-level decision: **no impact** (archival concept stub; all project-behavior evidence is SUMMARY-ONLY, which the skill forbids as a basis for porting).

## 14. Do-not-copy list

None found. No source text, frame layout, checksum, or constant exists that could be copied into shipping `firmware/`, `client/`, `packages/`, or `tools/` directories.

## 15. History and evolution

Local `git log --all --oneline --decorate` shows exactly one commit: `150182a (grafted, HEAD -> main, origin/main, origin/HEAD) Initial commit`. The clone is grafted/shallow, so no pre-pin history is available locally. There is no changelog, no release tag beyond the branch pointers, and no record of approaches tried, abandoned, or rolled back. Network-only claims (upstream issues, PRs, deleted branches, or a later source release) are **UNVERIFIED** — not consulted for this analysis.

## 16. Re-verification recipe

```bash
OLD=150182ad493a169102d2a1a84264639ee043985c
git ls-files -s legacy/codethaumaturge-911-tesla-gauges            # -> 160000 150182ad493a169102d2a1a84264639ee043985c 0
git -C legacy/codethaumaturge-911-tesla-gauges rev-parse HEAD      # -> same SHA
git -C legacy/codethaumaturge-911-tesla-gauges status --porcelain  # -> empty
git -C legacy/codethaumaturge-911-tesla-gauges fetch --tags --prune
git -C legacy/codethaumaturge-911-tesla-gauges log --oneline $OLD..origin/main
git -C legacy/codethaumaturge-911-tesla-gauges diff --stat $OLD..origin/main
git -C legacy/codethaumaturge-911-tesla-gauges diff $OLD..origin/main -- src/ include/ test/
git -C legacy/codethaumaturge-911-tesla-gauges ls-files            # -> LICENSE, README.md only (watch for new source files)
```

Grep anchors — demonstrated at write time 2026-11-21. GNU `grep` is unavailable in this Windows shell, so the identical ERE patterns were run with ripgrep (the repo's `grep` tool); results below are the observed output.

```text
grep -nE 'Raspberry Pi|CAN data|instrument panel' legacy/codethaumaturge-911-tesla-gauges/README.md
  -> line 2: Digital control by Raspberry Pi of stock instrument panel gauges for a classic Porsche 911 using Tesla drive unit CAN data.

grep -nE 'MIT License|Copyright \(c\) 2021 codethaumaturge' legacy/codethaumaturge-911-tesla-gauges/LICENSE
  -> line 1: MIT License
  -> line 3: Copyright (c) 2021 codethaumaturge

grep -nE '0x[0-9A-Fa-f]{3}|CAN_ID|#include|\.ino|\.cpp|\.h\b' legacy/codethaumaturge-911-tesla-gauges/README.md legacy/codethaumaturge-911-tesla-gauges/LICENSE
  -> no output (negative anchor: no source files, no CAN IDs, no includes)
```

Our-side drift anchors: None found — no "already present" decision and no parity row exists, so there is no our-side symbol to pin. If a future upstream bump adds source, re-run the full pipeline (procedure §2, §4-§7 of the skill) before reusing any number here.

If line numbers drift more than ~10, or if `git ls-files` shows any file beyond `LICENSE` and `README.md`, re-read the whole files and redo the mechanism/frame inventories; do not trust this document's numbers. Upstream history before the pinned SHA was **not** reviewed — the local clone is grafted and contains no such history.

## 17. Open questions and explicitly not investigated

- Whether upstream ever published implementation source outside this repo (separate branch, release asset, or another repository) — **UNVERIFIED**; would require network/issue review, which was not performed.
- Whether the project was abandoned, completed privately, or superseded — **UNVERIFIED**; no local evidence.
- Pre-pin upstream history — not investigated; none exists locally (grafted single commit).
- No CAPTURE or TEST-READ evidence exists; no bench capture could settle frame-level questions because no frames are defined.
- The thematic link to our Tesla drive-unit CAN decoding was not investigated past noting the absence of shared signals; it is SUMMARY-ONLY and was deliberately not turned into a parity row or adaptation decision.
