#pragma once
#include "core/types.h"
#include "protocol/can.h"

// ── 0x273 UI_vehicleControl — Shared Frame Reference ─────────────────────────
// DBC: BO_ 627 ID273UI_vehicleControl: 8 VehicleBus
// This frame controls most vehicle functions via CAN injection.
//
// Bit-level helpers are split into feature-specific headers:
//   mirror.h   — Mirror fold/heat/auto-fold/dip (bits 24-26, 52-53)
//   lock.h     — Lock/unlock/child lock (bits 16-19)
//   trunk.h    — Frunk request (bit 5), horn (bit 61)
//   light.h    — Fog/high-beam/ambient/dome (bits 3, 23, 30, 40-41, 59-60)
//   wiper.h    — Wiper speed (bits 56-58)
//   seat.h     — Seat heating (bits 42-51)
//   display.h  — Display brightness (bits 32-39)
//   power.h    — Accessory power/power-off/drive state (bits 0, 31, 62)
//   summon.h   — Summon active/direction/mode (bits 0, 4, 5)
//
// NOTE: Summon (bit 5 = direction) and Frunk (bit 5 = open) share the same
// bit on 0x273. They are mutually exclusive operations — never send both.
