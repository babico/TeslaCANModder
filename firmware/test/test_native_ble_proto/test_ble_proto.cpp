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

// SDK KeyFormFactor.NFC_CARD = 1 (vcsec.proto:32). Production still defines
// KEY_FORM_FACTOR_NFC_CARD = 3; aligning that constant is C1-03 scope.
static const uint8_t SDK_KEY_FORM_FACTOR_NFC_CARD = 1;

void setUp() {}
void tearDown() {}

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
	TEST_ASSERT_TRUE(buildAddKeyRequest(pub, 4, SDK_KEY_FORM_FACTOR_NFC_CARD, uuid, out, sizeof(out), outLen));

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
	//   key=PublicKey{PublicKeyRaw}, keyRole=4,
	//   metadataForKey=KeyMetadata{formFactor=1} } } }
	uint8_t expUm[80];
	size_t k = 0;
	expUm[k++] = 0x82; expUm[k++] = 0x01; expUm[k++] = 0x4D; // f16 WhitelistOperation, len 77
	expUm[k++] = 0x2A; expUm[k++] = 0x4B;					// f5 addKey..., len 75
	expUm[k++] = 0x0A; expUm[k++] = 0x43;					// PermissionChange.key f1, len 67
	expUm[k++] = 0x0A; expUm[k++] = 0x41;					// PublicKey.PublicKeyRaw f1, len 65
	for (int i = 0; i < 65; i++)
		expUm[k++] = 0x11;
	expUm[k++] = 0x20; expUm[k++] = 0x04;					// keyRole f4
	expUm[k++] = 0x32; expUm[k++] = 0x02; expUm[k++] = 0x08;
	expUm[k++] = 0x01;										// metadataForKey f6, formFactor=1
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
	//       -> PermissionChange.key (f1) = PublicKey.PublicKeyRaw (f1),
	//          keyRole (f4), metadataForKey (f6) = KeyMetadata.keyFormFactor (f1).
	// SDK KeyFormFactor.NFC_CARD=1 is the recorded input; the production constant
	// KEY_FORM_FACTOR_NFC_CARD=3 and ROLE_OWNER=2 are C1-03 scope. This pin keeps
	// the existing keyRole value 4.
	//   {0x32,0x02,0x08,0x02,                                  to_destination = Destination{VS}
	//    0x52,0x50,                                            field 10 UnsignedMessage, len 80
	//      0x82,0x01,0x4D,                                     UnsignedMessage.WhitelistOperation (f16), len 77
	//        0x2A,0x4B,                                        .addKeyToWhitelistAndAddPermissions (f5), len 75
	//          0x0A,0x43,                                      PermissionChange.key (f1), len 67
	//            0x0A,0x41,0x11 x65,                           PublicKey.PublicKeyRaw (f1), len 65
	//          0x20,0x04,                                      PermissionChange.keyRole (f4)
	//          0x32,0x02,0x08,0x01,                            .metadataForKey (f6), keyFormFactor=1
	//    0x9A,0x03,0x10,0x03 x16}                              uuid (f51)
	size_t n = 0;
	TEST_ASSERT_TRUE(buildAddKeyRequest(pub, 4, SDK_KEY_FORM_FACTOR_NFC_CARD, uuid, out, sizeof(out), n));

	uint8_t exp[105];
	size_t k = 0;
	exp[k++] = 0x32; exp[k++] = 0x02; exp[k++] = 0x08; exp[k++] = 0x02; // Destination{VS}
	exp[k++] = 0x52; exp[k++] = 0x50;									// field 10 UnsignedMessage, len 80
	exp[k++] = 0x82; exp[k++] = 0x01; exp[k++] = 0x4D;					// f16 WhitelistOperation, len 77
	exp[k++] = 0x2A; exp[k++] = 0x4B;									// f5 addKey..., len 75
	exp[k++] = 0x0A; exp[k++] = 0x43;									// PermissionChange.key f1, len 67
	exp[k++] = 0x0A; exp[k++] = 0x41;									// PublicKey.PublicKeyRaw f1, len 65
	for (int i = 0; i < 65; i++) exp[k++] = 0x11;
	exp[k++] = 0x20; exp[k++] = 0x04;									// keyRole f4 = 4 (existing value)
	exp[k++] = 0x32; exp[k++] = 0x02; exp[k++] = 0x08; exp[k++] = 0x01; // metadataForKey f6, formFactor=1
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

	// CHARACTERIZATION — current defective wire; flip to SDK vector with the production fix.
	// DEFECT (reported, not fixed in this task): firmware/lib/vehicle/ble/msg.h:313
	// writes signature_data at field 7, which SDK universal_message.proto:85 defines
	// as from_destination; SDK universal_message.proto:94 says field 13. Inner fields
	// are also wrong: msg.h:294-301 uses field 2 -> field 1 / tag=1, counter=2,
	// component=3, but SDK signatures.proto:80/46/48 defines field 5 -> counter=3, tag=5.
	// SDK-correct 57-byte vector: 32 02 08 03 | 52 08 30*8 |
	//   6A 16 2A 14 18 07 2A 10 04*16 | 9A 03 10 03*16
	// The pin below is the CURRENT 59-byte wire until production is fixed.
	uint8_t exp[64];
	size_t k = 0;
	exp[k++] = 0x32; exp[k++] = 0x02; exp[k++] = 0x08; exp[k++] = 0x03;
	exp[k++] = 0x52; exp[k++] = 0x08;
	for (int i = 0; i < 8; i++) exp[k++] = 0x30;
	exp[k++] = 0x3A; exp[k++] = 0x18;									// field 7 (from_destination), len 24
	exp[k++] = 0x12; exp[k++] = 0x16;									// field 2, len 22
	exp[k++] = 0x0A; exp[k++] = 0x10;									// tag at field 1, len 16
	for (int i = 0; i < 16; i++) exp[k++] = 0x04;
	exp[k++] = 0x10; exp[k++] = 0x07;									// counter at field 2
	exp[k++] = 0x18; exp[k++] = 0x03;									// component at field 3
	exp[k++] = 0x9A; exp[k++] = 0x03; exp[k++] = 0x10;
	for (int i = 0; i < 16; i++) exp[k++] = 0x03;
	TEST_ASSERT_EQUAL_UINT32(59, (uint32_t)k);
	assertExactWire(exp, k, out, n);
}

static void test_action_envelope_exact_wire()
{
	// CHARACTERIZATION — current defective wire; flip to SDK vector with the production fix.
	// DEFECT (reported, not fixed): msg.h:150 and msg.h:153 both wrap at field 1,
	// producing Action{field1:{field1:VehicleAction}}. SDK car_server.proto:16
	// defines a single Action{field2: VehicleAction}.
	// SDK-correct literal byte vector for this input (inner VehicleAction = 32 00), 4 bytes:
	//   {0x12, 0x02, 0x32, 0x00}   Action{field2: VehicleAction{field6: empty}}
	uint8_t inner[8], out[64];
	Proto p(inner, sizeof(inner));
	TEST_ASSERT_TRUE(p.fieldEmpty(6));
	size_t n = 0;
	TEST_ASSERT_TRUE(buildAction(p, out, sizeof(out), n));
	const uint8_t exp[] = {0x0A, 0x04, 0x0A, 0x02, 0x32, 0x00};
	assertExactWire(exp, sizeof(exp), out, n);
}

static void test_wake_action_exact_wire()
{
	// CHARACTERIZATION — current defective wire; flip to SDK vector with the production fix.
	// DEFECT (reported, not fixed): msg.h:169 uses fieldEmpty(25), but SDK
	// car_server.proto:42 has field 25 = vehicleControlCancelSoftwareUpdateAction;
	// there is no wake field in CarServer.VehicleAction (wake is a VCSEC RKE action).
	// msg.h:150/153 also double-wraps at field 1 instead of SDK Action.field 2.
	uint8_t out[32];
	size_t n = 0;
	TEST_ASSERT_TRUE(buildWakeAction(out, sizeof(out), n));
	const uint8_t exp[] = {0x0A, 0x05, 0x0A, 0x03, 0xCA, 0x01, 0x00};
	assertExactWire(exp, sizeof(exp), out, n);
}

static void test_charge_actions_exact_wire()
{
	// CHARACTERIZATION — current defective wire; flip to SDK vector with the production fix.
	// DEFECT (reported, not fixed): msg.h:191/213 use VehicleAction field 11; SDK
	// car_server.proto:27 says chargingStartStopAction = 6. msg.h:189 uses field 1
	// for start (SDK car_server.proto:186 says 2) and msg.h:211 uses field 2 for
	// stop (SDK car_server.proto:189 says 5). Action wrapper also wrong: msg.h:150/153.
	// SDK vectors: start 12 04 32 02 12 00 | stop 12 04 32 02 2A 00.
	uint8_t out[32];
	size_t n = 0;
	TEST_ASSERT_TRUE(buildChargeStartAction(out, sizeof(out), n));
	const uint8_t expStart[] = {0x0A, 0x08, 0x0A, 0x06, 0x5A, 0x04, 0x0A, 0x02, 0x0A, 0x00};
	assertExactWire(expStart, sizeof(expStart), out, n);

	n = 0;
	TEST_ASSERT_TRUE(buildChargeStopAction(out, sizeof(out), n));
	const uint8_t expStop[] = {0x0A, 0x08, 0x0A, 0x06, 0x5A, 0x04, 0x12, 0x02, 0x12, 0x00};
	assertExactWire(expStop, sizeof(expStop), out, n);
}

static void test_set_amps_limit_exact_wire()
{
	// CHARACTERIZATION — current defective wire; flip to SDK vector with the production fix.
	// DEFECT (reported, not fixed): msg.h:231 uses field 13 but SDK
	// car_server.proto:56 says setChargingAmpsAction = 43; msg.h:249 uses field 12
	// but SDK car_server.proto:26 says chargingSetLimitAction = 5.
	// SDK vectors: amps 12 05 9A 02 02 08 20 | limit 12 04 2A 02 08 50.
	uint8_t out[32];
	size_t n = 0;
	TEST_ASSERT_TRUE(buildSetAmpsAction(32, out, sizeof(out), n));
	const uint8_t expAmps[] = {0x0A, 0x06, 0x0A, 0x04, 0x6A, 0x02, 0x08, 0x20};
	assertExactWire(expAmps, sizeof(expAmps), out, n);

	n = 0;
	TEST_ASSERT_TRUE(buildSetLimitAction(80, out, sizeof(out), n));
	const uint8_t expLimit[] = {0x0A, 0x06, 0x0A, 0x04, 0x62, 0x02, 0x08, 0x50};
	assertExactWire(expLimit, sizeof(expLimit), out, n);
}

static void test_climate_action_exact_wire()
{
	// CHARACTERIZATION — current defective wire; flip to SDK vector with the production fix.
	// DEFECT (reported, not fixed): msg.h:267 uses field 14 but SDK
	// car_server.proto:31 says hvacAutoAction = 10. msg.h emits only power_on;
	// SDK HvacAutoAction has manual_override at field 2 (car_server.proto:211).
	// SDK vectors: on 12 06 52 04 08 01 10 00 | off 12 06 52 04 08 00 10 00.
	uint8_t out[32];
	size_t n = 0;
	TEST_ASSERT_TRUE(buildClimateAction(true, out, sizeof(out), n));
	const uint8_t expOn[] = {0x0A, 0x06, 0x0A, 0x04, 0x72, 0x02, 0x08, 0x01};
	assertExactWire(expOn, sizeof(expOn), out, n);

	n = 0;
	TEST_ASSERT_TRUE(buildClimateAction(false, out, sizeof(out), n));
	const uint8_t expOff[] = {0x0A, 0x06, 0x0A, 0x04, 0x72, 0x02, 0x08, 0x00};
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
