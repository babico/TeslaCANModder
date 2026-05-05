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
#include "vehicle/can/feature/stream.h"
#include "vehicle/can/feature/can_raw.h"
#include "vehicle/can/feature/can_clock.h"

// ── CAN-bus features (always compiled; runtime no-op when target bus inactive)
#include "vehicle/can/feature/das_drive.h"
#include "vehicle/can/feature/fsd.h"
#include "vehicle/can/feature/nag.h"
#include "vehicle/can/feature/auto_lane_change.h"
#include "vehicle/can/feature/ban_shield.h"
#include "vehicle/can/feature/tlssc.h"
#include "vehicle/can/feature/profile.h"
#include "vehicle/can/feature/offsets.h"
#include "vehicle/can/feature/isa_chime.h"
#include "vehicle/can/feature/summon.h"
#include "vehicle/can/feature/variant.h"
#include "vehicle/can/feature/bms.h"
#include "vehicle/can/feature/tpms.h"
#include "vehicle/can/feature/region.h"
#include "vehicle/can/feature/drive_mode.h"
#include "vehicle/can/feature/mirror.h"
#include "vehicle/can/feature/lock.h"
#include "vehicle/can/feature/light.h"
#include "vehicle/can/feature/wiper.h"
#include "vehicle/can/feature/seat.h"
#include "vehicle/can/feature/display.h"
#include "vehicle/can/feature/power.h"
#include "vehicle/can/feature/climate.h"
#include "vehicle/can/feature/charge.h"
#include "vehicle/can/feature/pedal.h"
#include "vehicle/can/feature/regen.h"
#include "vehicle/can/feature/stop.h"
#include "vehicle/can/feature/precondition.h"
#include "vehicle/can/feature/track_mode.h"
#include "vehicle/can/feature/turn_signal.h"
#include "vehicle/can/feature/seatbelt.h"
#include "vehicle/can/feature/air_recirc.h"
#include "vehicle/can/feature/powertrain.h"
#include "vehicle/can/feature/can_sim.h"
#include "vehicle/can/feature/single_shot.h"
#include "vehicle/can/feature/mqtt_bridge.h"
#include "vehicle/can/feature/fw_compat.h"
#include "vehicle/can/feature/vehicle_config.h"
#include "vehicle/can/feature/window.h"
#include "vehicle/can/feature/sentry.h"
#include "vehicle/can/feature/trunk.h"

#if BOARD_ENABLE_BLE
#include "io/ble/esp32/board.h"
#include "client/gamepad/gamepad.h"
#endif
