// ── Single-Shot TX Tests ────────────────────────────────────────────────────
// Tests single-shot TX mode enable/disable via command parsing.

#include <unity.h>
#include <cstring>

class __FlashStringHelper;
#define F(s) (reinterpret_cast<const __FlashStringHelper *>(s))

#define BUS_CHASSIS_ACTIVE 1
#define BUS_VEHICLE_ACTIVE 1
#define BUS_BODY_ACTIVE 1
#define BOARD_CAN_CLOCK_MHZ 8
#define BOARD_ENABLE_WIFI 0
#define BOARD_ENABLE_BLE 0

#include "core/types.h"

// ── Stubs ────────────────────────────────────────────────────────────────────
void saveSettings(const State &) {}
void sendLog(const char *) {}
void sendLog(const __FlashStringHelper *) {}

#include "feature/single_shot.h"

void setUp() {}
void tearDown() {}

// ── Tests ───────────────────────────────────────────────────────────────────

void test_singleshot_on_enables()
{
	State s = {};
	bool ok = execSingleShotCmd("singleshot:on", s);
	TEST_ASSERT_TRUE(ok);
	TEST_ASSERT_TRUE(s.singleShotTx);
}

void test_singleshot_off_disables()
{
	State s = {};
	s.singleShotTx = true;
	bool ok = execSingleShotCmd("singleshot:off", s);
	TEST_ASSERT_TRUE(ok);
	TEST_ASSERT_FALSE(s.singleShotTx);
}

void test_singleshot_unrelated_returns_false()
{
	State s = {};
	bool ok = execSingleShotCmd("fsd:on", s);
	TEST_ASSERT_FALSE(ok);
}

void test_singleshot_default_off()
{
	State s = {};
	TEST_ASSERT_FALSE(s.singleShotTx);
}

int main()
{
	UNITY_BEGIN();
	RUN_TEST(test_singleshot_on_enables);
	RUN_TEST(test_singleshot_off_disables);
	RUN_TEST(test_singleshot_unrelated_returns_false);
	RUN_TEST(test_singleshot_default_off);
	return UNITY_END();
}
