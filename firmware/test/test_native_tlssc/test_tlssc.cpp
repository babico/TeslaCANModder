// ── TLSSC Restore Tests ───────────────────────────────────────────────────────
// Tests executeTlsscCmd() and handleTlssc() from feature/tlssc.h.

#include <unity.h>
#include <cstring>

class __FlashStringHelper;
#define F(s) (reinterpret_cast<const __FlashStringHelper *>(s))

#define BUS_CHASSIS_ACTIVE 1
#define BUS_VEHICLE_ACTIVE 1
#define BUS_BODY_ACTIVE 1
#define BOARD_CAN_CLOCK_MHZ 8
#define BOARD_ENABLE_BLE 0
#define BOARD_ENABLE_WIFI 0

#include "core/types.h"
#include "infra/can/bus.h"

static int saveCount = 0;
void saveSettings(const State &)
{
	saveCount++;
}
void resetHandlerLogFlags() {}
void applyFilters(State &) {}

#include "infra/util/parse.h"
#include "feature/tlssc.h"
#include "support/helpers.h"

static State makeState()
{
	State s = {};
	return s;
}

void setUp()
{
	saveCount = 0;
}
void tearDown() {}

// ── executeTlsscCmd ───────────────────────────────────────────────────────────

void test_tlssc_cmd_on_enables()
{
	State s = makeState();
	TEST_ASSERT_TRUE(executeTlsscCmd("tlssc:on", s));
	TEST_ASSERT_TRUE(s.tlsscRestore);
	TEST_ASSERT_EQUAL(1, saveCount);
}

void test_tlssc_cmd_off_disables()
{
	State s = makeState();
	s.tlsscRestore = true;
	TEST_ASSERT_TRUE(executeTlsscCmd("tlssc:off", s));
	TEST_ASSERT_FALSE(s.tlsscRestore);
	TEST_ASSERT_EQUAL(1, saveCount);
}

void test_tlssc_cmd_unrelated_returns_false()
{
	State s = makeState();
	TEST_ASSERT_FALSE(executeTlsscCmd("fsd:on", s));
	TEST_ASSERT_EQUAL(0, saveCount);
}

void test_tlssc_cmd_invalid_arg_returns_false()
{
	State s = makeState();
	TEST_ASSERT_FALSE(executeTlsscCmd("tlssc:yes", s));
	TEST_ASSERT_EQUAL(0, saveCount);
}

// ── handleTlssc ───────────────────────────────────────────────────────────────

void test_tlssc_disabled_does_not_modify_frame()
{
	State s = makeState();
	s.tlsscRestore = false;
	Frame f = makeFrame(CAN_ID_DAS_AP_CONFIG);
	f.data[0] = 0x00;
	bool result = handleTlssc(f, s);
	TEST_ASSERT_FALSE(result);
	TEST_ASSERT_EQUAL_HEX8(0x00, f.data[0]);
}

void test_tlssc_enabled_sets_lower6_to_0x1b()
{
	State s = makeState();
	s.tlsscRestore = true;
	Frame f = makeFrame(CAN_ID_DAS_AP_CONFIG);
	f.data[0] = 0x00;
	bool result = handleTlssc(f, s);
	TEST_ASSERT_TRUE(result);
	TEST_ASSERT_EQUAL_HEX8(0x1B, f.data[0] & 0x3F);
}

void test_tlssc_preserves_upper_2_bits()
{
	State s = makeState();
	s.tlsscRestore = true;
	Frame f = makeFrame(CAN_ID_DAS_AP_CONFIG);
	f.data[0] = 0b11000000; // counter bits = 3
	bool result = handleTlssc(f, s);
	TEST_ASSERT_TRUE(result);
	TEST_ASSERT_EQUAL_HEX8(0b11000000, f.data[0] & 0xC0);
	TEST_ASSERT_EQUAL_HEX8(0x1B, f.data[0] & 0x3F);
}

void test_tlssc_already_correct_returns_false()
{
	State s = makeState();
	s.tlsscRestore = true;
	Frame f = makeFrame(CAN_ID_DAS_AP_CONFIG);
	// data[0] lower 6 bits already = 0x1B
	f.data[0] = (0x40 | 0x1B);
	bool result = handleTlssc(f, s);
	TEST_ASSERT_FALSE(result); // no change needed
}

void test_tlssc_short_frame_ignored()
{
	State s = makeState();
	s.tlsscRestore = true;
	Frame f = makeFrame(CAN_ID_DAS_AP_CONFIG, 0);
	bool result = handleTlssc(f, s);
	TEST_ASSERT_FALSE(result);
}

void test_tlssc_varies_counter_bits_correctly()
{
	State s = makeState();
	s.tlsscRestore = true;
	for (uint8_t counter = 0; counter < 4; counter++)
	{
		Frame f = makeFrame(CAN_ID_DAS_AP_CONFIG);
		f.data[0] = (uint8_t)(counter << 6) | 0x00;
		handleTlssc(f, s);
		TEST_ASSERT_EQUAL_HEX8((uint8_t)(counter << 6) | 0x1B, f.data[0]);
	}
}

int main()
{
	UNITY_BEGIN();

	RUN_TEST(test_tlssc_cmd_on_enables);
	RUN_TEST(test_tlssc_cmd_off_disables);
	RUN_TEST(test_tlssc_cmd_unrelated_returns_false);
	RUN_TEST(test_tlssc_cmd_invalid_arg_returns_false);

	RUN_TEST(test_tlssc_disabled_does_not_modify_frame);
	RUN_TEST(test_tlssc_enabled_sets_lower6_to_0x1b);
	RUN_TEST(test_tlssc_preserves_upper_2_bits);
	RUN_TEST(test_tlssc_already_correct_returns_false);
	RUN_TEST(test_tlssc_short_frame_ignored);
	RUN_TEST(test_tlssc_varies_counter_bits_correctly);

	return UNITY_END();
}
