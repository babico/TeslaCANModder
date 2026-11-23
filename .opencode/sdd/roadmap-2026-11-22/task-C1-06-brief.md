# Task C1-06 — OTA TX guard decision

Where this fits: Phase 1 safety. We advertise OTA detection / TX pause (`0x318`, `otaInProgress`, `txPaused`) but no handler exists. Worse, fresh evidence disputes the frame identity entirely (C14: `0x318` may be GTW_carState or an RTC frame depending on model/era).

## Requirement (verbatim from plan, lines 61-64)

Decide implement vs delete from docs/schema; if implement, decode 0x318 OTA bit, set `s.txPaused`, and check centrally in TX/burst paths. **Capture-gated:** C14 disputes that 0x318 is GTW_carState at all.

- Acceptance: native test feeding a real 0x318 capture with OTA bit asserts burst/drive-mode TX count == 0; OR all advertised OTA claims removed.
- Evidence: `ids.h:52`, `io.schema.json:1205-1206`, `core/types.h` (firmware.md §7.1#1); cross-cutting §4 C14, SC-014.
- Effort: 3–5 h + bench. Risk: High.

## Controller ruling (binding)

No real vehicle capture exists here and the fresh review found contradictory model/era evidence for `0x318`. Implementing a decode now would risk pausing or failing to pause TX at the wrong time. Choose the **removal** branch of the acceptance criterion:

1. Remove all advertised OTA guard / TX-pause claims from `docs/`, `packages/protocol` command/schema and status fields where they describe implemented behavior. Keep the raw `CAN_ID_GTW_CAR_STATE` define in `ids.h` as a documented ID (do not delete the ID itself).
2. Add a contract test (like C1-13's MQTT test) that asserts docs/schema no longer advertise OTA pause, so the claim cannot silently reappear.
3. Record in the report that real implementation is CAPTURE_FIRST with the exact disputed evidence (cross-cutting C14) and the capture needed to settle it.
4. Do NOT add any new TX gate or frame decode in this task.

## Global constraints

- No code copied from `legacy/`.
- Stage ONLY files you change; 191 unrelated unstaged files exist. Verify staged list before commit.
- Commit/push on the current feature branch; never main, never force-push.
