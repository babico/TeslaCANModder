---
name: refactor-over-duplication
description: Detect when adding a new variant/case to existing firmware code would duplicate established patterns, and require a refactor into a common pipeline before adding the new case. Prevents the "4th copy-paste apply function" anti-pattern in TeslaCANModder nag, nag-killer, and feature-handler code.
license: GPL-3.0
compatibility: opencode
metadata:
    area: firmware
    priority: high
---

## What I do

Stop and refactor before adding a new variant that would copy an existing per-mode/per-case pattern. The TeslaCANModder codebase has nag modes, command modes, and feature handlers that historically grew by copy-paste. Adding a 4th, 5th, or 6th copy of the same boilerplate is explicitly rejected — the right answer is to extract a common pipeline (compute / apply) once, then add the new case as a single function or a single switch arm.

## When to use me

Use this skill when a task adds a new variant to a family that already has 3+ similar variants, and the new variant would copy any of:

- DLC guards (`if (f.dlc < 8) return;`)
- Counter / sequence increment logic
- Checksum recalculation
- Bit-field writes with mask-and-set patterns like `data[x] = (data[x] & ~MASK) | VALUE`
- The "compute a value, write it to a frame" two-step pattern
- Gate logic (mode-eligible, conditions-pass, return-bool)

Concretely in this repo: nag modes (`firmware/lib/vehicle/can/feature/fsd/nag.h`), drive modes, nag-killer strategies, pedal/regen/stop/light enums, etc.

## Hard rules

1. **Detect the pattern.** Before adding a new case, count the existing per-variant functions in the family. If the family has 3+ cases and the new variant would be the 4th+ copy of the same boilerplate, this skill applies.
2. **Pause and present the refactor plan.** Show:
    - The duplicated boilerplate that would be added
    - A proposed common pipeline (e.g., `NagTorque` struct + `nagEchoCompute` switch + `nagEchoApply` that owns the boilerplate)
    - A byte-level equivalence argument: every existing variant produces the same byte sequence as before the refactor
    - The can-review-checklist walk if the refactor touches a CAN-mutating file (it usually does)
3. **Get explicit approval before refactoring.** The user has approved this pattern in the past (the `nag:mode:feifan` work collapsed `nagApplyZeroTorque`, `nagApplyNaturalTorque`, `nagOrganicApply` into one `nagEchoApply`). But "add a new mode" and "refactor to a common pipeline" are different scopes — both are valid; the user chooses.
4. **Adding a new mode is now a single switch arm, not a new function file.** After the refactor, the new variant should be expressible as:
    - One new case in the gate switch (or no new case if the existing natural-mode gate fits)
    - One new case in the compute switch returning a payload struct
    - Zero new boilerplate in apply
5. **Byte-level regression tests are mandatory.** The refactor must produce identical output bytes for every existing variant. Add a `test_*_byte_equivalence_through_pipeline` test per mode, plus the new mode's specific assertions.
6. **Watch for the "dropped check" class of bug.** When collapsing 3+ per-mode functions into one, it is easy to drop a guard (e.g., the `!s.dasSeen` check that existed in `nagOrganicShouldEcho` but was not carried into the unified gate). The existing gate/state/tick tests are the safety net; run them after every refactor.

## Anti-patterns to flag and reject

- **The 4th apply function.** If a task is "add `nag:mode:feifan`" and the current file has `nagApplyZeroTorque`, `nagApplyNaturalTorque`, `nagOrganicApply`, then writing `nagFeifanApply` is the anti-pattern. Refactor first.
- **The per-case DLC guard.** If three apply functions each open with `if (f.dlc < 8) return;`, the refactored apply does it once.
- **The per-case checksum line.** Same logic — one checksum at the bottom of the unified apply.
- **The per-case counter increment.** Pass the counter byte as part of the payload struct, increment in the unified apply.

## Acceptable as a "no-refactor" decision

- The family has 1–2 cases and the new one is genuinely different in shape.
- The user explicitly says "just add it, no refactor".
- The refactor scope would be larger than the feature (e.g., touching code outside the immediate family that we don't want to risk).

## Ask the user when

- You are about to add the 4th+ copy of a per-variant function family
- You are about to add a new bit/byte mutation that overlaps with an existing one
- The refactor scope is large enough that you cannot prove byte-level equivalence without a full test-suite pass

## Why this exists

The user explicitly said: _"so you bring new modes bring with more better implementation you just patching old code"_ after an attempt to add `nag:mode:feifan` as a 4th `nagFeifanApply` next to three existing apply functions. The right answer was to extract `nagEchoApply` first, then add the new mode as a single switch arm. This skill makes that reflex automatic.
