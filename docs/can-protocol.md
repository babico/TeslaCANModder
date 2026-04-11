# CAN Protocol

TeslaCANModder communicates with the vehicle by intercepting and modifying CAN bus frames on the Tesla X179 connector.

## Bus Assignments

The ESP32 3-CAN configuration maps to the Tesla X179 connector as follows:

| Bus | MCP2515 | X179 Pins | Function | CAN Speed |
|-----|---------|-----------|----------|-----------|
| **0** | MCP2515 #1 | 13–14 | FSD / Autopilot CAN | 500 kbps |
| **1** | MCP2515 #2 | 9–10 | Vehicle Control CAN | 500 kbps |
| **2** | MCP2515 #3 | 2–3 | Body Control CAN | 500 kbps |

> 1-CAN builds use only Bus 0 (FSD). Vehicle and body commands require 3-CAN builds.

## CAN IDs

| CAN ID | Hex | Name | Bus | Used By |
|--------|-----|------|-----|---------|
| 69 | 0x045 | Legacy stalk position | FSD | Legacy |
| 281 | 0x119 | Window vent control | Vehicle | HW3, HW4 |
| 627 | 0x273 | UI_vehicleControl (summon, lock, etc.) | Vehicle | HW3, HW4 |
| 644 | 0x284 | Sentry mode control | Vehicle | HW3, HW4 |
| 755 | 0x2F3 | Climate control | Vehicle | HW3, HW4 |
| 819 | 0x333 | Charge control | Vehicle | HW3, HW4 |
| 820 | 0x334 | Drive config (pedal/regen/stop) | Vehicle | HW3, HW4 |
| 921 | 0x399 | ISA speed chime | FSD | HW4 |
| 947 | 0x3B3 | Trunk/Glovebox control | Vehicle | HW3, HW4 |
| 1006 | 0x3EE | Legacy FSD mux | FSD | Legacy |
| 1016 | 0x3F8 | Follow distance (profile mapping) | FSD | HW3, HW4 |
| 1021 | 0x3FD | FSD mux (FSD/nag/profile/offset) | FSD | HW3, HW4 |

## FSD Mux Frame (CAN ID 1021)

The main FSD frame uses a multiplexer in the lower 3 bits of byte 0:

| Mux ID | Function | Modified Bits |
|--------|----------|---------------|
| **0** | FSD enable | bit 38 (FSD active), bit 46, bit 60 |
| **1** | Nag suppress | bit 19 (nag flag cleared), bit 47 (HW4) |
| **2** | Speed profile / offset | Profile bytes (HW4) or offset bytes (HW3) |

### HW4 Handler
- **Mux 0:** Sets bits 38, 46, 60 to enable FSD when UI has FSD selected
- **Mux 1:** Clears bit 19 and sets bit 47 to suppress nag
- **Mux 2:** Writes speed profile value

### HW3 Handler
- **Mux 0:** Sets bits 38, 46, writes speed profile into v12/v13 fields
- **Mux 1:** Clears bit 19 to suppress nag
- **Mux 2:** Writes speed offset value

### Legacy Handler
- Uses CAN ID **1006** instead of 1021
- **Mux 0:** Sets bit 46, writes speed profile
- **Mux 1:** Clears bit 19 to suppress nag

## ISA Speed Chime (HW4 only)

CAN ID 921 — ISA speed chime suppression:
- Sets `data[1] |= 0x20` to suppress the chime
- Recalculates checksum in `data[7]`
- Only active when `isaChimeSuppress` is enabled

## Follow Distance → Profile Mapping

CAN ID 1016 — The follow distance stalk position is read from `data[5] bits 7:5` and mapped to a speed profile (0–3) unless profile is manually pinned.

## Vehicle Control Frame (CAN ID 627)

Used for summon and vehicle commands. The summon system:
- Bit 4 of byte 0: Summon active
- Bit 5 of byte 0: Direction (0 = forward, 1 = reverse)
- Bit 0 of byte 0: Mode (start/stop)
- Sends 30-frame bursts when summon is activated

## Frame Routing

The dispatch system routes frames based on CAN ID and configured variant:

1. **RX from Bus 0 (FSD):** IDs 921, 1006, 1016, 1021, 69 → variant handler → modified frame sent back
2. **RX from Bus 1 (Vehicle):** ID 627 → cached as control frame for vehicle commands
3. **TX to Bus 1 (Vehicle):** Vehicle commands (lock, trunk, climate, etc.) sent using cached 627 frame
4. **Streaming:** All received frames optionally forwarded to serial/BLE as JSON

## Testing CAN Communication

```
# Enable raw CAN mode to see all frames
can:raw:on

# Start frame streaming
stream:on

# Expected frames by variant:
# HW4: 921, 1016, 1021, 627
# HW3: 1016, 1021, 627
# Legacy: 69, 1006
```

If no frames appear, check CAN-H/CAN-L connections and verify the vehicle is powered on.
