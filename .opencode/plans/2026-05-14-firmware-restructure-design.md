# Firmware Restructure Design

**Date:** 2026-05-14
**Status:** Approved — Phase 1 complete, Phase 2 in progress

## Summary

Four-phase improvement to the TeslaCANModder firmware: fix stale documentation, reorganize
48 flat feature files into 8 subdirectories, extract shared utilities, and improve the
storage abstraction for multi-board support.

## Phase 1: Fix feature-workflows.md ✅

- Update all stale paths (`infra/` → `vehicle/can/`, `handler/variant/` → `vehicle/can/handler/variant/`)
- Add 19 undocumented features
- Replace old `nag:on|off` + `nag:killer:*` with unified `nag:mode:*` system
- Expand Pattern Compliance Matrix from 32 to 53 entries
- Update File Map with new directory structure

## Phase 2: Group features + split dispatch 🔄

### 2a. Feature directory reorganization

48 flat headers → 8 subdirectories under `firmware/lib/vehicle/can/feature/`:

| Subdir     | Files | Contents                                                                            |
| ---------- | ----- | ----------------------------------------------------------------------------------- |
| fsd/       | 7     | fsd, nag, offsets, profile, isa_chime, region, auto_lane_change                     |
| comfort/   | 8     | climate, seat, wiper, light, display, air_recirc, seatbelt, precondition            |
| drive/     | 5     | pedal, regen, stop, drive_mode, drive_context                                       |
| body/      | 10    | lock, mirror, window, trunk, power, sentry, track_mode, turn_signal, charge, summon |
| telemetry/ | 7     | bms, tpms, powertrain, motor_temps, wheel_speeds, vehicle_config, fw_compat         |
| safety/    | 3     | ban_shield, ban_detect, single_shot                                                 |
| das/       | 2     | das_drive, tlssc                                                                    |
| misc/      | 6     | can_clock, can_raw, can_sim, stream, variant, mqtt_bridge                           |

### 2b. Split dispatch.h

`lib/client/command/dispatch.h` (1065 lines) → `lib/client/command/dispatch/` with per-category files:

```
command/dispatch/
  system.h      ping, status:*, status:live:*
  fsd.h         fsd:*, nag:*, profile:*, offset:*, isa-chime:*, region:*, ecer79:*, alc:*
  comfort.h     climate:*, seat:*, wiper:*, light:*, display:*, air_recirc:*, seatbelt:*, precondition:*, wiperpersist:*
  drive.h       pedal:*, regen:*, stop:*, drivemode:*, drive_context:*
  body.h        lock:*, mirror:*, window:*, trunk:*, power:*, sentry:*, trackmode:*, turn:*, charge:*, summon:*
  telemetry.h   bms, tpms, powertrain, motor_temps, wheel_speeds, vehicle, platform, fwcompat
  safety.h      banshield:*, gtwshield:*, apgate:*, singleshot:*
  das.h         drive:*, gamepad:*, tlssc:*
  misc.h        canclock:*, variant:*, stream:*, can:raw:*, simu:*, mqtt:*
```

Master `dispatch.h` includes all and calls each `dispatchXxx()` in order.

### 2c. Sub-feature splits (3 files > 400 lines)

- **nag.h (483 lines)** → `fsd/nag/math.h`, `checksum.h`, `strategies.h`, `nag.h`
- **bms.h (596 lines)** → `telemetry/bms/power.h`, `soc.h`, `thermal.h`, `energy.h`, `bms.h`
- **das_drive.h (542 lines)** → `das/das_drive/frames.h`, `safety.h`, `nvs.h`, `das_drive.h`

Files inside subdirectories use short names (no redundant prefix): `math.h` not `nag_math.h`.

### 2d. Include path updates

Files needing path updates:

- `client/command/features.h` — 44 includes
- `client/command/dispatch.h` — all feature includes
- `vehicle/can/handler/bus/vehicle.h` — 16 includes
- `vehicle/can/handler/bus/chassis.h` — 3 includes
- `vehicle/can/handler/ticks.h` — 2 includes
- `vehicle/can/handler/helpers.h` — 1 include
- `vehicle/can/handler/filters.h` — 1 include
- `client/gamepad/state.h` — 1 include
- All 48 feature file `@file` doxygen comments

## Phase 3: Extract shared utilities

- Move `nagChecksum()`, `driveChecksum()`, `hw4IsaChecksum()` to `vehicle/can/checksum.h`
- Feature files include the shared header instead of redefining
- No runtime behavior change

## Phase 4: Storage abstraction

- Add `persist/native/board.h` for native test persistence
- Extract NVS key names to `persist/keys.h` (single source of truth)
- Replace manual load/save pairs with key-value table
- Incremental version migration (keep old keys, add new defaults)

## Constraints

- No command names change
- No runtime logic changes
- No API changes (same function signatures)
- Git mv for all file moves (preserves history)
- Build must pass for all ESP32 environments after each phase
- Native tests must pass after each phase
