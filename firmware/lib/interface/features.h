#pragma once

/**
 * @file firmware/lib/interface/features.h
 * @brief Aggregates all vehicle feature command includes for the command dispatcher
 * @author Tesla CAN Mod Contributors
 * @license GPL-3.0
 */

#include <Arduino.h>
#include "core/config/esp32.h"
#include "core/types.h"
#include "transport/can/bus.h"
#include "core/forward.h"
#include "core/util/parse.h"
#include "core/log/ring.h"

// Bus-independent features (stream, raw CAN listen, clock profile)
#include "vehicle/can/feature/misc/stream.h"
#include "vehicle/can/feature/misc/can_raw.h"
#include "vehicle/can/feature/misc/can_clock.h"

// CAN-bus features (always compiled; runtime no-op when target bus inactive)
#include "vehicle/can/feature/das/das_drive.h"
#include "vehicle/can/feature/fsd/fsd.h"
#include "vehicle/can/feature/fsd/nag.h"
#include "vehicle/can/feature/fsd/auto_lane_change.h"
#include "vehicle/can/feature/safety/ban_shield.h"
#include "vehicle/can/feature/das/tlssc.h"
#include "vehicle/can/feature/fsd/profile.h"
#include "vehicle/can/feature/fsd/offsets.h"
#include "vehicle/can/feature/fsd/isa_chime.h"
#include "vehicle/can/feature/body/summon.h"
#include "vehicle/can/feature/misc/variant.h"
#include "vehicle/can/feature/telemetry/bms.h"
#include "vehicle/can/feature/telemetry/tpms.h"
#include "vehicle/can/feature/fsd/region.h"
#include "vehicle/can/feature/drive/drive_mode.h"
#include "vehicle/can/feature/body/mirror.h"
#include "vehicle/can/feature/body/lock.h"
#include "vehicle/can/feature/comfort/light.h"
#include "vehicle/can/feature/comfort/wiper.h"
#include "vehicle/can/feature/comfort/seat.h"
#include "vehicle/can/feature/comfort/display.h"
#include "vehicle/can/feature/body/power.h"
#include "vehicle/can/feature/comfort/climate.h"
#include "vehicle/can/feature/body/charge.h"
#include "vehicle/can/feature/drive/pedal.h"
#include "vehicle/can/feature/drive/regen.h"
#include "vehicle/can/feature/drive/stop.h"
#include "vehicle/can/feature/comfort/precondition.h"
#include "vehicle/can/feature/body/track_mode.h"
#include "vehicle/can/feature/body/turn_signal.h"
#include "vehicle/can/feature/comfort/seatbelt.h"
#include "vehicle/can/feature/comfort/air_recirc.h"
#include "vehicle/can/feature/telemetry/powertrain.h"
#include "vehicle/can/feature/misc/can_sim.h"
#include "vehicle/can/feature/safety/single_shot.h"
#include "vehicle/can/feature/misc/mqtt_bridge.h"
#include "vehicle/can/feature/telemetry/fw_compat.h"
#include "vehicle/can/feature/telemetry/vehicle_config.h"
#include "vehicle/can/feature/body/window.h"
#include "vehicle/can/feature/body/sentry.h"
#include "vehicle/can/feature/body/trunk.h"

#if BOARD_ENABLE_BLE
#include "interface/ble/peripheral.h"
#include "interface/gamepad/gamepad.h"
#endif
