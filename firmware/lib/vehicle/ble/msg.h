#pragma once
// ── Tesla RoutableMessage / VCSEC / CarServer protobuf builders ───────────────
//
// Field numbers are taken from the Tesla vehicle-command SDK protos:
//   universalmessage.proto, vcsec.proto, carserver.proto
//
// All functions write into a caller-supplied buffer and set *outLen on success.
// Return false if the buffer is too small or encoding fails.

#if BOARD_ENABLE_BLE

#include "proto.h"
#include <stdint.h>
#include <stddef.h>
#include <string.h>

namespace Tesla {

// ── Domain enum (universalmessage.proto) ──────────────────────────────────────
static const uint8_t DOMAIN_VEHICLE_SECURITY = 2;
static const uint8_t DOMAIN_INFOTAINMENT     = 3;

// ── KeyFormFactor enum (vcsec.proto) ──────────────────────────────────────────
static const uint8_t KEY_FORM_FACTOR_NFC_CARD = 3;
static const uint8_t KEY_FORM_FACTOR_BLE_KEY  = 2;

// ── Component enum (universalmessage.proto) ───────────────────────────────────
static const uint8_t COMPONENT_INFOTAINMENT     = 3;
static const uint8_t COMPONENT_VEHICLE_SECURITY = 2;

// ── Helper: encode Destination { domain = d } ─────────────────────────────────
static bool encDestination(uint8_t domain, uint8_t *buf, size_t cap, size_t &len)
{
    Proto p(buf, cap);
    p.fieldVarint(1, domain); // Destination.domain = field 1
    len = p.len;
    return p.ok();
}

// ── VCSEC: AddKeyToWhitelistAndAddPermissions ─────────────────────────────────
//   pub65  : uncompressed P-256 public key (65 bytes)
//   role   : ROLE_OWNER(4) or ROLE_CHARGING_MANAGER(5)
//   formFactor: KEY_FORM_FACTOR_NFC_CARD(3) or KEY_FORM_FACTOR_BLE_KEY(2)
//   Returns RoutableMessage bytes ready to send via BLE (no auth needed)
static bool buildAddKeyRequest(
    const uint8_t *pub65, uint8_t role, uint8_t formFactor,
    const uint8_t *uuid16,
    uint8_t *out, size_t cap, size_t &outLen)
{
    // ── inner: AddKeyToWhitelistAndAddPermissions ─────────────────────────────
    uint8_t addKeyBuf[80];
    Proto addKey(addKeyBuf, sizeof(addKeyBuf));
    addKey.fieldBytes(1, pub65, 65);    // .key       = field 1
    addKey.fieldVarint(2, role);        // .role      = field 2
    addKey.fieldVarint(5, formFactor);  // .form_factor = field 5
    if (!addKey.ok()) return false;

    // ── mid: UnsignedMessage { addKeyToWhitelistAndAddPermissions = field 2 }
    uint8_t umBuf[96];
    Proto um(umBuf, sizeof(umBuf));
    um.fieldMsg(2, addKey);
    if (!um.ok()) return false;

    // ── dest: Destination { domain = DOMAIN_VEHICLE_SECURITY }
    uint8_t destBuf[4];
    size_t  destLen = 0;
    if (!encDestination(DOMAIN_VEHICLE_SECURITY, destBuf, sizeof(destBuf), destLen))
        return false;

    // ── outer: RoutableMessage ────────────────────────────────────────────────
    Proto rm(out, cap);
    rm.fieldBytes(2, destBuf, destLen);    // to_destination = field 2
    rm.fieldMsg(5, um);                    // protobuf_message_as_bytes = field 5
    rm.fieldBytes(14, uuid16, 16);         // uuid = field 14 (16-byte random)
    outLen = rm.len;
    return rm.ok();
}

// ── Session: SessionInfoRequest ───────────────────────────────────────────────
//   eph65      : ephemeral P-256 public key (65 bytes, uncompressed)
//   challenge16: random 16 bytes
//   domain     : DOMAIN_INFOTAINMENT or DOMAIN_VEHICLE_SECURITY
static bool buildSessionInfoRequest(
    const uint8_t *eph65, const uint8_t *challenge16, uint8_t domain,
    const uint8_t *uuid16,
    uint8_t *out, size_t cap, size_t &outLen)
{
    // ── inner: SessionInfoRequest ─────────────────────────────────────────────
    uint8_t sirBuf[96];
    Proto sir(sirBuf, sizeof(sirBuf));
    sir.fieldBytes(1, eph65, 65);         // .public_key = field 1
    sir.fieldBytes(2, challenge16, 16);   // .challenge  = field 2
    if (!sir.ok()) return false;

    // ── dest
    uint8_t destBuf[4];
    size_t  destLen = 0;
    if (!encDestination(domain, destBuf, sizeof(destBuf), destLen)) return false;

    // ── outer: RoutableMessage { session_info_request = field 6 }
    Proto rm(out, cap);
    rm.fieldBytes(2, destBuf, destLen); // to_destination
    rm.fieldMsg(6, sir);               // session_info_request = field 6
    rm.fieldBytes(14, uuid16, 16);     // uuid
    outLen = rm.len;
    return rm.ok();
}

// ── CarServer: encode Action wrapping an already-serialised VehicleAction msg ─
//   innerBuf/innerLen: the serialised VehicleAction oneof field
static bool buildAction(const Proto &vehicleAction,
                         uint8_t *out, size_t cap, size_t &outLen)
{
    uint8_t vaBuf[256];
    Proto va(vaBuf, sizeof(vaBuf));
    va.fieldMsg(1, vehicleAction); // Action.vehicleAction = field 1

    Proto rm(out, cap);
    rm.fieldMsg(1, va);
    outLen = rm.len;
    return rm.ok() && va.ok();
}

// ── CarServer: specific vehicle actions ──────────────────────────────────────

// VehicleControlWakeupAction (field 25, empty)
static bool buildWakeAction(uint8_t *out, size_t cap, size_t &outLen)
{
    uint8_t waBuf[4];
    Proto wa(waBuf, sizeof(waBuf));
    wa.fieldEmpty(25); // vehicleControlWakeupAction = field 25
    return buildAction(wa, out, cap, outLen);
}

// ChargingStartStopAction { start: {} } – charge_start (field 11 → start field 1)
static bool buildChargeStartAction(uint8_t *out, size_t cap, size_t &outLen)
{
    uint8_t startBuf[4], cssBuf[8], vaBuf[16];
    Proto start(startBuf, sizeof(startBuf));
    start.fieldEmpty(1); // ChargeNow (empty message) at field 1
    Proto css(cssBuf, sizeof(cssBuf));
    css.fieldMsg(1, start); // start = field 1
    Proto va(vaBuf, sizeof(vaBuf));
    va.fieldMsg(11, css); // chargingStartStopAction = field 11
    return buildAction(va, out, cap, outLen);
}

// ChargingStartStopAction { stop: {} } – charge_stop (field 11 → stop field 2)
static bool buildChargeStopAction(uint8_t *out, size_t cap, size_t &outLen)
{
    uint8_t stopBuf[4], cssBuf[8], vaBuf[16];
    Proto stop(stopBuf, sizeof(stopBuf));
    stop.fieldEmpty(2); // StopCharge at field 2
    Proto css(cssBuf, sizeof(cssBuf));
    css.fieldMsg(2, stop); // stop = field 2
    Proto va(vaBuf, sizeof(vaBuf));
    va.fieldMsg(11, css);
    return buildAction(va, out, cap, outLen);
}

// ChargingSetAmpsAction { chargingAmps = n } (field 13 → amps field 1)
static bool buildSetAmpsAction(int32_t amps, uint8_t *out, size_t cap, size_t &outLen)
{
    uint8_t ampBuf[8], vaBuf[16];
    Proto amp(ampBuf, sizeof(ampBuf));
    amp.fieldVarint(1, (uint64_t)(uint32_t)amps); // chargingAmps = field 1
    Proto va(vaBuf, sizeof(vaBuf));
    va.fieldMsg(13, amp); // chargingSetAmpsAction = field 13
    return buildAction(va, out, cap, outLen);
}

// ChargingSetLimitAction { percent = n } (field 12 → percent field 1)
static bool buildSetLimitAction(int32_t pct, uint8_t *out, size_t cap, size_t &outLen)
{
    uint8_t lmtBuf[8], vaBuf[16];
    Proto lmt(lmtBuf, sizeof(lmtBuf));
    lmt.fieldVarint(1, (uint64_t)(uint32_t)pct); // percent = field 1
    Proto va(vaBuf, sizeof(vaBuf));
    va.fieldMsg(12, lmt); // chargingSetLimitAction = field 12
    return buildAction(va, out, cap, outLen);
}

// HvacAutoAction { power_on = powerOn } (field 14 → power_on field 1)
static bool buildClimateAction(bool powerOn, uint8_t *out, size_t cap, size_t &outLen)
{
    uint8_t hvacBuf[8], vaBuf[16];
    Proto hvac(hvacBuf, sizeof(hvacBuf));
    hvac.fieldBool(1, powerOn); // power_on = field 1
    Proto va(vaBuf, sizeof(vaBuf));
    va.fieldMsg(14, hvac); // hvacAutoAction = field 14
    return buildAction(va, out, cap, outLen);
}

// ── Authenticated RoutableMessage (after ECDH session established) ────────────
//   encPayload / encLen : AES-GCM ciphertext of CarServer.Action
//   gcmTag16           : 16-byte AES-GCM authentication tag
//   counter            : session counter (monotonically increasing)
//   domain             : DOMAIN_INFOTAINMENT
//   uuid16             : 16 random bytes for request correlation
static bool buildAuthMessage(
    const uint8_t *encPayload, size_t encLen,
    const uint8_t *gcmTag16, uint32_t counter, uint8_t domain,
    const uint8_t *uuid16,
    uint8_t *out, size_t cap, size_t &outLen)
{
    // AES_GCM_Personalized_Signature_Status { tag=field1, counter=field2, component_id=field3 }
    uint8_t agpsBuf[32];
    Proto agps(agpsBuf, sizeof(agpsBuf));
    agps.fieldBytes(1, gcmTag16, 16);                     // tag
    agps.fieldVarint(2, counter);                          // counter
    agps.fieldVarint(3, COMPONENT_INFOTAINMENT);           // component_id = 3

    // SignedData { AES_GCM_personalized_data = field 2 }
    uint8_t sdBuf[48];
    Proto sd(sdBuf, sizeof(sdBuf));
    sd.fieldMsg(2, agps);

    // Destination
    uint8_t destBuf[4];
    size_t  destLen = 0;
    if (!encDestination(domain, destBuf, sizeof(destBuf), destLen)) return false;

    // RoutableMessage
    Proto rm(out, cap);
    rm.fieldBytes(2, destBuf, destLen);    // to_destination = field 2
    rm.fieldBytes(5, encPayload, encLen);  // protobuf_message_as_bytes = field 5
    rm.fieldMsg(7, sd);                    // signature_data = field 7
    rm.fieldBytes(14, uuid16, 16);         // uuid = field 14
    outLen = rm.len;
    return rm.ok() && agps.ok() && sd.ok();
}

// ── Parse session_info from a received RoutableMessage ───────────────────────
// Returns pointer to session_info bytes inside buf, sets siLen.
// SessionInfo is in field 10 of RoutableMessage (sub_sigData oneof).
// Returns nullptr if not found.
static const uint8_t *parseSessionInfo(const uint8_t *buf, size_t bufLen, size_t &siLen)
{
    size_t i = 0;
    while (i < bufLen) {
        if (i >= bufLen) break;
        uint64_t tagV = 0;
        uint8_t  shift = 0;
        while (i < bufLen && shift < 64) {
            uint8_t b = buf[i++];
            tagV |= (uint64_t)(b & 0x7F) << shift;
            if (!(b & 0x80)) break;
            shift += 7;
        }
        uint32_t field = (uint32_t)(tagV >> 3);
        uint8_t  wire  = (uint8_t)(tagV & 0x07);

        if (wire == 2) {
            uint64_t len = 0; shift = 0;
            while (i < bufLen && shift < 64) {
                uint8_t b = buf[i++];
                len |= (uint64_t)(b & 0x7F) << shift;
                if (!(b & 0x80)) break;
                shift += 7;
            }
            if (field == 10) { // session_info = field 10
                siLen = (size_t)len;
                return buf + i;
            }
            i += (size_t)len; // skip
        } else if (wire == 0) {
            while (i < bufLen) { if (!(buf[i++] & 0x80)) break; }
        } else {
            break; // unsupported wire type, stop
        }
    }
    return nullptr;
}

// ── Parse SessionInfo fields ──────────────────────────────────────────────────
struct SessionInfoFields {
    uint8_t publicKey[65]; // vehicle ephemeral public key (field 1, should be 65 bytes)
    size_t  publicKeyLen;
    uint8_t challenge[32]; // vehicle challenge (field 2, variable length)
    size_t  challengeLen;
    bool    valid;
};

static SessionInfoFields parseSessionInfoFields(const uint8_t *buf, size_t bufLen)
{
    SessionInfoFields f;
    memset(&f, 0, sizeof(f));

    size_t i = 0;
    while (i < bufLen) {
        uint64_t tagV = 0; uint8_t shift = 0;
        while (i < bufLen && shift < 64) {
            uint8_t b = buf[i++];
            tagV |= (uint64_t)(b & 0x7F) << shift;
            if (!(b & 0x80)) break;
            shift += 7;
        }
        uint32_t field = (uint32_t)(tagV >> 3);
        uint8_t  wire  = (uint8_t)(tagV & 0x07);

        if (wire == 2) {
            uint64_t len = 0; shift = 0;
            while (i < bufLen && shift < 64) {
                uint8_t b = buf[i++];
                len |= (uint64_t)(b & 0x7F) << shift;
                if (!(b & 0x80)) break;
                shift += 7;
            }
            if (field == 1 && len <= 65) { // publicKey
                memcpy(f.publicKey, buf + i, (size_t)len);
                f.publicKeyLen = (size_t)len;
            } else if (field == 2 && len <= 32) { // challenge
                memcpy(f.challenge, buf + i, (size_t)len);
                f.challengeLen = (size_t)len;
            }
            i += (size_t)len;
        } else if (wire == 0) {
            while (i < bufLen) { if (!(buf[i++] & 0x80)) break; }
        } else { break; }
    }

    f.valid = (f.publicKeyLen > 0);
    return f;
}

} // namespace Tesla

#endif // BOARD_ENABLE_BLE
