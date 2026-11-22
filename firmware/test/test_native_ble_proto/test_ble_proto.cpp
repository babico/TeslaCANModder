/**
 * @file firmware/test/test_native_ble_proto/test_ble_proto.cpp
 * @brief Native tests pinning Tesla RoutableMessage protobuf field numbers
 * @author Tesla CAN Mod Contributors
 * @license GPL-3.0
 *
 * Field numbers are taken from the SDK proto
 * legacy/teslamotors-vehicle-command/pkg/protocol/protobuf/universal_message.proto
 * (message RoutableMessage, lines 80-99):
 *   to_destination             = 6   (wire 2, tag 0x32)
 *   protobuf_message_as_bytes  = 10  (wire 2, tag 0x52)
 *   session_info_request       = 14  (wire 2, tag 0x72)
 *   session_info               = 15  (wire 2, tag 0x7A)
 *   uuid                       = 51  (wire 2, tag bytes 0x9A 0x03)
 *
 * The tests use fixed inputs and assert both the literal SDK tag bytes and the
 * exact field numbers/payloads via an independent top-level protobuf scanner.
 */

#include <unity.h>
#include <string.h>
#include <stdint.h>
#include <stddef.h>

// key.h exposes the SDK role constants and the role-string mapping used by
// Esp32Board::roleValue outside the BLE guard. Include it while the native
// build flag BOARD_ENABLE_BLE=0 is still in effect so no mbedTLS/ESP32
// headers are pulled into this translation unit.
#include "vehicle/ble/feature/key.h"

// native env sets -DBOARD_ENABLE_BLE=0; override locally so msg.h compiles.
#undef BOARD_ENABLE_BLE
#define BOARD_ENABLE_BLE 1

#include "vehicle/ble/msg.h"

// feature/vcsec.h is not wired into production, but it is the only VCSEC builder
// source. Pull it into this TU with its wake builder renamed: msg.h already defines
// a static buildWakeAction with a different (CarServer) meaning. Both are static,
// so keeping them in one TU is safe once the name collision is removed.
#define buildWakeAction buildVCSECWakeAction
#include "vehicle/ble/feature/vcsec.h"
#undef buildWakeAction

using namespace Tesla;

void setUp() {}
void tearDown() {}

// ---------------------------------------------------------------------------
// Role values and NFC form factor (SDK keys.proto / vcsec.proto)
// ---------------------------------------------------------------------------

static void test_role_value_mapping()
{
	// SDK keys.proto:7-15: ROLE_OWNER=2, ROLE_CHARGING_MANAGER=6.
	// Esp32Board::roleValue() maps stored role strings through roleValueFromName().
	TEST_ASSERT_EQUAL_UINT8(2, roleValueFromName("owner"));
	TEST_ASSERT_EQUAL_UINT8(6, roleValueFromName("charging_manager"));
	TEST_ASSERT_EQUAL_UINT8(2, ROLE_OWNER);
	TEST_ASSERT_EQUAL_UINT8(6, ROLE_CHARGING_MANAGER);
}

static void test_nfc_form_factor_value()
{
	// SDK vcsec.proto KeyFormFactor.NFC_CARD = 1. Production dispatch passes
	// this constant directly into buildAddKeyRequest().
	TEST_ASSERT_EQUAL_UINT8(1, KEY_FORM_FACTOR_NFC_CARD);
}

// ---------------------------------------------------------------------------
// Independent top-level protobuf scanner (test-only reference decoder)
// ---------------------------------------------------------------------------

struct TopField
{
	uint32_t number;
	uint8_t wire;
	const uint8_t *payload;
	size_t len;
	uint64_t varint;
	size_t tagOff;
	size_t tagLen;
};

static size_t scanTopFields(const uint8_t *buf, size_t len, TopField *out, size_t max)
{
	size_t i = 0;
	size_t n = 0;
	while (i < len && n < max)
	{
		const size_t tagOff = i;
		uint64_t tag = 0;
		unsigned shift = 0;
		while (i < len && shift < 63)
		{
			uint8_t b = buf[i++];
			tag |= (uint64_t)(b & 0x7F) << shift;
			if (!(b & 0x80))
				break;
			shift += 7;
		}
		TopField f;
		memset(&f, 0, sizeof(f));
		f.number = (uint32_t)(tag >> 3);
		f.wire = (uint8_t)(tag & 0x07);
		f.tagOff = tagOff;
		f.tagLen = i - tagOff;
		if (f.wire == 2)
		{
			uint64_t l = 0;
			shift = 0;
			while (i < len && shift < 63)
			{
				uint8_t b = buf[i++];
				l |= (uint64_t)(b & 0x7F) << shift;
				if (!(b & 0x80))
					break;
				shift += 7;
			}
			f.payload = buf + i;
			f.len = (size_t)l;
			i += (size_t)l;
		}
		else if (f.wire == 0)
		{
			uint64_t v = 0;
			shift = 0;
			while (i < len && shift < 63)
			{
				uint8_t b = buf[i++];
				v |= (uint64_t)(b & 0x7F) << shift;
				if (!(b & 0x80))
					break;
				shift += 7;
			}
			f.varint = v;
		}
		else
		{
			break;
		}
		out[n++] = f;
	}
	return n;
}

static const TopField *findField(const TopField *fields, size_t n, uint32_t number)
{
	for (size_t i = 0; i < n; i++)
	{
		if (fields[i].number == number)
			return &fields[i];
	}
	return nullptr;
}

// ---------------------------------------------------------------------------
// SessionInfoRequest: to_destination=6, session_info_request=14, uuid=51
// ---------------------------------------------------------------------------

static void test_session_info_request_tags()
{
	uint8_t eph[65], challenge[16], uuid[16], out[256];
	memset(eph, 0x01, sizeof(eph));
	memset(challenge, 0x02, sizeof(challenge));
	memset(uuid, 0x03, sizeof(uuid));

	size_t outLen = 0;
	TEST_ASSERT_TRUE(buildSessionInfoRequest(eph, challenge, DOMAIN_INFOTAINMENT, uuid, out, sizeof(out), outLen));

	TopField fields[8];
	size_t n = scanTopFields(out, outLen, fields, 8);
	TEST_ASSERT_EQUAL_UINT32(3, (uint32_t)n);

	// to_destination (field 6): literal SDK tag 0x32, payload = Destination{domain=3}
	const TopField *d = findField(fields, n, 6);
	TEST_ASSERT_NOT_NULL(d);
	TEST_ASSERT_EQUAL_UINT32(2, d->wire);
	TEST_ASSERT_EQUAL_UINT32(0x32, out[0]);
	const uint8_t expDest[] = {0x08, 0x03};
	TEST_ASSERT_EQUAL_UINT32(sizeof(expDest), (uint32_t)d->len);
	TEST_ASSERT_EQUAL_HEX8_ARRAY(expDest, d->payload, sizeof(expDest));

	// session_info_request (field 14): literal SDK tag 0x72, payload = SessionInfoRequest
	const TopField *s = findField(fields, n, 14);
	TEST_ASSERT_NOT_NULL(s);
	TEST_ASSERT_EQUAL_UINT32(2, s->wire);
	TEST_ASSERT_EQUAL_UINT32(0x72, out[s->tagOff]);
	uint8_t expSir[85];
	size_t k = 0;
	expSir[k++] = 0x0A; // public_key = field 1, wire 2
	expSir[k++] = 0x41; // len 65
	for (int i = 0; i < 65; i++)
		expSir[k++] = 0x01;
	expSir[k++] = 0x12; // challenge = field 2, wire 2
	expSir[k++] = 0x10; // len 16
	for (int i = 0; i < 16; i++)
		expSir[k++] = 0x02;
	TEST_ASSERT_EQUAL_UINT32(85, (uint32_t)k);
	TEST_ASSERT_EQUAL_UINT32(k, (uint32_t)s->len);
	TEST_ASSERT_EQUAL_HEX8_ARRAY(expSir, s->payload, k);

	// uuid (field 51): literal SDK tag bytes 0x9A 0x03, 16-byte payload
	const TopField *u = findField(fields, n, 51);
	TEST_ASSERT_NOT_NULL(u);
	TEST_ASSERT_EQUAL_UINT32(2, u->wire);
	TEST_ASSERT_EQUAL_UINT32(0x9A, out[u->tagOff]);
	TEST_ASSERT_EQUAL_UINT32(0x03, out[u->tagOff + 1]);
	TEST_ASSERT_EQUAL_UINT32(16, (uint32_t)u->len);
	TEST_ASSERT_EQUAL_HEX8_ARRAY(uuid, u->payload, 16);

	// Regression: old wrong tags must be gone (2/old-to, 6/old-sir, 14/old-uuid reused).
	TEST_ASSERT_NULL(findField(fields, n, 2));
	TEST_ASSERT_EQUAL_UINT32(2, (uint32_t)d->len); // field 6 is Destination, not the 85-byte sir
	TEST_ASSERT_EQUAL_UINT32(85, (uint32_t)s->len); // field 14 is sir, not the 16-byte uuid
}

// ---------------------------------------------------------------------------
// AddKey: to_destination=6, protobuf_message_as_bytes=10, uuid=51
// ---------------------------------------------------------------------------

static void test_add_key_tags()
{
	uint8_t pub[65], uuid[16], out[256];
	memset(pub, 0x11, sizeof(pub));
	memset(uuid, 0x03, sizeof(uuid));

	size_t outLen = 0;
	TEST_ASSERT_TRUE(buildAddKeyRequest(pub, ROLE_OWNER, KEY_FORM_FACTOR_NFC_CARD, uuid, out, sizeof(out), outLen));

	TopField fields[8];
	size_t n = scanTopFields(out, outLen, fields, 8);
	TEST_ASSERT_EQUAL_UINT32(3, (uint32_t)n);

	// to_destination (field 6): Destination{domain=VEHICLE_SECURITY=2}
	const TopField *d = findField(fields, n, 6);
	TEST_ASSERT_NOT_NULL(d);
	const uint8_t expDest[] = {0x08, 0x02};
	TEST_ASSERT_EQUAL_UINT32(sizeof(expDest), (uint32_t)d->len);
	TEST_ASSERT_EQUAL_HEX8_ARRAY(expDest, d->payload, sizeof(expDest));

	// protobuf_message_as_bytes (field 10): literal SDK tag 0x52
	const TopField *p = findField(fields, n, 10);
	TEST_ASSERT_NOT_NULL(p);
	TEST_ASSERT_EQUAL_UINT32(2, p->wire);
	TEST_ASSERT_EQUAL_UINT32(0x52, out[p->tagOff]);
	// UnsignedMessage{ f16 WhitelistOperation{ f5 PermissionChange{
	//   key=PublicKey{PublicKeyRaw}, keyRole=ROLE_OWNER=2 },
	//   f6 metadataForKey=KeyMetadata{formFactor=NFC_CARD=1} } }
	uint8_t expUm[80];
	size_t k = 0;
	expUm[k++] = 0x82; expUm[k++] = 0x01; expUm[k++] = 0x4D; // f16 WhitelistOperation, len 77
	expUm[k++] = 0x2A; expUm[k++] = 0x47;					// f5 addKey..., len 71
	expUm[k++] = 0x0A; expUm[k++] = 0x43;					// PermissionChange.key f1, len 67
	expUm[k++] = 0x0A; expUm[k++] = 0x41;					// PublicKey.PublicKeyRaw f1, len 65
	for (int i = 0; i < 65; i++)
		expUm[k++] = 0x11;
	expUm[k++] = 0x20; expUm[k++] = 0x02;					// keyRole f4 = ROLE_OWNER (SDK keys.proto=2)
	expUm[k++] = 0x32; expUm[k++] = 0x02; expUm[k++] = 0x08;
	expUm[k++] = 0x01;										// f6 metadataForKey, NFC_CARD=1 (sibling of f5)
	TEST_ASSERT_EQUAL_UINT32(80, (uint32_t)k);
	TEST_ASSERT_EQUAL_UINT32(k, (uint32_t)p->len);
	TEST_ASSERT_EQUAL_HEX8_ARRAY(expUm, p->payload, k);

	// uuid (field 51)
	const TopField *u = findField(fields, n, 51);
	TEST_ASSERT_NOT_NULL(u);
	TEST_ASSERT_EQUAL_UINT32(0x9A, out[u->tagOff]);
	TEST_ASSERT_EQUAL_UINT32(0x03, out[u->tagOff + 1]);
	TEST_ASSERT_EQUAL_UINT32(16, (uint32_t)u->len);
	TEST_ASSERT_EQUAL_HEX8_ARRAY(uuid, u->payload, 16);

	// Regression: old wrong tags 2 (to), 5 (payload), 14 (uuid) must be gone.
	TEST_ASSERT_NULL(findField(fields, n, 2));
	TEST_ASSERT_NULL(findField(fields, n, 5));
	TEST_ASSERT_NULL(findField(fields, n, 14));
}

// ---------------------------------------------------------------------------
// Authenticated message: to_destination=6, protobuf_message_as_bytes=10, uuid=51
// ---------------------------------------------------------------------------

static void test_auth_message_tags()
{
	uint8_t enc[8], tag[16], uuid[16], out[256];
	memset(enc, 0x30, sizeof(enc));
	memset(tag, 0x04, sizeof(tag));
	memset(uuid, 0x03, sizeof(uuid));

	size_t outLen = 0;
	TEST_ASSERT_TRUE(buildAuthMessage(enc, sizeof(enc), tag, 7, DOMAIN_INFOTAINMENT, uuid, out, sizeof(out), outLen));

	TopField fields[8];
	size_t n = scanTopFields(out, outLen, fields, 8);

	const TopField *d = findField(fields, n, 6);
	TEST_ASSERT_NOT_NULL(d);
	const uint8_t expDest[] = {0x08, 0x03};
	TEST_ASSERT_EQUAL_UINT32(sizeof(expDest), (uint32_t)d->len);
	TEST_ASSERT_EQUAL_HEX8_ARRAY(expDest, d->payload, sizeof(expDest));

	const TopField *p = findField(fields, n, 10);
	TEST_ASSERT_NOT_NULL(p);
	TEST_ASSERT_EQUAL_UINT32(0x52, out[p->tagOff]);
	TEST_ASSERT_EQUAL_UINT32(sizeof(enc), (uint32_t)p->len);
	TEST_ASSERT_EQUAL_HEX8_ARRAY(enc, p->payload, sizeof(enc));

	const TopField *u = findField(fields, n, 51);
	TEST_ASSERT_NOT_NULL(u);
	TEST_ASSERT_EQUAL_UINT32(0x9A, out[u->tagOff]);
	TEST_ASSERT_EQUAL_UINT32(0x03, out[u->tagOff + 1]);
	TEST_ASSERT_EQUAL_UINT32(16, (uint32_t)u->len);
	TEST_ASSERT_EQUAL_HEX8_ARRAY(uuid, u->payload, 16);

	// Regression: old wrong tags 2 (to) and 5 (payload) must be gone.
	TEST_ASSERT_NULL(findField(fields, n, 2));
	TEST_ASSERT_NULL(findField(fields, n, 5));
}

// ---------------------------------------------------------------------------
// parseSessionInfo must read session_info at field 15 (tag 0x7A)
// ---------------------------------------------------------------------------

static void test_parse_session_info_uses_field_15()
{
	// SDK vector: RoutableMessage{ field 15 = bytes DE AD BE EF }
	const uint8_t rm[] = {0x7A, 0x04, 0xDE, 0xAD, 0xBE, 0xEF};
	size_t siLen = 0;
	const uint8_t *p = parseSessionInfo(rm, sizeof(rm), siLen);
	TEST_ASSERT_NOT_NULL(p);
	TEST_ASSERT_EQUAL_UINT32(4, (uint32_t)siLen);
	const uint8_t exp[] = {0xDE, 0xAD, 0xBE, 0xEF};
	TEST_ASSERT_EQUAL_HEX8_ARRAY(exp, p, sizeof(exp));
}

static void test_parse_session_info_skips_other_fields()
{
	// Preceding field 6 (to_destination) must be skipped; field 15 still found.
	const uint8_t rm[] = {0x32, 0x02, 0x08, 0x03, 0x7A, 0x04, 0xDE, 0xAD, 0xBE, 0xEF};
	size_t siLen = 0;
	const uint8_t *p = parseSessionInfo(rm, sizeof(rm), siLen);
	TEST_ASSERT_NOT_NULL(p);
	TEST_ASSERT_EQUAL_UINT32(4, (uint32_t)siLen);
	TEST_ASSERT_EQUAL_HEX8(0xDE, p[0]);
	TEST_ASSERT_EQUAL_HEX8(0xEF, p[3]);
}

static void test_parse_session_info_rejects_old_field_10()
{
	// field 10 is protobuf_message_as_bytes, not session_info; must not parse.
	const uint8_t rm[] = {0x52, 0x04, 0xDE, 0xAD, 0xBE, 0xEF};
	size_t siLen = 0;
	TEST_ASSERT_NULL(parseSessionInfo(rm, sizeof(rm), siLen));
}

static void test_session_info_fields_round_trip_via_field_15()
{
	// RoutableMessage{ field 15 = SessionInfo{ public_key=010203, challenge=AABB } }
	const uint8_t rm[] = {0x7A, 0x09, 0x0A, 0x03, 0x01, 0x02, 0x03, 0x12, 0x02, 0xAA, 0xBB};
	size_t siLen = 0;
	const uint8_t *p = parseSessionInfo(rm, sizeof(rm), siLen);
	TEST_ASSERT_NOT_NULL(p);
	SessionInfoFields f = parseSessionInfoFields(p, siLen);
	TEST_ASSERT_TRUE(f.valid);
	TEST_ASSERT_EQUAL_UINT32(3, (uint32_t)f.publicKeyLen);
	const uint8_t expKey[] = {0x01, 0x02, 0x03};
	TEST_ASSERT_EQUAL_HEX8_ARRAY(expKey, f.publicKey, sizeof(expKey));
	TEST_ASSERT_EQUAL_UINT32(2, (uint32_t)f.challengeLen);
	const uint8_t expChal[] = {0xAA, 0xBB};
	TEST_ASSERT_EQUAL_HEX8_ARRAY(expChal, f.challenge, sizeof(expChal));
}

// ---------------------------------------------------------------------------
// SDK-authority placement: metadataForKey is field 6 of WhitelistOperation,
// a SIBLING of addKeyToWhitelistAndAddPermissions (f5), NOT a child of
// PermissionChange (SDK vcsec.proto:104-108,121-136). A flat byte pin alone
// cannot distinguish the two nestings — the byte order is identical and only
// the f5 length byte differs — so parse each message level independently.
// ---------------------------------------------------------------------------

static void test_add_key_metadata_is_whitelist_sibling_of_f5()
{
	uint8_t pub[65], uuid[16], out[256];
	memset(pub, 0x11, sizeof(pub));
	memset(uuid, 0x03, sizeof(uuid));

	size_t outLen = 0;
	TEST_ASSERT_TRUE(buildAddKeyRequest(pub, ROLE_OWNER, KEY_FORM_FACTOR_NFC_CARD, uuid, out, sizeof(out), outLen));

	TopField top[8];
	size_t nTop = scanTopFields(out, outLen, top, 8);
	const TopField *um = findField(top, nTop, 10);
	TEST_ASSERT_NOT_NULL(um);

	TopField umFields[8];
	size_t nUm = scanTopFields(um->payload, um->len, umFields, 8);
	const TopField *wl = findField(umFields, nUm, 16);
	TEST_ASSERT_NOT_NULL(wl);

	TopField wlFields[8];
	size_t nWl = scanTopFields(wl->payload, wl->len, wlFields, 8);
	const TopField *f5 = findField(wlFields, nWl, 5);
	const TopField *f6 = findField(wlFields, nWl, 6);
	TEST_ASSERT_NOT_NULL(f5); // addKeyToWhitelistAndAddPermissions
	TEST_ASSERT_NOT_NULL(f6); // metadataForKey MUST be here, beside f5

	// PermissionChange contains key (f1) and keyRole (f4) only — no f6 child.
	TopField pcFields[8];
	size_t nPc = scanTopFields(f5->payload, f5->len, pcFields, 8);
	TEST_ASSERT_NOT_NULL(findField(pcFields, nPc, 1));
	TEST_ASSERT_NOT_NULL(findField(pcFields, nPc, 4));
	TEST_ASSERT_NULL(findField(pcFields, nPc, 6));

	// f6 payload = KeyMetadata{ keyFormFactor(f1) = NFC_CARD=1 }.
	TopField metaFields[4];
	size_t nMeta = scanTopFields(f6->payload, f6->len, metaFields, 4);
	const TopField *fff = findField(metaFields, nMeta, 1);
	TEST_ASSERT_NOT_NULL(fff);
	TEST_ASSERT_EQUAL_UINT32(1, (uint32_t)fff->varint);
}

// ---------------------------------------------------------------------------
// Exact-wire helpers and SDK vector pins for every BLE builder
// ---------------------------------------------------------------------------

static void assertExactWire(const uint8_t *exp, size_t expLen, const uint8_t *got, size_t gotLen)
{
	TEST_ASSERT_EQUAL_UINT32((uint32_t)expLen, (uint32_t)gotLen);
	TEST_ASSERT_EQUAL_HEX8_ARRAY(exp, got, expLen);
}

static void test_enc_destination_exact_wire()
{
	uint8_t out[8];
	size_t n = 0;
	TEST_ASSERT_TRUE(encDestination(DOMAIN_VEHICLE_SECURITY, out, sizeof(out), n));
	const uint8_t expVs[] = {0x08, 0x02};
	assertExactWire(expVs, sizeof(expVs), out, n);

	n = 0;
	TEST_ASSERT_TRUE(encDestination(DOMAIN_INFOTAINMENT, out, sizeof(out), n));
	const uint8_t expIt[] = {0x08, 0x03};
	assertExactWire(expIt, sizeof(expIt), out, n);
}

static void test_session_info_request_exact_wire()
{
	uint8_t eph[65], chal[16], uuid[16], out[256];
	memset(eph, 0x01, sizeof(eph));
	memset(chal, 0x02, sizeof(chal));
	memset(uuid, 0x03, sizeof(uuid));

	size_t n = 0;
	TEST_ASSERT_TRUE(buildSessionInfoRequest(eph, chal, DOMAIN_INFOTAINMENT, uuid, out, sizeof(out), n));

	uint8_t exp[110];
	size_t k = 0;
	exp[k++] = 0x32; exp[k++] = 0x02; exp[k++] = 0x08; exp[k++] = 0x03; // field 6 Destination{3}
	exp[k++] = 0x72; exp[k++] = 0x55;									// field 14 SessionInfoRequest, len 85
	exp[k++] = 0x0A; exp[k++] = 0x41;									// public_key, len 65
	for (int i = 0; i < 65; i++) exp[k++] = 0x01;
	exp[k++] = 0x12; exp[k++] = 0x10;									// challenge, len 16
	for (int i = 0; i < 16; i++) exp[k++] = 0x02;
	exp[k++] = 0x9A; exp[k++] = 0x03; exp[k++] = 0x10;					// field 51 uuid, len 16
	for (int i = 0; i < 16; i++) exp[k++] = 0x03;
	TEST_ASSERT_EQUAL_UINT32(110, (uint32_t)k);
	assertExactWire(exp, k, out, n);
}

static void test_add_key_exact_wire()
{
	uint8_t pub[65], uuid[16], out[256];
	memset(pub, 0x11, sizeof(pub));
	memset(uuid, 0x03, sizeof(uuid));

	// SDK vector (vcsec.proto, read-only):
	//   UnsignedMessage.WhitelistOperation (f16)
	//     -> WhitelistOperation.addKeyToWhitelistAndAddPermissions (f5)
	//          -> PermissionChange.key (f1) = PublicKey.PublicKeyRaw (f1),
	//             keyRole (f4)
	//     -> WhitelistOperation.metadataForKey (f6) = KeyMetadata.keyFormFactor (f1)
	//        (SIBLING of f5 per vcsec.proto:121-136, NOT a child of PermissionChange)
	// SDK values pinned here: ROLE_OWNER=2 (keys.proto:7-15) and
	// KeyFormFactor.NFC_CARD=1 (vcsec.proto:32), both passed as production constants.
	//   {0x32,0x02,0x08,0x02,                                  to_destination = Destination{VS}
	//    0x52,0x50,                                            field 10 UnsignedMessage, len 80
	//      0x82,0x01,0x4D,                                     UnsignedMessage.WhitelistOperation (f16), len 77
	//        0x2A,0x47,                                        .addKeyToWhitelistAndAddPermissions (f5), len 71
	//          0x0A,0x43,                                      PermissionChange.key (f1), len 67
	//            0x0A,0x41,0x11 x65,                           PublicKey.PublicKeyRaw (f1), len 65
	//          0x20,0x02,                                      PermissionChange.keyRole (f4) = ROLE_OWNER
	//        0x32,0x02,0x08,0x01,                              .metadataForKey (f6), keyFormFactor=NFC_CARD
	//    0x9A,0x03,0x10,0x03 x16}                              uuid (f51)
	size_t n = 0;
	TEST_ASSERT_TRUE(buildAddKeyRequest(pub, ROLE_OWNER, KEY_FORM_FACTOR_NFC_CARD, uuid, out, sizeof(out), n));

	uint8_t exp[105];
	size_t k = 0;
	exp[k++] = 0x32; exp[k++] = 0x02; exp[k++] = 0x08; exp[k++] = 0x02; // Destination{VS}
	exp[k++] = 0x52; exp[k++] = 0x50;									// field 10 UnsignedMessage, len 80
	exp[k++] = 0x82; exp[k++] = 0x01; exp[k++] = 0x4D;					// f16 WhitelistOperation, len 77
	exp[k++] = 0x2A; exp[k++] = 0x47;									// f5 addKey..., len 71
	exp[k++] = 0x0A; exp[k++] = 0x43;									// PermissionChange.key f1, len 67
	exp[k++] = 0x0A; exp[k++] = 0x41;									// PublicKey.PublicKeyRaw f1, len 65
	for (int i = 0; i < 65; i++) exp[k++] = 0x11;
	exp[k++] = 0x20; exp[k++] = 0x02;									// keyRole f4 = ROLE_OWNER (SDK=2)
	exp[k++] = 0x32; exp[k++] = 0x02; exp[k++] = 0x08; exp[k++] = 0x01; // f6 metadataForKey (sibling of f5), NFC_CARD=1
	exp[k++] = 0x9A; exp[k++] = 0x03; exp[k++] = 0x10;					// uuid f51
	for (int i = 0; i < 16; i++) exp[k++] = 0x03;
	TEST_ASSERT_EQUAL_UINT32(105, (uint32_t)k);
	assertExactWire(exp, k, out, n);
}

static void test_add_key_charging_manager_exact_wire()
{
	uint8_t pub[65], uuid[16], out[256];
	memset(pub, 0x11, sizeof(pub));
	memset(uuid, 0x03, sizeof(uuid));

	// Same SDK nesting as the owner pin (metadataForKey f6 is a sibling of f5);
	// only keyRole changes to ROLE_CHARGING_MANAGER=6 (keys.proto:7-15).
	size_t n = 0;
	TEST_ASSERT_TRUE(
		buildAddKeyRequest(pub, ROLE_CHARGING_MANAGER, KEY_FORM_FACTOR_NFC_CARD, uuid, out, sizeof(out), n));

	uint8_t exp[105];
	size_t k = 0;
	exp[k++] = 0x32; exp[k++] = 0x02; exp[k++] = 0x08; exp[k++] = 0x02; // Destination{VS}
	exp[k++] = 0x52; exp[k++] = 0x50;									// field 10 UnsignedMessage, len 80
	exp[k++] = 0x82; exp[k++] = 0x01; exp[k++] = 0x4D;					// f16 WhitelistOperation, len 77
	exp[k++] = 0x2A; exp[k++] = 0x47;									// f5 addKey..., len 71
	exp[k++] = 0x0A; exp[k++] = 0x43;									// PermissionChange.key f1, len 67
	exp[k++] = 0x0A; exp[k++] = 0x41;									// PublicKey.PublicKeyRaw f1, len 65
	for (int i = 0; i < 65; i++) exp[k++] = 0x11;
	exp[k++] = 0x20; exp[k++] = 0x06;									// keyRole f4 = ROLE_CHARGING_MANAGER (SDK=6)
	exp[k++] = 0x32; exp[k++] = 0x02; exp[k++] = 0x08; exp[k++] = 0x01; // f6 metadataForKey (sibling of f5), NFC_CARD=1
	exp[k++] = 0x9A; exp[k++] = 0x03; exp[k++] = 0x10;					// uuid f51
	for (int i = 0; i < 16; i++) exp[k++] = 0x03;
	TEST_ASSERT_EQUAL_UINT32(105, (uint32_t)k);
	assertExactWire(exp, k, out, n);
}

static void test_auth_message_exact_wire()
{
	uint8_t enc[8], tag[16], uuid[16], out[256];
	memset(enc, 0x30, sizeof(enc));
	memset(tag, 0x04, sizeof(tag));
	memset(uuid, 0x03, sizeof(uuid));

	size_t n = 0;
	TEST_ASSERT_TRUE(buildAuthMessage(enc, sizeof(enc), tag, 7, DOMAIN_INFOTAINMENT, uuid, out, sizeof(out), n));

	// SDK vector (C1-14 recorded), 57 bytes:
	//   signature_data = field 13 (SDK universal_message.proto:94); field 7 is
	//   from_destination. SignedData.AES_GCM_personalized_data = field 5
	//   (SDK signatures.proto:80), counter = field 3 (:46), tag = field 5 (:48).
	//   32 02 08 03 | 52 08 30*8 | 6A 16 2A 14 18 07 2A 10 04*16 | 9A 03 10 03*16
	uint8_t exp[64];
	size_t k = 0;
	exp[k++] = 0x32; exp[k++] = 0x02; exp[k++] = 0x08; exp[k++] = 0x03;
	exp[k++] = 0x52; exp[k++] = 0x08;
	for (int i = 0; i < 8; i++) exp[k++] = 0x30;
	exp[k++] = 0x6A; exp[k++] = 0x16;									// signature_data f13, len 22
	exp[k++] = 0x2A; exp[k++] = 0x14;									// AES_GCM_personalized_data f5, len 20
	exp[k++] = 0x18; exp[k++] = 0x07;									// counter f3 = 7
	exp[k++] = 0x2A; exp[k++] = 0x10;									// tag f5, len 16
	for (int i = 0; i < 16; i++) exp[k++] = 0x04;
	exp[k++] = 0x9A; exp[k++] = 0x03; exp[k++] = 0x10;
	for (int i = 0; i < 16; i++) exp[k++] = 0x03;
	TEST_ASSERT_EQUAL_UINT32(57, (uint32_t)k);
	assertExactWire(exp, k, out, n);
}

static void test_action_envelope_exact_wire()
{
	// SDK vector (C1-14 recorded): single wrap at field 2 per car_server.proto:16.
	//   {0x12, 0x02, 0x32, 0x00}   Action{field2: VehicleAction{field6: empty}}
	uint8_t inner[8], out[64];
	Proto p(inner, sizeof(inner));
	TEST_ASSERT_TRUE(p.fieldEmpty(6));
	size_t n = 0;
	TEST_ASSERT_TRUE(buildAction(p, out, sizeof(out), n));
	const uint8_t exp[] = {0x12, 0x02, 0x32, 0x00};
	assertExactWire(exp, sizeof(exp), out, n);
}

static void test_wake_action_exact_wire()
{
	// LEGACY — no SDK vector; wake belongs to VCSEC RKE; see follow-up C1-23.
	// C1-14 marked this CHARACTERIZATION but recorded no literal SDK byte vector:
	// car_server.proto:42 has field 25 = vehicleControlCancelSoftwareUpdateAction and
	// no wake field. Controller ruling C1-17: keep the pre-C1-16 double-wrap wire;
	// do not mechanically flip to the single-wrap Action envelope without SDK evidence.
	uint8_t out[32];
	size_t n = 0;
	TEST_ASSERT_TRUE(buildWakeAction(out, sizeof(out), n));
	const uint8_t exp[] = {0x0A, 0x05, 0x0A, 0x03, 0xCA, 0x01, 0x00};
	assertExactWire(exp, sizeof(exp), out, n);
}

static void test_charge_actions_exact_wire()
{
	// SDK vectors (C1-14 recorded): chargingStartStopAction = 6 (car_server.proto:27),
	// start = field 2 (:186), stop = field 5 (:189), Action single wrap at field 2.
	uint8_t out[32];
	size_t n = 0;
	TEST_ASSERT_TRUE(buildChargeStartAction(out, sizeof(out), n));
	const uint8_t expStart[] = {0x12, 0x04, 0x32, 0x02, 0x12, 0x00};
	assertExactWire(expStart, sizeof(expStart), out, n);

	n = 0;
	TEST_ASSERT_TRUE(buildChargeStopAction(out, sizeof(out), n));
	const uint8_t expStop[] = {0x12, 0x04, 0x32, 0x02, 0x2A, 0x00};
	assertExactWire(expStop, sizeof(expStop), out, n);
}

static void test_set_amps_limit_exact_wire()
{
	// SDK vectors: setChargingAmpsAction = 43 (car_server.proto:56) -> tag DA 02;
	// chargingSetLimitAction = 5 (:26) -> tag 2A 02. Action single wrap at field 2.
	// C1-14 recorded the amps tag as 9A 02 (field 35); that was a transcription
	// error. Controller ruling C1-17: SDK proto is authority, corrected here to DA 02.
	uint8_t out[32];
	size_t n = 0;
	TEST_ASSERT_TRUE(buildSetAmpsAction(32, out, sizeof(out), n));
	const uint8_t expAmps[] = {0x12, 0x05, 0xDA, 0x02, 0x02, 0x08, 0x20};
	assertExactWire(expAmps, sizeof(expAmps), out, n);

	n = 0;
	TEST_ASSERT_TRUE(buildSetLimitAction(80, out, sizeof(out), n));
	const uint8_t expLimit[] = {0x12, 0x04, 0x2A, 0x02, 0x08, 0x50};
	assertExactWire(expLimit, sizeof(expLimit), out, n);
}

static void test_climate_action_exact_wire()
{
	// SDK vectors (C1-14 recorded): hvacAutoAction = 10 (car_server.proto:31),
	// power_on = field 1, manual_override = field 2 (:211); Action field 2 wrap.
	uint8_t out[32];
	size_t n = 0;
	TEST_ASSERT_TRUE(buildClimateAction(true, out, sizeof(out), n));
	const uint8_t expOn[] = {0x12, 0x06, 0x52, 0x04, 0x08, 0x01, 0x10, 0x00};
	assertExactWire(expOn, sizeof(expOn), out, n);

	n = 0;
	TEST_ASSERT_TRUE(buildClimateAction(false, out, sizeof(out), n));
	const uint8_t expOff[] = {0x12, 0x06, 0x52, 0x04, 0x08, 0x00, 0x10, 0x00};
	assertExactWire(expOff, sizeof(expOff), out, n);
}

static void test_vcsec_rke_exact_wire()
{
	uint8_t out[32];
	size_t n = 0;
	TEST_ASSERT_TRUE(buildVCSECRKEAction(RKE_ACTION_LOCK, out, sizeof(out), n));
	const uint8_t expLock[] = {0x0A, 0x06, 0x12, 0x02, 0x10, 0x01, 0x18, 0x02};
	assertExactWire(expLock, sizeof(expLock), out, n);

	n = 0;
	TEST_ASSERT_TRUE(buildVCSECRKEAction(RKE_ACTION_UNLOCK, out, sizeof(out), n));
	const uint8_t expUnlock[] = {0x0A, 0x06, 0x12, 0x02, 0x10, 0x00, 0x18, 0x02};
	assertExactWire(expUnlock, sizeof(expUnlock), out, n);

	n = 0;
	TEST_ASSERT_TRUE(buildVCSECWakeAction(out, sizeof(out), n));
	const uint8_t expWake[] = {0x0A, 0x06, 0x12, 0x02, 0x10, 0x1E, 0x18, 0x02};
	assertExactWire(expWake, sizeof(expWake), out, n);
}

static void test_vcsec_closure_exact_wire()
{
	uint8_t out[32];
	size_t n = 0;
	TEST_ASSERT_TRUE(buildVCSECClosureAction(CLOSURE_FIELD_REAR_TRUNK, CLOSURE_MOVE_TYPE_OPEN, out, sizeof(out), n));
	const uint8_t expTrunkOpen[] = {0x0A, 0x08, 0x12, 0x04, 0x22, 0x02, 0x28, 0x03, 0x18, 0x02};
	assertExactWire(expTrunkOpen, sizeof(expTrunkOpen), out, n);

	n = 0;
	TEST_ASSERT_TRUE(buildVCSECClosureAction(CLOSURE_FIELD_FRONT_TRUNK, CLOSURE_MOVE_TYPE_CLOSE, out, sizeof(out), n));
	const uint8_t expFrunkClose[] = {0x0A, 0x08, 0x12, 0x04, 0x22, 0x02, 0x30, 0x04, 0x18, 0x02};
	assertExactWire(expFrunkClose, sizeof(expFrunkClose), out, n);
}

int main()
{
	UNITY_BEGIN();

	RUN_TEST(test_role_value_mapping);
	RUN_TEST(test_nfc_form_factor_value);

	RUN_TEST(test_session_info_request_tags);
	RUN_TEST(test_add_key_tags);
	RUN_TEST(test_auth_message_tags);
	RUN_TEST(test_parse_session_info_uses_field_15);
	RUN_TEST(test_parse_session_info_skips_other_fields);
	RUN_TEST(test_parse_session_info_rejects_old_field_10);
	RUN_TEST(test_session_info_fields_round_trip_via_field_15);

	RUN_TEST(test_enc_destination_exact_wire);
	RUN_TEST(test_session_info_request_exact_wire);
	RUN_TEST(test_add_key_exact_wire);
	RUN_TEST(test_add_key_charging_manager_exact_wire);
	RUN_TEST(test_add_key_metadata_is_whitelist_sibling_of_f5);
	RUN_TEST(test_auth_message_exact_wire);
	RUN_TEST(test_action_envelope_exact_wire);
	RUN_TEST(test_wake_action_exact_wire);
	RUN_TEST(test_charge_actions_exact_wire);
	RUN_TEST(test_set_amps_limit_exact_wire);
	RUN_TEST(test_climate_action_exact_wire);
	RUN_TEST(test_vcsec_rke_exact_wire);
	RUN_TEST(test_vcsec_closure_exact_wire);

	return UNITY_END();
}
