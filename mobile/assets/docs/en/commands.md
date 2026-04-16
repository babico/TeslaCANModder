# Commands Reference

Complete list of commands supported by the firmware. All commands are sent as JSON over serial or Bluetooth.

## Connection Commands

- `{"cmd":"ping"}` — Ping the board, expects `{"type":"pong"}`
- `{"cmd":"status"}` — Request full status report
- `{"cmd":"stream","on":true}` — Start CAN frame streaming
- `{"cmd":"stream","on":false}` — Stop CAN frame streaming
- `{"cmd":"variant","val":"hw4"}` — Set vehicle variant (hw4, hw3, legacy)

## FSD & Nag Commands

- `{"cmd":"fsd","on":true}` — Enable FSD
- `{"cmd":"fsd","on":false}` — Disable FSD
- `{"cmd":"nag","on":true}` — Enable nag suppression
- `{"cmd":"nag","on":false}` — Disable nag suppression

## Speed Profile Commands

- `{"cmd":"profile","val":0}` — Chill (0)
- `{"cmd":"profile","val":1}` — Normal (1)
- `{"cmd":"profile","val":2}` — Hurry (2)
- `{"cmd":"profile","val":3}` — Max (3)
- `{"cmd":"profile","val":4}` — Sloth (4)
- `{"cmd":"profile_auto"}` — Auto profile selection

## Speed Offset Commands (HW3 only)

- `{"cmd":"offset","val":0}` — 0% offset
- `{"cmd":"offset","val":20}` — 20% offset
- `{"cmd":"offset","val":100}` — 100% offset
- `{"cmd":"offset_auto"}` — Auto offset

## ISA Speed Chime (HW4 only)

- `{"cmd":"isa_chime","on":true}` — Suppress ISA chime
- `{"cmd":"isa_chime","on":false}` — Restore original chime

## Summon Commands

- `{"cmd":"summon_fwd"}` — Summon forward
- `{"cmd":"summon_rev"}` — Summon reverse
- `{"cmd":"summon_stop"}` — Stop summon

## Vehicle Control Commands

### Mirrors
- `mirrorFold`, `mirrorUnfold`, `mirrorHeat`, `mirrorAutofold`, `mirrorDip`

### Locks & Horn
- `lock`, `unlock`, `lockChild`, `horn`

### Trunk & Frunk
- `frunkOpen`, `frunkClose`, `trunkOpen`, `trunkClose`, `glovebox`

### Lights
- `lightFogFront`, `lightFogRear`, `lightHighbeamAuto`, `lightAmbient`
- `lightHome`, `lightDomeOff`, `lightDomeOn`, `lightDomeAuto`

### Wipers
- `wiperOff`, `wiper1`, `wiper2`, `wiper3`

### Seat Heating
- `seatFL(level)`, `seatFR(level)`, `seatRL(level)`, `seatRR(level)`, `seatRC(level)` — Level 0–3

### Window & Sentry
- `ventOpen`, `ventClose`, `sentryOn`, `sentryOff`

### Climate
- `climateKeep`, `climateOff`

### Charging
- `chargeStart`, `chargeStop`, `chargePort`

### Drive Configuration
- Pedal: `pedalStandard`, `pedalChill`, `pedalSport`
- Regen: `regenOff`, `regenLow`, `regenStd`, `regenMax`
- Stop Mode: `stopCreep`, `stopRoll`, `stopHold`

### Display
- `mainDisplay(brightness)` — Brightness 0–127

### Power
- `powerAccOn`, `powerAccOff`, `powerReady`, `powerOff`

## CAN Raw Commands

- `{"cmd":"raw","id":1234,"data":"AABBCCDD"}` — Send raw CAN frame
- `{"cmd":"raw","id":1234,"data":"AABBCCDD","bus":1}` — Send on secondary bus

## Response Types

| Type | Description |
| ---- | ----------- |
| boot | Board boot message with hardware info |
| status | Full status report |
| frame | CAN frame data |
| ack | Command acknowledgment |
| error | Error response |
| log | Informational log message |
| pong | Response to ping |
