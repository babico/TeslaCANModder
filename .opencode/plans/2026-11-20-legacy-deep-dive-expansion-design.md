# Legacy deep-dive expansion — design (2026-11-20)

## Intent

The maintainer wants each `legacy/` repo analysis to extract everything useful, not just the mechanism catalogue: full frame/signal data, constants, failure paths, test gaps, parity with our decoders, exposed surfaces, security notes, and upstream history. Goal: make the docs the single research base for adapting upstream solutions into TeslaCANModder.

## Why

The pilot batch (10 repos, 2026-11-20) proved the method (30/30 spot-checked claims verified) but the contract stops at mechanisms. The highest-value mining targets — frame tables, parity deltas, dead ideas in history — were only partially captured inside prose cards.

## Goals

- Every deep-dive doc extracts: frame/signal inventory, constants table, failure/edge cases, test coverage map, upstream-vs-our parity matrix, surface + security inventory, history/evolution, and diagrams.
- Evidence grades and closed vocabularies apply to all new sections.
- `None found` rule covers repos that lack a given dimension (DBC-only repo has no commands).

## Non-goals

- No code copied into shipping code. No vehicle testing. No wholesale DBC dumps pasted as "analysis".
- Not splitting docs into per-repo file trees; one doc per repo stays the unit.

## Approaches considered

1. **Single doc, expanded numbered contract (picked).** 17 sections in one `docs/legacy/<repo>.md`. Single source, greppable, audit-able; cost: docs grow to ~50-70 KB.
2. Split into `docs/legacy/deep/<repo>/` sub-files (frames.md, parity.md, ...). Cleaner per topic, but multiplies navigation and audit surface for 90+ repos.
3. Tiered (mandatory vs optional per repo type). Rejected: pilot showed agents already handle missing dimensions well via `None found`; tiers would add negotiation room.

## Decisions

- Section order: evidence first, data tables next, parity before adaptation decisions, history near the end, re-verification last.
- Frame inventory caps: list frames the repo actually decodes/mutates/injects; a vendored DBC catalog is labeled `STRUCTURAL-PARSE` and not dumped wholesale.
- Diagrams: one mermaid data-flow in the architecture section; a mermaid state diagram only for mechanisms with 3+ states.
- History mining: local `git log` of the submodule (already full clones); network-dependent claims stay UNVERIFIED.
- Our-side parity: read our decoder first (`firmware/lib/vehicle/can/`, `packages/protocol/`); a parity row without our-side CODE-READ is SUMMARY-ONLY and cannot drive a decision.

## Left to the builder

- Pilot 10 docs predate sections 7-12 and 15; they need a delta pass (add missing sections only) before the corpus is uniform.
