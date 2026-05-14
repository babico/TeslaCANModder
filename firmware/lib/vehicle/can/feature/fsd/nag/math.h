#pragma once

/**
 * @file firmware/lib/vehicle/can/feature/fsd/nag/math.h
 * @brief PRNG and Gaussian noise helpers for nag suppression strategies
 * @author Tesla CAN Mod Contributors
 * @license GPL-3.0
 */

#include <stdint.h>
#include <cmath>

/**
 * @brief Deterministic xorshift32 PRNG state shared across all nag strategies.
 */
static uint32_t _nagPrngState = 2463534242UL;

/**
 * @brief Advance the xorshift32 PRNG and return the next pseudo-random value.
 * @return 32-bit pseudo-random number.
 */
inline uint32_t _nagXorshift()
{
	_nagPrngState ^= _nagPrngState << 13;
	_nagPrngState ^= _nagPrngState >> 17;
	_nagPrngState ^= _nagPrngState << 5;
	return _nagPrngState;
}

/**
 * @brief Generate a uniform random float in [0, 1) from the xorshift PRNG.
 * @return Float in the range [0.0, 1.0).
 */
inline float _nagRandFloat()
{
	return (_nagXorshift() & 0xFFFFFF) / 16777216.0f;
}

/**
 * @brief Generate a Gaussian-distributed random value using Box-Muller transform.
 * @param sigma Standard deviation of the distribution.
 * @return Random sample from N(0, sigma).
 */
inline float _nagGaussian(float sigma)
{
	float u1 = _nagRandFloat();
	float u2 = _nagRandFloat();
	if (u1 < 1e-7f)
		u1 = 1e-7f;
	float z = std::sqrt(-2.0f * std::log(u1)) * std::cos(6.2831853f * u2);
	return z * sigma;
}
