#pragma once

/**
 * @file firmware/lib/vehicle/ble/feature/vcsec.h
 * @brief VCSEC (Domain 2) unsigned message builders — RKE actions and closure control
 *
 * Encodes UnsignedMessage → SignedMessage → ToVCSECMessage per Tesla vcsec.proto.
 * VCSEC uses SIGNATURE_TYPE_PRESENT_KEY (no ECDH session required).
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

// ── VCSEC constants (from vcsec.proto) ──────────────────────────────────────

const uint8_t SIGNATURE_TYPE_PRESENT_KEY = 2; // signatures.proto
const uint8_t RKE_ACTION_UNLOCK = 0;		  // RKEAction_E
const uint8_t RKE_ACTION_LOCK = 1;
const uint8_t RKE_ACTION_WAKE_VEHICLE = 30;

const uint8_t CLOSURE_MOVE_TYPE_NONE = 0; // ClosureMoveType_E
const uint8_t CLOSURE_MOVE_TYPE_MOVE = 1;
const uint8_t CLOSURE_MOVE_TYPE_STOP = 2;
const uint8_t CLOSURE_MOVE_TYPE_OPEN = 3;
const uint8_t CLOSURE_MOVE_TYPE_CLOSE = 4;

// ClosureMoveRequest field numbers
const uint8_t CLOSURE_FIELD_FRONT_DRIVER = 1;
const uint8_t CLOSURE_FIELD_FRONT_PASSENGER = 2;
const uint8_t CLOSURE_FIELD_REAR_DRIVER = 3;
const uint8_t CLOSURE_FIELD_REAR_PASSENGER = 4;
const uint8_t CLOSURE_FIELD_REAR_TRUNK = 5;
const uint8_t CLOSURE_FIELD_FRONT_TRUNK = 6;
const uint8_t CLOSURE_FIELD_CHARGE_PORT = 7;
const uint8_t CLOSURE_FIELD_TONNEAU = 8;

// ── ToVCSECMessage wrapping helper ──────────────────────────────────────────

/**
 * @brief Wrap an UnsignedMessage payload in SignedMessage(protobufMessageAsBytes=2,
 *        signatureType=SIGNATURE_TYPE_PRESENT_KEY=3) → ToVCSECMessage(signedMessage=1).
 * @param payload   Serialized UnsignedMessage bytes.
 * @param payloadLen Length of payload.
 * @param out       Output buffer.
 * @param cap       Output buffer capacity.
 * @param outLen    Set to number of bytes written.
 * @return True on success.
 */
static bool wrapVCSECSigned(const uint8_t *payload, size_t payloadLen, uint8_t *out, size_t cap, size_t &outLen)
{
	uint8_t smBuf[384];
	Proto sm(smBuf, sizeof(smBuf));
	if (!sm.fieldBytes(2, payload, payloadLen))
		return false;
	if (!sm.fieldVarint(3, SIGNATURE_TYPE_PRESENT_KEY))
		return false;

	Proto tvm(out, cap);
	if (!tvm.fieldMsg(1, sm))
		return false;
	outLen = tvm.len;
	return true;
}

// ── RKE action builder ──────────────────────────────────────────────────────

/**
 * @brief Build a VCSEC RKE action (lock, unlock, wake).
 *
 * UnsignedMessage { RKEAction (field 2): <action> }
 *
 * @param action  RKE_ACTION_LOCK, RKE_ACTION_UNLOCK, or RKE_ACTION_WAKE_VEHICLE.
 * @param out     Output buffer for the complete ToVCSECMessage.
 * @param cap     Output buffer capacity.
 * @param outLen  Set to number of bytes written.
 * @return True on success.
 */
static bool buildVCSECRKEAction(uint8_t action, uint8_t *out, size_t cap, size_t &outLen)
{
	uint8_t umBuf[16];
	Proto um(umBuf, sizeof(umBuf));
	if (!um.fieldVarint(2, action)) // UnsignedMessage.RKEAction = field 2
		return false;

	return wrapVCSECSigned(umBuf, um.len, out, cap, outLen);
}

// ── Closure action builder ──────────────────────────────────────────────────

/**
 * @brief Build a VCSEC closure move request (trunk, frunk, tonneau).
 *
 * UnsignedMessage { closureMoveRequest (field 4): { <closureField>: <motion> } }
 *
 * @param closureField  Which closure (CLOSURE_FIELD_REAR_TRUNK, etc.).
 * @param motion        What to do (CLOSURE_MOVE_TYPE_OPEN, CLOSE, STOP).
 * @param out           Output buffer for the complete ToVCSECMessage.
 * @param cap           Output buffer capacity.
 * @param outLen        Set to number of bytes written.
 * @return True on success.
 */
static bool buildVCSECClosureAction(uint8_t closureField, uint8_t motion, uint8_t *out, size_t cap, size_t &outLen)
{
	uint8_t cmrBuf[16];
	Proto cmr(cmrBuf, sizeof(cmrBuf));
	if (!cmr.fieldVarint(closureField, motion))
		return false;

	uint8_t umBuf[32];
	Proto um(umBuf, sizeof(umBuf));
	if (!um.fieldMsg(4, cmr)) // UnsignedMessage.closureMoveRequest = field 4
		return false;

	return wrapVCSECSigned(umBuf, um.len, out, cap, outLen);
}

// ── Wake action (convenience — uses RKE_ACTION_WAKE_VEHICLE) ────────────────

/**
 * @brief Build a wake-vehicle action (RKE_ACTION_WAKE_VEHICLE = 30).
 *
 * This is a VCSEC RKE action compatible with both asleep and awake vehicles.
 *
 * @param out     Output buffer.
 * @param cap     Output buffer capacity.
 * @param outLen  Set to number of bytes written.
 * @return True on success.
 */
static bool buildWakeAction(uint8_t *out, size_t cap, size_t &outLen)
{
	return buildVCSECRKEAction(RKE_ACTION_WAKE_VEHICLE, out, cap, outLen);
}

} // namespace Tesla

#endif // BOARD_ENABLE_BLE
