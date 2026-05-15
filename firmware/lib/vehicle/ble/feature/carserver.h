#pragma once

/**
 * @file firmware/lib/vehicle/ble/feature/carserver.h
 * @brief CarServer (Domain 3) action builders — honk, lights, climate, charge, etc.
 *
 * Each builder encodes an Action { vehicleAction (2) { VehicleAction { <action> (<field>) } } }
 * message per Tesla car_server.proto. These are sent through an ECDH + AES-GCM
 * session via runAuthCommand.
 *
 * @author Tesla CAN Mod Contributors
 * @license GPL-3.0
 */

#if BOARD_ENABLE_BLE

#include "../proto.h"
#include <stdint.h>
#include <stddef.h>
#include <string.h>

namespace Tesla
{

// ── CarServer VehicleAction oneof field numbers (from car_server.proto) ─────

const uint32_t VEHICLE_ACTION_HVAC_AUTO = 10;
const uint32_t VEHICLE_ACTION_STEERING_WHEEL_HEATER = 13;
const uint32_t VEHICLE_ACTION_SET_CHARGE_LIMIT = 5;
const uint32_t VEHICLE_ACTION_CHARGE_START_STOP = 6;
const uint32_t VEHICLE_ACTION_FLASH_LIGHTS = 26;
const uint32_t VEHICLE_ACTION_HONK_HORN = 27;
const uint32_t VEHICLE_ACTION_SET_SENTRY_MODE = 30;
const uint32_t VEHICLE_ACTION_SET_VALET_MODE = 31;
const uint32_t VEHICLE_ACTION_SUNROOF_OPEN_CLOSE = 32;
const uint32_t VEHICLE_ACTION_TRIGGER_HOMELINK = 33;
const uint32_t VEHICLE_ACTION_WINDOW = 34;
const uint32_t VEHICLE_ACTION_SEAT_HEATER = 36;
const uint32_t VEHICLE_ACTION_SET_CHARGING_AMPS = 43;
const uint32_t VEHICLE_ACTION_CLIMATE_KEEPER = 44;
const uint32_t VEHICLE_ACTION_CHARGE_PORT_CLOSE = 61;
const uint32_t VEHICLE_ACTION_CHARGE_PORT_OPEN = 62;

// ── ClimateKeeperAction_E enum ──────────────────────────────────────────────

const uint8_t CLIMATE_KEEPER_OFF = 0;
const uint8_t CLIMATE_KEEPER_ON = 1;
const uint8_t CLIMATE_KEEPER_DOG = 2;
const uint8_t CLIMATE_KEEPER_CAMP = 3;

// ── Charge start/stop enum ──────────────────────────────────────────────────

const uint8_t CHARGE_ACTION_START = 2;
const uint8_t CHARGE_ACTION_START_STANDARD = 3;
const uint8_t CHARGE_ACTION_START_MAX_RANGE = 4;
const uint8_t CHARGE_ACTION_STOP = 5;

// ── Seat heater enum values ─────────────────────────────────────────────────

const uint8_t SEAT_HEATER_OFF = 2;
const uint8_t SEAT_HEATER_LOW = 3;
const uint8_t SEAT_HEATER_MED = 4;
const uint8_t SEAT_HEATER_HIGH = 5;

const uint8_t SEAT_POS_FRONT_LEFT = 7;
const uint8_t SEAT_POS_FRONT_RIGHT = 8;
const uint8_t SEAT_POS_REAR_LEFT = 9;
const uint8_t SEAT_POS_REAR_RIGHT = 12;

// ── CarServer action envelope ───────────────────────────────────────────────

/**
 * @brief Wrap an action message in the standard CarServer Action envelope.
 *
 * Encoding:
 *   Action { vehicleAction (2) { VehicleAction { <msg> (<actionField>) } } }
 *
 * @param actionField  VehicleAction oneof field number.
 * @param actionMsg    Serialized action-specific message (may have len==0 for empty).
 * @param out          Output buffer for the complete Action message.
 * @param cap          Output buffer capacity.
 * @param outLen       Set to number of bytes written.
 * @return True on success.
 */
static bool wrapCarServerAction(uint32_t actionField, const Proto &actionMsg, uint8_t *out, size_t cap, size_t &outLen)
{
	uint8_t vaBuf[64];
	Proto va(vaBuf, sizeof(vaBuf));
	if (actionMsg.len == 0)
	{
		if (!va.fieldEmpty(actionField))
			return false;
	}
	else
	{
		if (!va.fieldMsg(actionField, actionMsg))
			return false;
	}

	Proto action(out, cap);
	if (!action.fieldMsg(2, va)) // Action.vehicleAction = field 2
		return false;
	outLen = action.len;
	return true;
}

// ── Simple / empty actions ──────────────────────────────────────────────────

static const Proto s_emptyProto(nullptr, 0);

static bool buildHonkAction(uint8_t *out, size_t cap, size_t &outLen)
{
	return wrapCarServerAction(VEHICLE_ACTION_HONK_HORN, s_emptyProto, out, cap, outLen);
}

static bool buildFlashLightsAction(uint8_t *out, size_t cap, size_t &outLen)
{
	return wrapCarServerAction(VEHICLE_ACTION_FLASH_LIGHTS, s_emptyProto, out, cap, outLen);
}

// ── Charge port door ────────────────────────────────────────────────────────

static bool buildChargePortDoorAction(bool open, uint8_t *out, size_t cap, size_t &outLen)
{
	uint32_t field = open ? VEHICLE_ACTION_CHARGE_PORT_OPEN : VEHICLE_ACTION_CHARGE_PORT_CLOSE;
	return wrapCarServerAction(field, s_emptyProto, out, cap, outLen);
}

// ── Window control ──────────────────────────────────────────────────────────

/**
 * @brief Build a window vent or close action.
 *
 * VehicleControlWindowAction { vent (3) or close (4) } — both are Void (empty).
 *
 * @param open   true = vent, false = close.
 */
static bool buildWindowAction(bool open, uint8_t *out, size_t cap, size_t &outLen)
{
	uint8_t waBuf[8];
	Proto wa(waBuf, sizeof(waBuf));
	if (!wa.fieldEmpty(open ? 3 : 4)) // vent=3, close=4
		return false;
	return wrapCarServerAction(VEHICLE_ACTION_WINDOW, wa, out, cap, outLen);
}

// ── Sunroof control ─────────────────────────────────────────────────────────

/**
 * @brief Build a sunroof action.
 *
 * VehicleControlSunroofOpenCloseAction { vent (3) | close (4) | open (5) } — all Void.
 *
 * @param position  3=vent, 4=close, 5=open.
 */
static bool buildSunroofAction(uint8_t position, uint8_t *out, size_t cap, size_t &outLen)
{
	if (position < 3 || position > 5)
		return false;
	uint8_t saBuf[8];
	Proto sa(saBuf, sizeof(saBuf));
	if (!sa.fieldEmpty(position))
		return false;
	return wrapCarServerAction(VEHICLE_ACTION_SUNROOF_OPEN_CLOSE, sa, out, cap, outLen);
}

// ── Sentry mode ─────────────────────────────────────────────────────────────

static bool buildSentryModeAction(bool on, uint8_t *out, size_t cap, size_t &outLen)
{
	uint8_t smBuf[8];
	Proto sm(smBuf, sizeof(smBuf));
	if (!sm.fieldBool(1, on))
		return false;
	return wrapCarServerAction(VEHICLE_ACTION_SET_SENTRY_MODE, sm, out, cap, outLen);
}

// ── Valet mode ──────────────────────────────────────────────────────────────

static bool buildValetModeAction(bool on, const char *password, uint8_t *out, size_t cap, size_t &outLen)
{
	uint8_t vmBuf[64];
	Proto vm(vmBuf, sizeof(vmBuf));
	if (!vm.fieldBool(1, on))
		return false;
	if (on && password && password[0])
	{
		if (!vm.fieldStr(2, password))
			return false;
	}
	return wrapCarServerAction(VEHICLE_ACTION_SET_VALET_MODE, vm, out, cap, outLen);
}

// ── Climate keeper ──────────────────────────────────────────────────────────

static bool buildClimateKeeperAction(uint8_t mode, uint8_t *out, size_t cap, size_t &outLen)
{
	uint8_t ckBuf[16];
	Proto ck(ckBuf, sizeof(ckBuf));
	if (!ck.fieldVarint(1, mode))
		return false;
	return wrapCarServerAction(VEHICLE_ACTION_CLIMATE_KEEPER, ck, out, cap, outLen);
}

// ── Steering wheel heater ───────────────────────────────────────────────────

static bool buildSteeringWheelHeaterAction(bool on, uint8_t *out, size_t cap, size_t &outLen)
{
	uint8_t shBuf[8];
	Proto sh(shBuf, sizeof(shBuf));
	if (!sh.fieldBool(1, on))
		return false;
	return wrapCarServerAction(VEHICLE_ACTION_STEERING_WHEEL_HEATER, sh, out, cap, outLen);
}

// ── Seat heater ─────────────────────────────────────────────────────────────

static bool buildSeatHeaterAction(uint8_t position, uint8_t level, uint8_t *out, size_t cap, size_t &outLen)
{
	uint8_t seatBuf[16];
	Proto seat(seatBuf, sizeof(seatBuf));
	if (!seat.fieldEmpty(level)) // seat_heater_level = OFF/LOW/MED/HIGH
		return false;
	if (!seat.fieldEmpty(position)) // seat_position = FL/FR/RL/RR
		return false;

	uint8_t shBuf[32];
	Proto sh(shBuf, sizeof(shBuf));
	if (!sh.fieldMsg(1, seat)) // hvacSeatHeaterAction (repeated, field 1)
		return false;

	return wrapCarServerAction(VEHICLE_ACTION_SEAT_HEATER, sh, out, cap, outLen);
}

// ── Homelink ────────────────────────────────────────────────────────────────

static bool buildHomelinkAction(float lat, float lon, uint8_t *out, size_t cap, size_t &outLen)
{
	uint8_t llBuf[16];
	Proto ll(llBuf, sizeof(llBuf));
	if (!ll.fieldFloat(1, lat))
		return false;
	if (!ll.fieldFloat(2, lon))
		return false;

	uint8_t hlBuf[32];
	Proto hl(hlBuf, sizeof(hlBuf));
	if (!hl.fieldMsg(1, ll)) // location = field 1
		return false;

	return wrapCarServerAction(VEHICLE_ACTION_TRIGGER_HOMELINK, hl, out, cap, outLen);
}

// ── Climate on/off ──────────────────────────────────────────────────────────

static bool buildClimateAction(bool on, uint8_t *out, size_t cap, size_t &outLen)
{
	uint8_t caBuf[8];
	Proto ca(caBuf, sizeof(caBuf));
	if (!ca.fieldBool(1, on)) // power_on
		return false;
	if (!ca.fieldBool(2, false)) // manual_override
		return false;
	return wrapCarServerAction(VEHICLE_ACTION_HVAC_AUTO, ca, out, cap, outLen);
}

// ── Charge actions ──────────────────────────────────────────────────────────

static bool buildChargeStartAction(uint8_t *out, size_t cap, size_t &outLen)
{
	uint8_t csBuf[8];
	Proto cs(csBuf, sizeof(csBuf));
	if (!cs.fieldEmpty(CHARGE_ACTION_START))
		return false;
	return wrapCarServerAction(VEHICLE_ACTION_CHARGE_START_STOP, cs, out, cap, outLen);
}

static bool buildChargeStopAction(uint8_t *out, size_t cap, size_t &outLen)
{
	uint8_t csBuf[8];
	Proto cs(csBuf, sizeof(csBuf));
	if (!cs.fieldEmpty(CHARGE_ACTION_STOP))
		return false;
	return wrapCarServerAction(VEHICLE_ACTION_CHARGE_START_STOP, cs, out, cap, outLen);
}

static bool buildSetAmpsAction(int amps, uint8_t *out, size_t cap, size_t &outLen)
{
	uint8_t saBuf[8];
	Proto sa(saBuf, sizeof(saBuf));
	if (!sa.fieldVarint(1, (uint32_t)amps))
		return false;
	return wrapCarServerAction(VEHICLE_ACTION_SET_CHARGING_AMPS, sa, out, cap, outLen);
}

static bool buildSetLimitAction(int pct, uint8_t *out, size_t cap, size_t &outLen)
{
	uint8_t slBuf[8];
	Proto sl(slBuf, sizeof(slBuf));
	if (!sl.fieldVarint(1, (uint32_t)pct))
		return false;
	return wrapCarServerAction(VEHICLE_ACTION_SET_CHARGE_LIMIT, sl, out, cap, outLen);
}

} // namespace Tesla

#endif // BOARD_ENABLE_BLE
