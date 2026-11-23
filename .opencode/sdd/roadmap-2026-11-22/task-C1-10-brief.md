# Task C1-10 — Advertised-but-unimplemented command decision

Where this fits: Phase 1 correctness. 21 NO_HANDLER builders, ~31 dead wire strings and 29 BLE-gated builders are advertised to users but do nothing (or return `Unknown command` at `dispatch.h:1085`). Decide per command and make the contract honest.

## Requirement (verbatim from plan, lines 77-80)

For 21 NO_HANDLER builders / ~31 dead wire strings / 29 BLE-gated builders: implement a minimal safe handler, alias (`apfirst`→`ap-first`, `lanegraph`→`lane-graph`), or delete builder + spec + docs together. Move `log`/`platform`/`recorder:*` above the `!s.hasCtrl` return.

- Acceptance: grep-based CI check — every command in `docs/reference/commands.md` has a dispatch branch or is marked unsupported; fresh-boot command test with no CAN traffic.
- Evidence: cross-cutting §2.2/§2.3, §1 D14, §8; client-protocol §5, §7 N1/N2; `dispatch.h:422-427,1085`.
- Effort: 6–20 h. Risk: Medium–High.

## Evidence to read before deciding

- `.opencode/analysis/current-state-2026-11-22/cross-cutting.md` §2.2/§2.3 and the command verdict list.
- `.opencode/analysis/current-state-2026-11-22/client-protocol.md` §5, §7 N1/N2.
- Firmware dispatch: `firmware/lib/client/command/dispatch.h` (especially `:422-427`, `:1085`).
- Protocol registry: `packages/protocol/src/commands.ts`, `gating.ts`; docs: `docs/reference/commands.md`.

## Decisions you must make (and record in the report)

For EVERY advertised command with no real firmware effect, choose one and justify in one line:

1. **implement** a minimal safe handler (only if behavior is unambiguous and testable),
2. **alias** to the canonical firmware spelling (`apfirst`→`ap-first`, `lanegraph`→`lane-graph`),
3. **delete** the builder + protocol entry + docs row together (mark unsupported in docs only if keeping the wire name for compatibility).

Do not silently leave a command advertised without a decision. Do not add new features beyond the decision.

## Global constraints

- No code copied from `legacy/`.
- Any CAN-affecting change needs can-review checklist + capture; prefer alias/delete over new injection behavior here.
- Stage ONLY files you change; 191 unrelated unstaged files exist. Verify staged list before commit.
- Commit/push on current feature branch; never main, never force-push.
- Add the grep-based CI check as a test or script in the repo, not a one-off command in the report.
