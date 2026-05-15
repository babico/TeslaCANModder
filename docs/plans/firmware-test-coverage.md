---
plan name: firmware-test-coverage
plan description: Add missing firmware tests
plan status: done
---

## Idea

Add native PlatformIO Unity tests for 15 headers that contain pure logic but lack test coverage. Grouped into 3 phases: HIGH priority (checksum, ids, types, nag math, frame readers), MEDIUM priority (ban detect, motor temps, wheel speeds, id filter, ring buffer, recorder), LOW priority (burst, parse, handler helpers).

## Implementation

- Phase 1 (HIGH): Create test_native_checksum for vehicle/can/checksum.h — test dasChecksum, computeHW4IsaChecksum, nagChecksum, driveChecksum with known inputs and DLC boundary guards
- Phase 1 (HIGH): Create test_native_ids for vehicle/can/ids.h — test readFollowDistance and isFSDSelectedInUI with all bit values and short frames
- Phase 1 (HIGH): Create test_native_types for core/types.h testable subset — test setBit, readMuxID, variantName/parseVariant roundtrip, getFeatures per variant, nag mode predicates, apGateOpen logic
- Phase 1 (HIGH): Create test_native_nag_math for vehicle/can/feature/fsd/nag/math.h — test deterministic PRNG sequence, randFloat range, Gaussian distribution properties
- Phase 1 (HIGH): Create test_native_frame_readers for vehicle/can/handler/frame_readers.h — test readDASAutopilotStatus, readDASAutopilotState, isDASAutopilotActive, readGtwAutopilotTier with valid/short frames
- Phase 2 (MEDIUM): Create test_native_ban_detect for vehicle/can/feature/safety/ban_detect.h — test apTierName, checkBanDetection with all tier transitions, threat levels, counter increments
- Phase 2 (MEDIUM): Create test_native_motor_temps for vehicle/can/feature/telemetry/motor_temps.h — test all 6 decoders with known raw values (0→-40, 40→0, 215→175)
- Phase 2 (MEDIUM): Create test_native_wheel_speeds for vehicle/can/feature/telemetry/wheel_speeds.h — test all 4 decoders with known bit patterns, 8191 sentinel, zero/max speed
- Phase 2 (MEDIUM): Create test_native_id_filter for transport/can/id_filter.h — test add/test/remove cycle, boundary IDs, clear, swFilterAccept per bus
- Phase 2 (MEDIUM): Create test_native_ring_buffer for transport/can/ring_buffer.h — test push/peek/advance, overflow detection, reset, multiple consumers
- Phase 2 (MEDIUM): Create test_native_recorder for transport/can/recorder.h — test capture/retrieve, wrap-around, start/stop/reset, disabled recorder
- Phase 3 (LOW): Create test_native_burst for vehicle/can/burst.h — test txPaused guard, apGateOpen guard, successful burst setup
- Phase 3 (LOW): Create test_native_parse for core/util/parse.h — test parseBoolCmd with on/off/unknown strings
- Phase 3 (LOW): Create test_native_handler_helpers for transport/can/handler/helpers.h — test \_updateCanFrameRate math
- Run `npm run test:firmware` to verify all new tests pass, fix any compilation or test failures

## Required Specs

<!-- SPECS_START -->
<!-- SPECS_END -->
