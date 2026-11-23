# Batched firmware fixes — C1-05, C1-11, C1-12, C1-13

Four small independent firmware correctness fixes. Implement all four, each with its own native test, in ONE commit. Re-verify each against current code first; mark stale items in the report and skip.

## C1-05 — CarServer seat encode

Use `fieldVarint` with correct enum field numbers instead of `fieldEmpty(level)` / `fieldEmpty(position)`.

- Acceptance: wire-byte assertion on `buildSeatHeaterAction` output (old P1-001).
- Evidence: `firmware/lib/vehicle/ble/carserver.h:222-225` (firmware.md §7.1#7).

## C1-11 — NVS key integrity

Add keys for `region:spoof:*`, `alc`, `ap-first`, `rawCanListen`; bump `NVS_SETTINGS_VERSION`; load/save in both persist backends.

- Acceptance: reboot round-trip tests for all four settings (old P1-004).
- Evidence: `region.h:175-185`, `auto_lane_change.h:126-134`, `dispatch.h:619-635,138-144`, `keys.h:11-58`, `nvs.h:63-101,120-156` (cross-cutting §3).

## C1-12 — 0x334 pedal/drive-mode collision

Allocate non-overlapping bit fields; rebuild from `lastDrive`.

- Acceptance: `test_native_multi_feature` sets both and asserts both bitfields survive (old P1-006).
- Evidence: `pedal.h:38-48` vs `drive_mode.h:94-110`.

## C1-13 — MQTT publish loop

Wire `mqttShouldPublish` into a tick and set `mqttConnected`, OR remove MQTT claims from docs/schema.

- Acceptance: telemetry publish observable with broker up; `mqttConnected == 1`.
- Evidence: `mqtt_bridge.h:81`, `core/types.h:791`, `messages.h:201,451` (cross-cutting §2.2, N8).

## Global constraints

- No code copied from `legacy/`.
- CAN frame mutation needs can-review checklist + capture. C1-12 touches a 0x334 build path — keep the existing counter/checksum behavior; do not introduce new injection. Mark can-review as required in the report.
- Stage ONLY files you change; 191 unrelated unstaged files exist. Verify `git diff --cached --name-only` before commit.
- Commit and push on the current feature branch; never main, never force-push.
