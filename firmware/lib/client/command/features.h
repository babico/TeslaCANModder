#pragma once
#include <Arduino.h>
#include "core/config/esp32/board.h"
#include "core/types.h"
#include "core/can/bus.h"
#include "io/log.h"
bool driverReinit();
void driverSetClockMHz(uint8_t mhz);
uint8_t driverGetClockReqMHz();
uint8_t driverGetClockMHz();

#include "core/util/parse.h"
#include "core/log/ring.h"

// ── Bus-independent features ─────────────────────────────────────────────────
#include "feature/stream.h"
#include "feature/can_raw.h"
#include "feature/can_clock.h"

// ── Chassis CAN features (BUS_CHASSIS) ──────────────────────────────────────
#if BUS_CHASSIS_ACTIVE
#include "feature/fsd.h"
#include "feature/nag.h"
#include "feature/auto_lane_change.h"
#include "feature/ban_shield.h"
#include "feature/tlssc.h"
#include "feature/profile.h"
#include "feature/offsets.h"
#include "feature/isa_chime.h"
#include "feature/summon.h"
#include "feature/variant.h"
#include "feature/bms.h"
#include "feature/tpms.h"
#include "feature/region.h"
#endif

// ── Vehicle CAN features (BUS_VEHICLE) ──────────────────────────────────────
#if BUS_VEHICLE_ACTIVE
#include "feature/drive_mode.h"
#include "feature/mirror.h"
#include "feature/lock.h"
#include "feature/light.h"
#include "feature/wiper.h"
#include "feature/seat.h"
#include "feature/display.h"
#include "feature/power.h"
#include "feature/climate.h"
#include "feature/charge.h"
#include "feature/pedal.h"
#include "feature/regen.h"
#include "feature/stop.h"
#include "feature/precondition.h"
#include "feature/track_mode.h"
#include "feature/turn_signal.h"
#include "feature/seatbelt.h"
#include "feature/air_recirc.h"
#include "feature/powertrain.h"
#include "feature/can_sim.h"
#include "feature/single_shot.h"
#include "feature/mqtt_bridge.h"
#include "feature/fw_compat.h"
#include "feature/vehicle_config.h"
#endif

// ── Body CAN features (BUS_BODY) ─────────────────────────────────────────────
#if BUS_BODY_ACTIVE
#include "feature/window.h"
#include "feature/sentry.h"
#endif

#if BUS_VEHICLE_ACTIVE || BUS_BODY_ACTIVE
#include "feature/trunk.h"
#endif

#if BOARD_ENABLE_BLE
#include "io/ble/esp32/board.h"
#endif
