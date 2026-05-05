// ── Gamepad Axis Math Tests ──────────────────────────────────────────────────
// Pure unit tests for client/gamepad/axis_math.h. No NimBLE / Arduino deps.
//
// Behaviors covered:
//   - Stick center → 0
//   - Stick deadzone suppression
//   - Stick saturation at ±128 raw → ±1.0
//   - Stick inversion flag
//   - Trigger min (raw=0) → 0, trigger max (raw=255) → +1.0
//   - Trigger deadzone shifts curve start
//   - Expo curve compresses mid-range, preserves endpoints
//   - Expo k clamped to 1.0 for out-of-range input

#include <unity.h>
#include <cmath>
#include <cstdint>

#include "client/gamepad/axis_math.h"

static constexpr float EPS = 1e-3f;

void setUp(void) {}
void tearDown(void) {}

// ── Stick: center & deadzone ────────────────────────────────────────────────
static void test_stick_center_returns_zero(void)
{
	TEST_ASSERT_FLOAT_WITHIN(EPS, 0.0f, gpAxisMath(0, 128, 6, 0, false));
}

static void test_stick_inside_deadzone_returns_zero(void)
{
	// raw=130 → v=2; dz=6 → suppressed
	TEST_ASSERT_FLOAT_WITHIN(EPS, 0.0f, gpAxisMath(0, 130, 6, 0, false));
	TEST_ASSERT_FLOAT_WITHIN(EPS, 0.0f, gpAxisMath(0, 125, 6, 0, false));
}

static void test_stick_just_outside_deadzone_is_small(void)
{
	// raw=135 → v=7; dz=6 → v→1; n=1/(128-6)=~0.0082
	float n = gpAxisMath(0, 135, 6, 0, false);
	TEST_ASSERT_TRUE(n > 0.0f);
	TEST_ASSERT_FLOAT_WITHIN(0.01f, 1.0f / 122.0f, n);
}

static void test_stick_max_positive_near_one(void)
{
	// raw=255 → v=127 (not 128, uint8 max), v=127-6=121, n=121/122≈0.9918
	float n = gpAxisMath(0, 255, 6, 0, false);
	TEST_ASSERT_FLOAT_WITHIN(0.01f, 121.0f / 122.0f, n);
	TEST_ASSERT_TRUE(n <= 1.0f);
}

static void test_stick_min_negative_saturates_to_neg_one(void)
{
	// raw=0 → v=-128, abs(v)=128 > scale; v→-(128-6)=-122; n=-122/122=-1.0
	TEST_ASSERT_FLOAT_WITHIN(EPS, -1.0f, gpAxisMath(0, 0, 6, 0, false));
}

static void test_stick_clamps_when_overdriven(void)
{
	// dz=0, raw=0 → v=-128, n=-128/128=-1.0 exact
	TEST_ASSERT_FLOAT_WITHIN(EPS, -1.0f, gpAxisMath(0, 0, 0, 0, false));
}

static void test_stick_zero_dz_linear(void)
{
	// raw=192 → v=64; n=64/128=0.5
	TEST_ASSERT_FLOAT_WITHIN(EPS, 0.5f, gpAxisMath(1, 192, 0, 0, false));
}

// ── Stick: inversion ────────────────────────────────────────────────────────
static void test_stick_inversion_flips_sign(void)
{
	float pos = gpAxisMath(0, 200, 0, 0, false);
	float inv = gpAxisMath(0, 200, 0, 0, true);
	TEST_ASSERT_FLOAT_WITHIN(EPS, pos, -inv);
	TEST_ASSERT_TRUE(pos > 0.0f);
}

// ── Trigger: range ──────────────────────────────────────────────────────────
static void test_trigger_zero_returns_zero(void)
{
	TEST_ASSERT_FLOAT_WITHIN(EPS, 0.0f, gpAxisMath(4, 0, 8, 0, false));
	TEST_ASSERT_FLOAT_WITHIN(EPS, 0.0f, gpAxisMath(5, 0, 8, 0, false));
}

static void test_trigger_max_returns_one(void)
{
	TEST_ASSERT_FLOAT_WITHIN(EPS, 1.0f, gpAxisMath(4, 255, 8, 0, false));
	TEST_ASSERT_FLOAT_WITHIN(EPS, 1.0f, gpAxisMath(5, 255, 8, 0, false));
}

static void test_trigger_inside_deadzone_returns_zero(void)
{
	// dz=8, raw=5 → suppressed
	TEST_ASSERT_FLOAT_WITHIN(EPS, 0.0f, gpAxisMath(4, 5, 8, 0, false));
}

static void test_trigger_above_deadzone_normalized(void)
{
	// raw=131 with dz=8 → v=131-8=123; denom=255-8=247; n≈0.498
	float n = gpAxisMath(4, 131, 8, 0, false);
	TEST_ASSERT_FLOAT_WITHIN(0.01f, 123.0f / 247.0f, n);
}

// ── Expo curve ──────────────────────────────────────────────────────────────
static void test_expo_preserves_endpoints(void)
{
	// At |n|=1.0, expo blend leaves it unchanged: (1-k)*1 + k*1 = 1.
	// Use trigger axis (raw=255 reaches exactly 1.0); stick raw=0 also reaches -1.0.
	TEST_ASSERT_FLOAT_WITHIN(EPS, 1.0f, gpAxisMath(4, 255, 0, 50, false));
	TEST_ASSERT_FLOAT_WITHIN(EPS, -1.0f, gpAxisMath(0, 0, 0, 50, false));
}

static void test_expo_compresses_midrange(void)
{
	// At raw=192 (n=0.5 linear), expo=50 → (1-0.5)*0.5 + 0.5*0.125 = 0.3125
	float n = gpAxisMath(0, 192, 0, 50, false);
	TEST_ASSERT_FLOAT_WITHIN(EPS, 0.3125f, n);
}

static void test_expo_zero_is_linear(void)
{
	// expo=0 → no curve applied
	float n = gpAxisMath(0, 192, 0, 0, false);
	TEST_ASSERT_FLOAT_WITHIN(EPS, 0.5f, n);
}

static void test_expo_clamped_to_one(void)
{
	// expo > 100 should clamp k to 1.0 → n = 1.0 * a^3 = 0.125 at raw=192
	float n = gpAxisMath(0, 192, 0, 200, false);
	TEST_ASSERT_FLOAT_WITHIN(EPS, 0.125f, n);
}

static void test_expo_negative_is_symmetric(void)
{
	float pos = gpAxisMath(0, 192, 0, 50, false);
	float neg = gpAxisMath(0, 64, 0, 50, false);
	TEST_ASSERT_FLOAT_WITHIN(EPS, pos, -neg);
}

// ── Combined: deadzone + expo + inversion ───────────────────────────────────
static void test_combined_dz_expo_inv(void)
{
	// raw=200, dz=6, expo=50, inv=true
	// v=200-128=72 → v=72-6=66 → n=66/(128-6)=66/122≈0.5410
	// expo: a=0.5410, n = 0.5*0.5410 + 0.5*0.5410^3 ≈ 0.2705 + 0.0792 ≈ 0.3497
	// inv → -0.3497
	float n = gpAxisMath(0, 200, 6, 50, true);
	float expectedLinear = 66.0f / 122.0f;
	float expectedExpo = 0.5f * expectedLinear + 0.5f * expectedLinear * expectedLinear * expectedLinear;
	TEST_ASSERT_FLOAT_WITHIN(EPS, -expectedExpo, n);
}

int main(int, char **)
{
	UNITY_BEGIN();
	RUN_TEST(test_stick_center_returns_zero);
	RUN_TEST(test_stick_inside_deadzone_returns_zero);
	RUN_TEST(test_stick_just_outside_deadzone_is_small);
	RUN_TEST(test_stick_max_positive_near_one);
	RUN_TEST(test_stick_min_negative_saturates_to_neg_one);
	RUN_TEST(test_stick_clamps_when_overdriven);
	RUN_TEST(test_stick_zero_dz_linear);
	RUN_TEST(test_stick_inversion_flips_sign);
	RUN_TEST(test_trigger_zero_returns_zero);
	RUN_TEST(test_trigger_max_returns_one);
	RUN_TEST(test_trigger_inside_deadzone_returns_zero);
	RUN_TEST(test_trigger_above_deadzone_normalized);
	RUN_TEST(test_expo_preserves_endpoints);
	RUN_TEST(test_expo_compresses_midrange);
	RUN_TEST(test_expo_zero_is_linear);
	RUN_TEST(test_expo_clamped_to_one);
	RUN_TEST(test_expo_negative_is_symmetric);
	RUN_TEST(test_combined_dz_expo_inv);
	return UNITY_END();
}
