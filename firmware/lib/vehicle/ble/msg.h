#pragma once

/**
 * @file firmware/lib/vehicle/ble/msg.h
 * @brief Tesla RoutableMessage, VCSEC, and CarServer protobuf message builders for BLE
 * @author Tesla CAN Mod Contributors
 * @license GPL-3.0
 */

#if BOARD_ENABLE_BLE

#include "proto.h"
#include <stdint.h>
#include <stddef.h>
#include <string.h>

namespace Tesla
{

// Domain enum values from universalmessage.proto
static const uint8_t DOMAIN_VEHICLE_SECURITY = 2;
static const uint8_t DOMAIN_INFOTAINMENT = 3;

// KeyFormFactor enum values from vcsec.proto
static const uint8_t KEY_FORM_FACTOR_NFC_CARD = 3;
static const uint8_t KEY_FORM_FACTOR_BLE_KEY = 2;

// Component enum values from universalmessage.proto
static const uint8_t COMPONENT_INFOTAINMENT = 3;
static const uint8_t COMPONENT_VEHICLE_SECURITY = 2;

/**
 * @brief Encode a Destination message containing a single domain field.
 * @param domain Domain identifier (DOMAIN_VEHICLE_SECURITY or DOMAIN_INFOTAINMENT).
 * @param buf Output buffer for the encoded Destination.
 * @param cap Capacity of the output buffer in bytes.
 * @param len Set to the number of bytes written on success.
 * @return True if encoding succeeded without overflow.
 */
static bool encDestination(uint8_t domain, uint8_t *buf, size_t cap, size_t &len)
{
	Proto p(buf, cap);
	p.fieldVarint(1, domain); // Destination.domain = field 1
	len = p.len;
	return p.ok();
}

/**
 * @brief Build an AddKeyToWhitelistAndAddPermissions request as a RoutableMessage.
 *
 * Constructs a VCSEC unsigned message that adds a public key to the vehicle whitelist
 * with the specified role and form factor. No authentication is required for this message.
 *
 * @param pub65 Uncompressed P-256 public key (65 bytes).
 * @param role Key role (e.g. ROLE_OWNER=4, ROLE_CHARGING_MANAGER=5).
 * @param formFactor Key form factor (KEY_FORM_FACTOR_NFC_CARD or KEY_FORM_FACTOR_BLE_KEY).
 * @param uuid16 16-byte random UUID for request correlation.
 * @param out Output buffer for the encoded RoutableMessage.
 * @param cap Capacity of the output buffer in bytes.
 * @param outLen Set to the number of bytes written on success.
 * @return True if encoding succeeded without overflow.
 */
static bool buildAddKeyRequest(const uint8_t *pub65, uint8_t role, uint8_t formFactor, const uint8_t *uuid16,
							   uint8_t *out, size_t cap, size_t &outLen)
{
	// Encode AddKeyToWhitelistAndAddPermissions inner message
	uint8_t addKeyBuf[80];
	Proto addKey(addKeyBuf, sizeof(addKeyBuf));
	addKey.fieldBytes(1, pub65, 65);   // .key = field 1
	addKey.fieldVarint(2, role);	   // .role = field 2
	addKey.fieldVarint(5, formFactor); // .form_factor = field 5
	if (!addKey.ok())
		return false;

	// Wrap in UnsignedMessage (addKeyToWhitelistAndAddPermissions = field 2)
	uint8_t umBuf[96];
	Proto um(umBuf, sizeof(umBuf));
	um.fieldMsg(2, addKey);
	if (!um.ok())
		return false;

	// Encode Destination targeting vehicle security domain
	uint8_t destBuf[4];
	size_t destLen = 0;
	if (!encDestination(DOMAIN_VEHICLE_SECURITY, destBuf, sizeof(destBuf), destLen))
		return false;

	// Assemble outer RoutableMessage
	Proto rm(out, cap);
	rm.fieldBytes(2, destBuf, destLen); // to_destination = field 2
	rm.fieldMsg(5, um);					// protobuf_message_as_bytes = field 5
	rm.fieldBytes(14, uuid16, 16);		// uuid = field 14
	outLen = rm.len;
	return rm.ok();
}

/**
 * @brief Build a SessionInfoRequest as a RoutableMessage.
 *
 * Constructs a session info request containing an ephemeral public key and a random
 * challenge, addressed to the specified domain.
 *
 * @param eph65 Ephemeral uncompressed P-256 public key (65 bytes).
 * @param challenge16 Random 16-byte challenge for session negotiation.
 * @param domain Target domain (DOMAIN_INFOTAINMENT or DOMAIN_VEHICLE_SECURITY).
 * @param uuid16 16-byte random UUID for request correlation.
 * @param out Output buffer for the encoded RoutableMessage.
 * @param cap Capacity of the output buffer in bytes.
 * @param outLen Set to the number of bytes written on success.
 * @return True if encoding succeeded without overflow.
 */
static bool buildSessionInfoRequest(const uint8_t *eph65, const uint8_t *challenge16, uint8_t domain,
									const uint8_t *uuid16, uint8_t *out, size_t cap, size_t &outLen)
{
	// Encode SessionInfoRequest inner message
	uint8_t sirBuf[96];
	Proto sir(sirBuf, sizeof(sirBuf));
	sir.fieldBytes(1, eph65, 65);		// .public_key = field 1
	sir.fieldBytes(2, challenge16, 16); // .challenge = field 2
	if (!sir.ok())
		return false;

	// Encode Destination for the target domain
	uint8_t destBuf[4];
	size_t destLen = 0;
	if (!encDestination(domain, destBuf, sizeof(destBuf), destLen))
		return false;

	// Assemble outer RoutableMessage (session_info_request = field 6)
	Proto rm(out, cap);
	rm.fieldBytes(2, destBuf, destLen); // to_destination = field 2
	rm.fieldMsg(6, sir);				// session_info_request = field 6
	rm.fieldBytes(14, uuid16, 16);		// uuid = field 14
	outLen = rm.len;
	return rm.ok();
}

/**
 * @brief Wrap a VehicleAction proto into a CarServer Action message.
 * @param vehicleAction Already-encoded Proto containing the VehicleAction oneof field.
 * @param out Output buffer for the encoded Action.
 * @param cap Capacity of the output buffer in bytes.
 * @param outLen Set to the number of bytes written on success.
 * @return True if encoding succeeded without overflow.
 */
static bool buildAction(const Proto &vehicleAction, uint8_t *out, size_t cap, size_t &outLen)
{
	uint8_t vaBuf[256];
	Proto va(vaBuf, sizeof(vaBuf));
	va.fieldMsg(1, vehicleAction); // Action.vehicleAction = field 1

	Proto rm(out, cap);
	rm.fieldMsg(1, va);
	outLen = rm.len;
	return rm.ok() && va.ok();
}

/**
 * @brief Build a VehicleControlWakeupAction (empty message at field 25).
 * @param out Output buffer for the encoded Action.
 * @param cap Capacity of the output buffer in bytes.
 * @param outLen Set to the number of bytes written on success.
 * @return True if encoding succeeded without overflow.
 */
static bool buildWakeAction(uint8_t *out, size_t cap, size_t &outLen)
{
	uint8_t waBuf[4];
	Proto wa(waBuf, sizeof(waBuf));
	wa.fieldEmpty(25); // vehicleControlWakeupAction = field 25
	return buildAction(wa, out, cap, outLen);
}

/**
 * @brief Build a ChargingStartStopAction to start charging.
 *
 * Encodes field 11 (chargingStartStopAction) with start (field 1) set to an empty message.
 *
 * @param out Output buffer for the encoded Action.
 * @param cap Capacity of the output buffer in bytes.
 * @param outLen Set to the number of bytes written on success.
 * @return True if encoding succeeded without overflow.
 */
static bool buildChargeStartAction(uint8_t *out, size_t cap, size_t &outLen)
{
	uint8_t startBuf[4], cssBuf[8], vaBuf[16];
	Proto start(startBuf, sizeof(startBuf));
	start.fieldEmpty(1); // ChargeNow (empty) at field 1
	Proto css(cssBuf, sizeof(cssBuf));
	css.fieldMsg(1, start); // start = field 1
	Proto va(vaBuf, sizeof(vaBuf));
	va.fieldMsg(11, css); // chargingStartStopAction = field 11
	return buildAction(va, out, cap, outLen);
}

/**
 * @brief Build a ChargingStartStopAction to stop charging.
 *
 * Encodes field 11 (chargingStartStopAction) with stop (field 2) set to an empty message.
 *
 * @param out Output buffer for the encoded Action.
 * @param cap Capacity of the output buffer in bytes.
 * @param outLen Set to the number of bytes written on success.
 * @return True if encoding succeeded without overflow.
 */
static bool buildChargeStopAction(uint8_t *out, size_t cap, size_t &outLen)
{
	uint8_t stopBuf[4], cssBuf[8], vaBuf[16];
	Proto stop(stopBuf, sizeof(stopBuf));
	stop.fieldEmpty(2); // StopCharge at field 2
	Proto css(cssBuf, sizeof(cssBuf));
	css.fieldMsg(2, stop); // stop = field 2
	Proto va(vaBuf, sizeof(vaBuf));
	va.fieldMsg(11, css); // chargingStartStopAction = field 11
	return buildAction(va, out, cap, outLen);
}

/**
 * @brief Build a ChargingSetAmpsAction to set the charging current.
 * @param amps Desired charging current in amps.
 * @param out Output buffer for the encoded Action.
 * @param cap Capacity of the output buffer in bytes.
 * @param outLen Set to the number of bytes written on success.
 * @return True if encoding succeeded without overflow.
 */
static bool buildSetAmpsAction(int32_t amps, uint8_t *out, size_t cap, size_t &outLen)
{
	uint8_t ampBuf[8], vaBuf[16];
	Proto amp(ampBuf, sizeof(ampBuf));
	amp.fieldVarint(1, (uint64_t)(uint32_t)amps); // chargingAmps = field 1
	Proto va(vaBuf, sizeof(vaBuf));
	va.fieldMsg(13, amp); // chargingSetAmpsAction = field 13
	return buildAction(va, out, cap, outLen);
}

/**
 * @brief Build a ChargingSetLimitAction to set the charge limit percentage.
 * @param pct Desired charge limit as a percentage (0-100).
 * @param out Output buffer for the encoded Action.
 * @param cap Capacity of the output buffer in bytes.
 * @param outLen Set to the number of bytes written on success.
 * @return True if encoding succeeded without overflow.
 */
static bool buildSetLimitAction(int32_t pct, uint8_t *out, size_t cap, size_t &outLen)
{
	uint8_t lmtBuf[8], vaBuf[16];
	Proto lmt(lmtBuf, sizeof(lmtBuf));
	lmt.fieldVarint(1, (uint64_t)(uint32_t)pct); // percent = field 1
	Proto va(vaBuf, sizeof(vaBuf));
	va.fieldMsg(12, lmt); // chargingSetLimitAction = field 12
	return buildAction(va, out, cap, outLen);
}

/**
 * @brief Build an HvacAutoAction to turn climate control on or off.
 * @param powerOn True to enable climate, false to disable.
 * @param out Output buffer for the encoded Action.
 * @param cap Capacity of the output buffer in bytes.
 * @param outLen Set to the number of bytes written on success.
 * @return True if encoding succeeded without overflow.
 */
static bool buildClimateAction(bool powerOn, uint8_t *out, size_t cap, size_t &outLen)
{
	uint8_t hvacBuf[8], vaBuf[16];
	Proto hvac(hvacBuf, sizeof(hvacBuf));
	hvac.fieldBool(1, powerOn); // power_on = field 1
	Proto va(vaBuf, sizeof(vaBuf));
	va.fieldMsg(14, hvac); // hvacAutoAction = field 14
	return buildAction(va, out, cap, outLen);
}

/**
 * @brief Build an authenticated RoutableMessage after an ECDH session is established.
 *
 * Wraps AES-GCM encrypted payload with signature data containing the authentication
 * tag, session counter, and component identifier.
 *
 * @param encPayload AES-GCM ciphertext of the CarServer.Action.
 * @param encLen Length of the encrypted payload in bytes.
 * @param gcmTag16 16-byte AES-GCM authentication tag.
 * @param counter Monotonically increasing session counter.
 * @param domain Target domain (typically DOMAIN_INFOTAINMENT).
 * @param uuid16 16-byte random UUID for request correlation.
 * @param out Output buffer for the encoded RoutableMessage.
 * @param cap Capacity of the output buffer in bytes.
 * @param outLen Set to the number of bytes written on success.
 * @return True if encoding succeeded without overflow.
 */
static bool buildAuthMessage(const uint8_t *encPayload, size_t encLen, const uint8_t *gcmTag16, uint32_t counter,
							 uint8_t domain, const uint8_t *uuid16, uint8_t *out, size_t cap, size_t &outLen)
{
	// AES_GCM_Personalized_Signature_Status: tag, counter, component_id
	uint8_t agpsBuf[32];
	Proto agps(agpsBuf, sizeof(agpsBuf));
	agps.fieldBytes(1, gcmTag16, 16);			 // tag = field 1
	agps.fieldVarint(2, counter);				 // counter = field 2
	agps.fieldVarint(3, COMPONENT_INFOTAINMENT); // component_id = field 3

	// SignedData wrapping the AES-GCM personalized data
	uint8_t sdBuf[48];
	Proto sd(sdBuf, sizeof(sdBuf));
	sd.fieldMsg(2, agps); // AES_GCM_personalized_data = field 2

	// Encode Destination for the target domain
	uint8_t destBuf[4];
	size_t destLen = 0;
	if (!encDestination(domain, destBuf, sizeof(destBuf), destLen))
		return false;

	// Assemble outer RoutableMessage
	Proto rm(out, cap);
	rm.fieldBytes(2, destBuf, destLen);	  // to_destination = field 2
	rm.fieldBytes(5, encPayload, encLen); // protobuf_message_as_bytes = field 5
	rm.fieldMsg(7, sd);					  // signature_data = field 7
	rm.fieldBytes(14, uuid16, 16);		  // uuid = field 14
	outLen = rm.len;
	return rm.ok() && agps.ok() && sd.ok();
}

/**
 * @brief Parse session_info bytes from a received RoutableMessage.
 *
 * Walks the top-level protobuf fields looking for field 10 (session_info) which is
 * a length-delimited payload within the RoutableMessage.
 *
 * @param buf Raw RoutableMessage bytes.
 * @param bufLen Length of the buffer in bytes.
 * @param siLen Set to the length of the session_info payload on success.
 * @return Pointer to the session_info bytes within buf, or nullptr if not found.
 */
static const uint8_t *parseSessionInfo(const uint8_t *buf, size_t bufLen, size_t &siLen)
{
	size_t i = 0;
	while (i < bufLen)
	{
		if (i >= bufLen)
			break;
		// Decode field tag as varint
		uint64_t tagV = 0;
		uint8_t shift = 0;
		while (i < bufLen && shift < 64)
		{
			uint8_t b = buf[i++];
			tagV |= (uint64_t)(b & 0x7F) << shift;
			if (!(b & 0x80))
				break;
			shift += 7;
		}
		uint32_t field = (uint32_t)(tagV >> 3);
		uint8_t wire = (uint8_t)(tagV & 0x07);

		if (wire == 2)
		{
			// Length-delimited: decode the length varint
			uint64_t len = 0;
			shift = 0;
			while (i < bufLen && shift < 64)
			{
				uint8_t b = buf[i++];
				len |= (uint64_t)(b & 0x7F) << shift;
				if (!(b & 0x80))
					break;
				shift += 7;
			}
			if (field == 10)
			{
				// session_info = field 10
				siLen = (size_t)len;
				return buf + i;
			}
			i += (size_t)len; // Skip payload of non-matching fields
		}
		else if (wire == 0)
		{
			// Varint: skip by consuming continuation bytes
			while (i < bufLen)
			{
				if (!(buf[i++] & 0x80))
					break;
			}
		}
		else
		{
			break; // Unsupported wire type
		}
	}
	return nullptr;
}

/**
 * @brief Parsed fields from a SessionInfo protobuf message.
 */
struct SessionInfoFields
{
	uint8_t publicKey[65]; // Vehicle ephemeral P-256 public key (expected 65 bytes)
	size_t publicKeyLen;   // Actual length of the public key received
	uint8_t challenge[32]; // Vehicle challenge bytes (variable length)
	size_t challengeLen;   // Actual length of the challenge received
	bool valid;            // True if at least the public key was present
};

/**
 * @brief Parse individual fields from a SessionInfo protobuf payload.
 *
 * Extracts the vehicle ephemeral public key (field 1) and challenge (field 2) from
 * the raw SessionInfo bytes.
 *
 * @param buf Pointer to the SessionInfo payload bytes.
 * @param bufLen Length of the SessionInfo payload.
 * @return Parsed SessionInfoFields with valid set to true if the public key was found.
 */
static SessionInfoFields parseSessionInfoFields(const uint8_t *buf, size_t bufLen)
{
	SessionInfoFields f;
	memset(&f, 0, sizeof(f));

	size_t i = 0;
	while (i < bufLen)
	{
		// Decode field tag as varint
		uint64_t tagV = 0;
		uint8_t shift = 0;
		while (i < bufLen && shift < 64)
		{
			uint8_t b = buf[i++];
			tagV |= (uint64_t)(b & 0x7F) << shift;
			if (!(b & 0x80))
				break;
			shift += 7;
		}
		uint32_t field = (uint32_t)(tagV >> 3);
		uint8_t wire = (uint8_t)(tagV & 0x07);

		if (wire == 2)
		{
			// Length-delimited: decode the length varint
			uint64_t len = 0;
			shift = 0;
			while (i < bufLen && shift < 64)
			{
				uint8_t b = buf[i++];
				len |= (uint64_t)(b & 0x7F) << shift;
				if (!(b & 0x80))
					break;
				shift += 7;
			}
			if (field == 1 && len <= 65)
			{
				// publicKey = field 1 (P-256 uncompressed, max 65 bytes)
				memcpy(f.publicKey, buf + i, (size_t)len);
				f.publicKeyLen = (size_t)len;
			}
			else if (field == 2 && len <= 32)
			{
				// challenge = field 2 (variable length, max 32 bytes)
				memcpy(f.challenge, buf + i, (size_t)len);
				f.challengeLen = (size_t)len;
			}
			i += (size_t)len;
		}
		else if (wire == 0)
		{
			// Varint: skip by consuming continuation bytes
			while (i < bufLen)
			{
				if (!(buf[i++] & 0x80))
					break;
			}
		}
		else
		{
			break; // Unsupported wire type
		}
	}

	f.valid = (f.publicKeyLen > 0);
	return f;
}

} // namespace Tesla

#endif // BOARD_ENABLE_BLE
