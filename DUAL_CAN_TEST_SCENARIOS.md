# Dual-CAN Bus Test Scenarios

## Test Setup
- Arduino Uno with 2x MCP2515 modules
- Bus 0 (VehicleBus): FSD CAN frames (0x3F8, 0x3FD, 0x273, etc.)
- Bus 1 (PowertrainBus): Drive/Climate/Charge frames (0x334, 0x2F3, 0x333, 0x273)

## Scenario 1: Bus 2 Init Failure Detection
**Goal:** Verify bus 2 init failure is handled gracefully

**Setup:**
1. Wire only Bus 0 (MCP2515 on CS pin 10)
2. Leave Bus 1 disconnected or use wrong CS pin
3. Flash firmware with `uno_full` environment

**Expected Behavior:**
- Board boots successfully
- `driverInit()` returns true (bus 0 OK)
- `bus2Available` flag is false
- All commands to bus 1 are ignored (fallback to bus 0)
- No crashes or hangs

**Test Commands:**
```
status
pedal:sport    # Should fail gracefully (no bus 1)
lock           # Should work (uses bus 0 via ctrlBus)
```

**Debug Output to Check:**
- Serial monitor shows boot message
- No error messages about bus 2
- Commands execute without crashes

---

## Scenario 2: Dual-CAN with 0x273 on Bus 0
**Goal:** Verify vehicle commands use correct bus when 0x273 is on bus 0

**Setup:**
1. Both buses connected
2. 0x273 UI_vehicleControl frames coming from Bus 0
3. 0x334 Drive Config frames coming from Bus 1

**Expected Behavior:**
- `s.ctrlBus` is set to 0 when 0x273 is received on bus 0
- All mirror/lock/light/wiper/seat/display/power commands send to bus 0
- Drive config commands (pedal/regen/stop) send to bus 1

**Test Commands:**
```
status
lock           # Should send to bus 0
mirror:fold    # Should send to bus 0
pedal:sport    # Should send to bus 1 (if 0x334 cached)
```

**Debug Output to Check:**
- Watch CAN traffic on both buses with logic analyzer
- Verify 0x273 commands go to bus 0
- Verify 0x334 commands go to bus 1

---

## Scenario 3: Dual-CAN with 0x273 on Bus 1
**Goal:** Verify vehicle commands use correct bus when 0x273 is on bus 1

**Setup:**
1. Both buses connected
2. 0x273 UI_vehicleControl frames coming from Bus 1
3. FSD frames (0x3FD) coming from Bus 0

**Expected Behavior:**
- `s.ctrlBus` is set to 1 when 0x273 is received on bus 1
- All mirror/lock/light/wiper/seat/display/power commands send to bus 1
- FSD mux modifications still work on bus 0

**Test Commands:**
```
status
lock           # Should send to bus 1
mirror:fold    # Should send to bus 1
fsd:on         # FSD mux on bus 0 still works
```

**Debug Output to Check:**
- Watch CAN traffic on both buses
- Verify 0x273 commands go to bus 1
- Verify FSD mux modifications go to bus 0

---

## Scenario 4: Bus 2 Filter Verification
**Goal:** Verify bus 2 only receives filtered IDs, not all frames

**Setup:**
1. Both buses connected
2. Bus 1 has heavy traffic (100+ frames/sec)
3. Enable CAN streaming: `stream:on`

**Expected Behavior:**
- Bus 0 filters: 0x399 (ISA), 0x3F8 (FOLLOW_DIST), 0x3FD (FSD_MUX), 0x273 (UI_VEHICLE_CTRL)
- Bus 1 filters: 0x2F3 (CLIMATE), 0x333 (CHARGE), 0x334 (DRIVE_CONFIG), 0x273 (UI_VEHICLE_CTRL)
- Only filtered frames are processed
- No CPU overload from irrelevant frames

**Test Commands:**
```
can:raw:off    # Apply filters
stream:on      # Enable streaming
# Wait 10 seconds
stream:off
```

**Debug Output to Check:**
- Frame count should be low (only filtered IDs)
- No frames with IDs outside filter list
- Board remains responsive

---

## Scenario 5: Bus 2 Promiscuous Mode (Raw CAN)
**Goal:** Verify bus 2 passes all frames when raw CAN is enabled

**Setup:**
1. Both buses connected
2. Enable raw CAN mode: `can:raw:on`

**Expected Behavior:**
- Both buses set to pass-all filters (0x000 mask)
- All frames from both buses are received
- Frame streaming shows all IDs

**Test Commands:**
```
can:raw:on     # Enable promiscuous mode
stream:on      # Enable streaming
# Wait 10 seconds
stream:off
can:raw:off    # Restore filters
```

**Debug Output to Check:**
- High frame count (all IDs visible)
- Frames from both buses appear
- Filters restored after `can:raw:off`

---

## Scenario 6: Round-Robin Bus Reading
**Goal:** Verify bus 0 doesn't starve bus 1 under heavy load

**Setup:**
1. Both buses connected
2. Bus 0 has continuous heavy traffic (200+ frames/sec)
3. Bus 1 has occasional frames (10 frames/sec)

**Expected Behavior:**
- Both buses are read in round-robin fashion
- Bus 1 frames are not lost
- `lastReadBus1` flag alternates between true/false

**Test Commands:**
```
stream:on      # Enable streaming
# Observe frame sources for 30 seconds
stream:off
```

**Debug Output to Check:**
- Frames from both buses appear in stream
- No long gaps where only bus 0 frames appear
- Bus 1 frames are processed within reasonable time

---

## Scenario 7: LEGACY Variant Blocks Vehicle Commands
**Goal:** Verify LEGACY variant rejects all vehicle control commands

**Setup:**
1. Set variant to LEGACY: `variant:legacy`
2. Try vehicle control commands

**Expected Behavior:**
- All vehicle commands return error/no-op
- Only FSD/nag/profile commands work
- No 0x273 frames are sent

**Test Commands:**
```
variant:legacy
lock           # Should fail (LEGACY has no 0x273)
mirror:fold    # Should fail
pedal:sport    # Should fail
fsd:on         # Should work (LEGACY supports FSD)
nag:on         # Should work
profile:2      # Should work
```

**Debug Output to Check:**
- Error messages for vehicle commands
- No 0x273 frames sent
- FSD/nag/profile commands execute normally

---

## Scenario 8: Window/Sentry Always Use Bus 0
**Goal:** Verify window and sentry commands always send to bus 0

**Setup:**
1. Both buses connected
2. 0x273 is on bus 1 (ctrlBus = 1)
3. Send window/sentry commands

**Expected Behavior:**
- Window (0x119) always sends to bus 0
- Sentry (0x284) always sends to bus 0
- These IDs are NOT in bus 1 filter list

**Test Commands:**
```
# Ensure ctrlBus is set to 1 (0x273 on bus 1)
vent:open      # Should send 0x119 to bus 0
sentry:on      # Should send 0x284 to bus 0
```

**Debug Output to Check:**
- 0x119 frames appear on bus 0 only
- 0x284 frames appear on bus 0 only
- No errors or crashes

---

## Scenario 9: Trunk Commands Use Correct Bus
**Goal:** Verify frunk uses ctrlBus, trunk/glovebox use bus 0

**Setup:**
1. Both buses connected
2. 0x273 is on bus 1 (ctrlBus = 1)

**Expected Behavior:**
- Frunk (0x273 bit 5) sends to bus 1 (follows ctrlBus)
- Trunk (0x3B3) always sends to bus 0
- Glovebox (0x3B3) always sends to bus 0

**Test Commands:**
```
frunk:open     # Should send 0x273 to bus 1
trunk:open     # Should send 0x3B3 to bus 0
glovebox       # Should send 0x3B3 to bus 0
```

**Debug Output to Check:**
- 0x273 with bit 5 set appears on bus 1
- 0x3B3 frames appear on bus 0 only

---

## Scenario 10: Climate/Charge/Drive Use ctrlBus
**Goal:** Verify advanced features respect bus routing

**Setup:**
1. Both buses connected
2. 0x2F3 (climate) on bus 1
3. 0x333 (charge) on bus 1
4. 0x334 (drive) on bus 1
5. 0x273 on bus 1 (ctrlBus = 1)

**Expected Behavior:**
- Climate commands send modified 0x2F3 to bus 1
- Charge commands send modified 0x333 to bus 1
- Drive commands send modified 0x334 to bus 1

**Test Commands:**
```
climate:keep   # Should send 0x2F3 to bus 1
charge:start   # Should send 0x333 to bus 1
pedal:sport    # Should send 0x334 to bus 1
```

**Debug Output to Check:**
- Modified frames appear on bus 1
- Frame caching works (hasClimate, hasCharge, hasDrive)
- Commands fail gracefully if frames not cached

---

## Scenario 11: Summon Uses ctrlBus
**Goal:** Verify summon burst respects ctrlBus

**Setup:**
1. Both buses connected
2. 0x273 is on bus 1 (ctrlBus = 1)
3. Trigger summon

**Expected Behavior:**
- Summon burst sends 0x273 frames to bus 1
- 50 frames sent over 1 second
- Summon bits are set correctly

**Test Commands:**
```
summon         # Should send burst to bus 1
```

**Debug Output to Check:**
- 50x 0x273 frames appear on bus 1
- Summon active bit is set
- Direction and mode bits are correct

---

## Scenario 12: Bus 2 Not Installed (Single Bus Mode)
**Goal:** Verify firmware works correctly with only bus 0

**Setup:**
1. Only bus 0 connected
2. Compile with `uno` environment (Bluetooth, single bus)

**Expected Behavior:**
- Board boots normally
- All commands work (send to bus 0)
- No bus 2 code is compiled
- No crashes or errors

**Test Commands:**
```
status
lock
mirror:fold
pedal:sport    # Will fail (no 0x334 cached)
```

**Debug Output to Check:**
- Normal operation
- No bus 2 references in code
- All commands execute without errors

---

## Debug Checklist

### Serial Monitor Output
- [ ] Boot message appears
- [ ] Status updates every 500ms
- [ ] Command acknowledgments
- [ ] Error messages are clear
- [ ] No crashes or resets

### CAN Bus Traffic (Logic Analyzer)
- [ ] Frames appear on correct bus
- [ ] Frame IDs match expected values
- [ ] Frame data is correct
- [ ] Timing is correct (20ms intervals)
- [ ] No frames on wrong bus

### Memory Usage
- [ ] RAM usage < 80% (< 1638 bytes)
- [ ] Flash usage < 90% (< 28672 bytes)
- [ ] No stack overflow
- [ ] No memory leaks

### Performance
- [ ] Commands execute within 1 second
- [ ] No lag or delays
- [ ] Frame processing keeps up with traffic
- [ ] No dropped frames

### Edge Cases
- [ ] Bus 2 init failure handled
- [ ] Missing frame cache handled
- [ ] LEGACY variant blocks correctly
- [ ] Invalid commands rejected
- [ ] Overflow values clamped

---

## Quick Smoke Test

Run this sequence to verify basic functionality:

```bash
# 1. Boot check
status

# 2. Variant check
variant:hw4
variant:hw3
variant:legacy

# 3. FSD features
fsd:on
nag:on
profile:2

# 4. Vehicle controls (HW3/HW4 only)
variant:hw4
lock
unlock
mirror:fold
mirror:unfold
vent:open
vent:close

# 5. Advanced features (requires frame cache)
climate:keep
charge:start
pedal:sport
regen:max
stop:hold

# 6. Streaming
stream:on
# Wait 5 seconds
stream:off

# 7. Raw CAN
can:raw:on
stream:on
# Wait 5 seconds
stream:off
can:raw:off

# 8. LEGACY variant
variant:legacy
lock           # Should fail
fsd:on         # Should work
```

Expected: All commands execute without errors, correct frames appear on correct buses.
