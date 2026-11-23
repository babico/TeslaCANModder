# Task C1-08 — ALC `0x249` frame shape

Where this fits: Phase 1 safety. Our `buildStalkFrame` emits DLC-3 additive-checksum frames; the offline corpus shows accepted frames are DLC-4 with a different CRC scheme. Wrong frames may be rejected or misread by the vehicle.

## Requirement (verbatim from plan, lines 69-72)

Re-derive DLC, stalk field, counter and CRC from bench capture; rebuild `buildStalkFrame`; walk can-review checklist. **S0 capture-first.**

- Acceptance: replay 1048-frame corpus; generated DLC-4 frame matches accepted frames; checklist signed.
- Evidence: `auto_lane_change.h:48-59`, repo-state `:29` (firmware.md §7.1#6); cross-cutting §4 C7, SC-012.
- Effort: 3–5 h + bench. Risk: High.

## Controller ruling on capture (binding)

No vehicle/bench capture is possible in this environment. Use the read-only offline corpus in `legacy/uhi22-tesla-crc/249_sortedFile.txt` (249 lines of 4-byte frames) and `229_sortedFile.txt` as CAPTURE-grade evidence with provenance recorded from `legacy/uhi22-tesla-crc/readme.md` and `docs/legacy/uhi22-tesla-crc.md`. Do NOT copy any code from `legacy/` into shipping code — derive the CRC algorithm independently, implement it in our style, and cite the corpus as evidence only. Real-vehicle acceptance remains UNVERIFIED; say so in the report. Walk `docs/checklists/can-review-checklist.md` in the report (tick each applicable item with a one-line answer).

## Interfaces / decisions from earlier tasks

- C1-07 (single gate authority) is not done yet. Do not add any new injection gate here — keep existing gate calls intact.
- C1-10 marked some commands UNSUPPORTED; do not re-advertise them.

## Global constraints

- CAN frame mutation → can-review checklist + capture evidence; no code copied from `legacy/`.
- Stage ONLY files you change; 191 unrelated unstaged files exist. Verify staged list before commit.
- Commit/push on the current feature branch; never main, never force-push.
