/**
 * @file firmware/test/test_native_nag_math/test_nag_math.cpp
 * @brief Unit tests for nag suppression PRNG and Gaussian math helpers
 * @author Tesla CAN Mod Contributors
 * @license GPL-3.0
 */

#include <unity.h>
#include <cmath>
#include <cstring>

#define BUS_CHASSIS_ACTIVE 1
#define BUS_VEHICLE_ACTIVE 1
#define BUS_BODY_ACTIVE 1
#define BOARD_CAN_CLOCK_MHZ 8
#define BOARD_ENABLE_WIFI 0
#define BOARD_ENABLE_BLE 0

#include "core/types.h"
#include "feature/fsd/nag/math.h"

void setUp()
{
	_nagPrngState = 2463534242UL;
}
void tearDown() {}

/* ── _nagXorshift determinism ───────────────────────────────────────────── */

void test_xorshift_first_value_is_deterministic()
{
	uint32_t a = _nagXorshift();
	_nagPrngState = 2463534242UL;
	uint32_t b = _nagXorshift();
	TEST_ASSERT_EQUAL_UINT32(a, b);
}

void test_xorshift_sequence_is_reproducible()
{
	uint32_t seq1[10];
	for (int i = 0; i < 10; i++)
		seq1[i] = _nagXorshift();

	_nagPrngState = 2463534242UL;
	for (int i = 0; i < 10; i++)
	{
		uint32_t val = _nagXorshift();
		TEST_ASSERT_EQUAL_UINT32(seq1[i], val);
	}
}

void test_xorshift_produces_non_zero_values()
{
	uint32_t val = _nagXorshift();
	TEST_ASSERT_NOT_EQUAL(0, val);
}

void test_xorshift_values_vary()
{
	uint32_t a = _nagXorshift();
	uint32_t b = _nagXorshift();
	uint32_t c = _nagXorshift();
	TEST_ASSERT_TRUE(a != b || b != c);
}

/* ── _nagRandFloat ───────────────────────────────────────────────────────── */

void test_rand_float_in_range()
{
	for (int i = 0; i < 100; i++)
	{
		float v = _nagRandFloat();
		TEST_ASSERT_TRUE(v >= 0.0f);
		TEST_ASSERT_TRUE(v < 1.0f);
	}
}

void test_rand_float_not_constant()
{
	float a = _nagRandFloat();
	float b = _nagRandFloat();
	float c = _nagRandFloat();
	TEST_ASSERT_TRUE(a != b || b != c);
}

/* ── _nagGaussian ────────────────────────────────────────────────────────── */

void test_gaussian_mean_near_zero()
{
	float sum = 0.0f;
	const int N = 1000;
	for (int i = 0; i < N; i++)
		sum += _nagGaussian(1.0f);
	float mean = sum / N;
	TEST_ASSERT_FLOAT_WITHIN(0.3f, 0.0f, mean);
}

void test_gaussian_std_near_sigma()
{
	const float sigma = 2.0f;
	const int N = 1000;
	float sum = 0.0f;
	float sumSq = 0.0f;
	for (int i = 0; i < N; i++)
	{
		float v = _nagGaussian(sigma);
		sum += v;
		sumSq += v * v;
	}
	float mean = sum / N;
	float variance = (sumSq / N) - (mean * mean);
	float std = std::sqrt(variance);
	TEST_ASSERT_FLOAT_WITHIN(0.5f, sigma, std);
}

void test_gaussian_different_sigma_different_spread()
{
	const int N = 500;
	float sumSmall = 0.0f;
	float sumLarge = 0.0f;
	for (int i = 0; i < N; i++)
	{
		sumSmall += std::abs(_nagGaussian(0.5f));
		sumLarge += std::abs(_nagGaussian(3.0f));
	}
	TEST_ASSERT_TRUE(sumLarge > sumSmall);
}

int main()
{
	UNITY_BEGIN();

	RUN_TEST(test_xorshift_first_value_is_deterministic);
	RUN_TEST(test_xorshift_sequence_is_reproducible);
	RUN_TEST(test_xorshift_produces_non_zero_values);
	RUN_TEST(test_xorshift_values_vary);

	RUN_TEST(test_rand_float_in_range);
	RUN_TEST(test_rand_float_not_constant);

	RUN_TEST(test_gaussian_mean_near_zero);
	RUN_TEST(test_gaussian_std_near_sigma);
	RUN_TEST(test_gaussian_different_sigma_different_spread);

	return UNITY_END();
}
