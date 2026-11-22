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

using namespace Tesla;

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
	TEST_ASSERT_TRUE(buildAddKeyRequest(pub, 4, KEY_FORM_FACTOR_NFC_CARD, uuid, out, sizeof(out), outLen));

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
	// UnsignedMessage{ field 2 = AddKey{ key, role=4, form_factor=3 } }
	uint8_t expUm[73];
	size_t k = 0;
	expUm[k++] = 0x12; // addKeyToWhitelistAndAddPermissions = field 2, wire 2
	expUm[k++] = 0x47; // AddKey len = 71
	expUm[k++] = 0x0A; // key = field 1
	expUm[k++] = 0x41; // len 65
	for (int i = 0; i < 65; i++)
		expUm[k++] = 0x11;
	expUm[k++] = 0x10; // role = field 2, wire 0
	expUm[k++] = 0x04; // ROLE_OWNER
	expUm[k++] = 0x28; // form_factor = field 5, wire 0
	expUm[k++] = 0x03; // KEY_FORM_FACTOR_NFC_CARD
	TEST_ASSERT_EQUAL_UINT32(73, (uint32_t)k);
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

	return UNITY_END();
}
