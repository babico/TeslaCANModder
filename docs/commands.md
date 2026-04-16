# Command Reference

All commands work over USB Serial, BLE (Nordic UART), and WiFi REST API. Commands are case-sensitive, lowercase, colon-separated.

## System Commands

| Command | Description |
| ------- | ----------- |
| `ping` | Health check — returns `{"t":"pong","v":1}` |
| `status` | Full board state with all features, hardware info, uptime |

## FSD & Autopilot

| Command | Description |
| ------- | ----------- |
| `fsd:on` | Enable FSD CAN modification |
| `fsd:off` | Disable FSD CAN modification |
| `fsd:toggle` | Toggle FSD state |
| `nag:on` | Enable nag suppression |
| `nag:off` | Disable nag suppression |
| `nag:toggle` | Toggle nag suppression |

## Speed Profile

| Command | Description |
| ------- | ----------- |
| `profile:0` | Set speed profile to 0 (most aggressive) |
| `profile:1` | Set speed profile to 1 |
| `profile:2` | Set speed profile to 2 |
| `profile:3` | Set speed profile to 3 (most conservative) |
| `profile:auto` | Auto-track from follow distance stalk |

## Speed Offset (HW3 only)

| Command | Description |
| ------- | ----------- |
| `offset:N` | Set speed offset to N |
| `offset:auto` | Auto-track from UI offset steps |

## ISA Speed Chime (HW4 only)

| Command | Description |
| ------- | ----------- |
| `isa-chime:on` | Suppress ISA speed chime |
| `isa-chime:off` | Restore original ISA chime |
| `isa-chime:toggle` | Toggle ISA chime suppression |

## Variant Selection

| Command | Description |
| ------- | ----------- |
| `variant:hw4` | Set vehicle variant to HW4 |
| `variant:hw3` | Set vehicle variant to HW3 |
| `variant:legacy` | Set vehicle variant to Legacy |

## Nag Killer (EPAS Torque Spoofing)

| Command | Description |
| ------- | ----------- |
| `nagkiller:on` | Enable EPAS torque spoofing (zero steering torque) |
| `nagkiller:off` | Disable EPAS torque spoofing |
| `nagkiller:toggle` | Toggle nag killer state |

## Battery Preconditioning

| Command | Description |
| ------- | ----------- |
| `precondition:on` | Start battery preconditioning for supercharging |
| `precondition:off` | Stop battery preconditioning |
| `precondition:toggle` | Toggle preconditioning state |

## Track Mode

| Command | Description |
| ------- | ----------- |
| `trackmode:on` | Enable Track Mode |
| `trackmode:off` | Disable Track Mode |
| `trackmode:toggle` | Toggle Track Mode state |

## BMS Battery Telemetry

| Command | Description |
| ------- | ----------- |
| `bms` | Query current BMS telemetry (voltage, current, SoC, temps) |

## Summon (HW3/HW4, requires 3 CAN buses)

| Command | Description |
| ------- | ----------- |
| `summon` | Start summon (30-frame burst) |
| `summon:forward` | Summon forward |
| `summon:reverse` | Summon reverse |
| `summon:stop` | Stop summon |

> Summon requires a cached 0x273 frame. Console shows "Waiting for 0x273" if not yet received.

## Streaming & Raw CAN

| Command | Description |
| ------- | ----------- |
| `stream:on` | Start frame streaming (JSON per frame) |
| `stream:off` | Stop frame streaming |
| `can:raw:on` | Listen to all CAN IDs (unfiltered) |
| `can:raw:off` | Return to filtered mode (variant-specific IDs) |

## Vehicle Commands (3 CAN buses required)

### Lock & Security

| Command | Description |
| ------- | ----------- |
| `lock` | Lock vehicle |
| `unlock` | Unlock vehicle |
| `lock:child` | Child lock |
| `horn` | Horn |

### Trunk & Frunk

| Command | Description |
| ------- | ----------- |
| `frunk` / `frunk:open` | Open frunk |
| `frunk:close` | Close frunk |
| `trunk:open` | Open trunk |
| `trunk:close` | Close trunk |
| `glovebox` | Toggle glovebox |

### Windows

| Command | Description |
| ------- | ----------- |
| `vent:open` | Vent all windows |
| `vent:close` | Close all windows |

### Mirrors

| Command | Description |
| ------- | ----------- |
| `mirror:fold` | Fold mirrors |
| `mirror:unfold` | Unfold mirrors |
| `mirror:heat` | Heat mirrors |
| `mirror:autofold` | Auto-fold mirrors |
| `mirror:dip` | Dip mirrors |

### Lights

| Command | Description |
| ------- | ----------- |
| `light:fog:front` | Toggle front fog lights |
| `light:fog:rear` | Toggle rear fog lights |
| `light:highbeam:auto` | Auto highbeam |
| `light:ambient` | Toggle ambient lighting |
| `light:home` | Home lights |
| `light:dome:on` | Dome light on |
| `light:dome:off` | Dome light off |
| `light:dome:auto` | Dome light auto |

### Sentry

| Command | Description |
| ------- | ----------- |
| `sentry:on` | Enable sentry mode |
| `sentry:off` | Disable sentry mode |

### Climate

| Command | Description |
| ------- | ----------- |
| `climate:keep` | Keep climate on |
| `climate:off` | Turn climate off |

### Charge

| Command | Description |
| ------- | ----------- |
| `charge:start` | Start charging |
| `charge:stop` | Stop charging |
| `chargeport` | Open charge port |

### Wiper

| Command | Description |
| ------- | ----------- |
| `wiper:off` | Wiper off |
| `wiper:1` | Wiper speed 1 (low) |
| `wiper:2` | Wiper speed 2 (medium) |
| `wiper:3` | Wiper speed 3 (high) |

### Seat Heating

| Command | Description |
| ------- | ----------- |
| `seat:fl:0` – `seat:fl:3` | Front-left seat heat (0=off, 1=low, 2=med, 3=high) |
| `seat:fr:0` – `seat:fr:3` | Front-right seat heat |
| `seat:rl:0` – `seat:rl:3` | Rear-left seat heat |
| `seat:rr:0` – `seat:rr:3` | Rear-right seat heat |
| `seat:rc:0` – `seat:rc:3` | Rear-center seat heat |

### Display Brightness

| Command | Description |
| ------- | ----------- |
| `maindisplay:N` | Set main display brightness (0–127) |

### Power Control

| Command | Description |
| ------- | ----------- |
| `power:acc:on` | Accessory power on |
| `power:acc:off` | Accessory power off |
| `power:ready` | Drive-ready state |
| `power:off` | Power off vehicle |

### Drive Config

| Command | Description |
| ------- | ----------- |
| `pedal:standard` / `pedal:std` | Standard pedal mode |
| `pedal:chill` | Chill pedal mode |
| `pedal:sport` | Sport pedal mode |
| `regen:off` | Regen braking off |
| `regen:low` | Regen braking low |
| `regen:standard` / `regen:std` | Regen braking standard |
| `regen:max` | Regen braking max |
| `stop:creep` | Stopping mode: creep |
| `stop:roll` | Stopping mode: roll |
| `stop:hold` | Stopping mode: hold |

## Response Format

All responses are JSON with a `t` (type) field:

```json
{"t":"pong","v":1}
{"t":"ack","cmd":"fsd:on"}
{"t":"status","hw":"ESP32S_DevKit","variant":"hw4",...}
{"t":"error","msg":"Unknown command"}
{"t":"log","msg":"FSD enabled - saved to NVS"}
{"t":"frame","dir":"rx","bus":0,"id":1021,"dlc":8,"d":"0102030405060708"}
```
