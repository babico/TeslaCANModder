# Vehicle Features

TeslaCANModder modifies CAN bus frames to enable and control various Tesla vehicle features. All features are OFF by default and must be explicitly enabled.

## FSD (Full Self-Driving) Enable

Modifies the FSD mux frame (CAN ID 1021 or 1006) to activate FSD capability.

- **HW4:** Sets bits 38, 46, 60 in mux 0
- **HW3:** Sets bits 38, 46 in mux 0, writes speed profile
- **Legacy:** Sets bit 46 in mux 0 (CAN ID 1006)

**Requires:** FSD must be selected in the vehicle UI. The modification only takes effect when `isFSDSelectedInUI` is true.

```
fsd:on     # Enable FSD modification
fsd:off    # Disable FSD modification
```

## Nag Suppression

Suppresses the "hands on wheel" nag prompt by clearing bit 19 in the FSD mux frame (mux ID 1).

- **HW4:** Also sets bit 47
- **HW3/Legacy:** Clears bit 19 only

```
nag:on     # Enable nag suppression
nag:off    # Disable nag suppression
```

## Speed Profile

Controls the FSD speed profile (aggressiveness), mapped from follow distance stalk position:

| Profile | Description |
| -------- | ------------ |
| 0 | Most aggressive |
| 1 | Moderate-aggressive |
| 2 | Moderate-conservative |
| 3 | Most conservative |

```
profile:0      # Set to most aggressive
profile:auto   # Auto-track from follow distance stalk
```

In auto mode, the profile follows the stalk position. When pinned, it stays at the set value regardless of stalk changes.

## Speed Offset (HW3 only)

Applies a speed offset to the FSD mux frame (mux ID 2). Only available on HW3 vehicles.

```
offset:5       # Set speed offset to 5
offset:auto    # Auto-track from UI offset steps
```

## ISA Speed Chime Suppression (HW4 only)

Suppresses the ISA (Intelligent Speed Assistance) speed warning chime by modifying CAN ID 921.

```
isa-chime:on   # Suppress ISA chime
isa-chime:off  # Restore original chime
```

## Summon

Sends summon CAN frames via the 0x273 (UI_vehicleControl) frame on the Vehicle Control bus. Sends a 30-frame burst.

```
summon:forward   # Summon forward
summon:reverse   # Summon reverse  
summon:stop      # Stop summon
```

> Requires a cached 0x273 frame from the vehicle bus. If not yet received, Console shows "Waiting for 0x273 frame".

## Vehicle Commands (3 CAN required)

These commands require a 3-CAN build and send frames on the Vehicle Control bus (Bus 1):

### Lock & Horn
- `lock` — Lock all doors
- `unlock` — Unlock all doors
- `lock:child` — Child lock
- `horn` — Sound horn

### Trunk & Frunk
- `frunk` / `frunk:open` — Open frunk
- `frunk:close` — Close frunk
- `trunk:open` — Open trunk
- `trunk:close` — Close trunk
- `glovebox` — Toggle glovebox

### Windows
- `vent:open` — Vent all windows
- `vent:close` — Close all windows

### Mirrors
- `mirror:fold` — Fold mirrors
- `mirror:unfold` — Unfold mirrors
- `mirror:heat` — Heat mirrors
- `mirror:autofold` — Auto-fold setting
- `mirror:dip` — Dip mirrors

### Lights
- `light:fog:front` / `light:fog:rear` — Fog lights
- `light:highbeam:auto` — Auto highbeam
- `light:ambient` — Ambient lighting
- `light:dome:on` / `light:dome:off` — Dome light

### Sentry
- `sentry:on` — Enable sentry mode
- `sentry:off` — Disable sentry mode

### Climate
- `climate:keep` — Keep climate on
- `climate:off` — Turn climate off

### Charge
- `charge:start` — Start charging
- `charge:stop` — Stop charging
- `chargeport` — Open charge port

### Drive Config
- `pedal:standard` / `pedal:chill` / `pedal:sport` — Pedal response
- `regen:off` / `regen:low` / `regen:standard` / `regen:max` — Regen braking
- `stop:creep` / `stop:roll` / `stop:hold` — Stopping mode

## Feature Availability by Variant

| Feature | HW4 | HW3 | Legacy |
| -------- | ---- | ---- | ------- |
| FSD Enable | ✅ | ✅ | ✅ |
| Nag Suppress | ✅ | ✅ | ✅ |
| Speed Profile | ✅ | ✅ | ✅ (limited) |
| Speed Offset | ❌ | ✅ | ❌ |
| ISA Chime | ✅ | ❌ | ❌ |
| Summon | ✅ | ✅ | ❌ |
| Vehicle Commands | ✅ | ✅ | ❌ |

## NVS Persistence

All toggle states (FSD, nag, ISA chime, profile, offset, variant) are saved to NVS flash (ESP32) or EEPROM (Arduino) and persist across reboots.
