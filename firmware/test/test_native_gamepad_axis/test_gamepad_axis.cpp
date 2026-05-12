/**
 * @file firmware/test/test_native_gamepad_axis/test_gamepad_axis.cpp
 * @brief Unit tests for gamepad axis mapping and deadzone
 * @author Tesla CAN Mod Contributors
 * @license GPL-3.0
 */

#include <unity.h>
#include <cmath>
#include <cstdint>

#include "client/gamepad/axis_math.h"

static constexpr float EPS = 1e-3f;

/** @brief Test fixture setup — no per-test state required */
void setUp(void) {}

/** @brief Test fixture teardown — no cleanup required */
void tearDown(void) {}

/** @brief Verifies stick at center position (128) returns zero output */
static void test_stick_center_returns_zero(void)
{
	TEST_ASSERT_FLOAT_WITHIN(EPS, 0.0f, gpAxisMath(0, 128, 6, 0, false));
}

/** @brief Verifies stick values within the deadzone band return zero */
static void test_stick_inside_deadzone_returns_zero(void)
{
	TEST_ASSERT_FLOAT_WITHIN(EPS, 0.0f, gpAxisMath(0, 130, 6, 0, false));
	TEST_ASSERT_FLOAT_WITHIN(EPS, 0.0f, gpAxisMath(0, 125, 6, 0, false));
}

/** @brief Verifies stick just outside deadzone produces a small positive value */
static void test_stick_just_outside_deadzone_is_small(void)
{
	float n = gpAxisMath(0, 135, 6, 0, false);
	TEST_ASSERT_TRUE(n > 0.0f);
	TEST_ASSERT_FLOAT_WITHIN(0.01f, 1.0f / 122.0f, n);
}

/** @brief Verifies stick at maximum positive (255) approaches but does not exceed 1.0 */
static void test_stick_max_positive_near_one(void)
{
	float n = gpAxisMath(0, 255, 6, 0, false);
	TEST_ASSERT_FLOAT_WITHIN(0.01f, 121.0f / 122.0f, n);
	TEST_ASSERT_TRUE(n <= 1.0f);
}

/** @brief Verifies stick at minimum (0) saturates to -1.0 */
static void test_stick_min_negative_saturates_to_neg_one(void)
{
	TEST_ASSERT_FLOAT_WITHIN(EPS, -1.0f, gpAxisMath(0, 0, 6, 0, false));
}

/** @brief Verifies overdriven stick input clamps to -1.0 */
static void test_stick_clamps_when_overdriven(void)
{
	TEST_ASSERT_FLOAT_WITHIN(EPS, -1.0f, gpAxisMath(0, 0, 0, 0, false));
}

/** @brief Verifies zero deadzone produces linear mapping */
static void test_stick_zero_dz_linear(void)
{
	TEST_ASSERT_FLOAT_WITHIN(EPS, 0.5f, gpAxisMath(1, 192, 0, 0, false));
}

/** @brief Verifies inversion flag flips the output sign */
static void test_stick_inversion_flips_sign(void)
{
	float pos = gpAxisMath(0, 200, 0, 0, false);
	float inv = gpAxisMath(0, 200, 0, 0, true);
	TEST_ASSERT_FLOAT_WITHIN(EPS, pos, -inv);
	TEST_ASSERT_TRUE(pos > 0.0f);
}

/** @brief Verifies trigger axis at zero raw value returns zero output */
static void test_trigger_zero_returns_zero(void)
{
	TEST_ASSERT_FLOAT_WITHIN(EPS, 0.0f, gpAxisMath(4, 0, 8, 0, false));
	TEST_ASSERT_FLOAT_WITHIN(EPS, 0.0f, gpAxisMath(5, 0, 8, 0, false));
}

/** @brief Verifies trigger axis at maximum raw value returns 1.0 */
static void test_trigger_max_returns_one(void)
{
	TEST_ASSERT_FLOAT_WITHIN(EPS, 1.0f, gpAxisMath(4, 255, 8, 0, false));
	TEST_ASSERT_FLOAT_WITHIN(EPS, 1.0f, gpAxisMath(5, 255, 8, 0, false));
}

/** @brief Verifies trigger values inside deadzone return zero */
static void test_trigger_inside_deadzone_returns_zero(void)
{
	TEST_ASSERT_FLOAT_WITHIN(EPS, 0.0f, gpAxisMath(4, 5, 8, 0, false));
}

/** @brief Verifies trigger above deadzone is normalized to 0..1 range */
static void test_trigger_above_deadzone_normalized(void)
{
	float n = gpAxisMath(4, 131, 8, 0, false);
	TEST_ASSERT_FLOAT_WITHIN(0.01f, 123.0f / 247.0f, n);
}

/** @brief Verifies expo curve preserves endpoint values (0 and ±1) */
static void test_expo_preserves_endpoints(void)
{
	TEST_ASSERT_FLOAT_WITHIN(EPS, 1.0f, gpAxisMath(4, 255, 0, 50, false));
	TEST_ASSERT_FLOAT_WITHIN(EPS, -1.0f, gpAxisMath(0, 0, 0, 50, false));
}

/** @brief Verifies expo curve compresses midrange values toward zero */
static void test_expo_compresses_midrange(void)
{
	float n = gpAxisMath(0, 192, 0, 50, false);
	TEST_ASSERT_FLOAT_WITHIN(EPS, 0.3125f, n);
}

/** @brief Verifies expo=0 produces linear output */
static void test_expo_zero_is_linear(void)
{
	float n = gpAxisMath(0, 192, 0, 0, false);
	TEST_ASSERT_FLOAT_WITHIN(EPS, 0.5f, n);
}

/** @brief Verifies expo parameter is clamped to valid range (max 100) */
static void test_expo_clamped_to_one(void)
{
	float n = gpAxisMath(0, 192, 0, 200, false);
	TEST_ASSERT_FLOAT_WITHIN(EPS, 0.125f, n);
}

/** @brief Verifies expo curve is symmetric for positive and negative inputs */
static void test_expo_negative_is_symmetric(void)
{
	float pos = gpAxisMath(0, 192, 0, 50, false);
	float neg = gpAxisMath(0, 64, 0, 50, false);
	TEST_ASSERT_FLOAT_WITHIN(EPS, pos, -neg);
}

/** @brief Verifies combined deadzone + expo + inversion produces expected result */
static void test_combined_dz_expo_inv(void)
{
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
