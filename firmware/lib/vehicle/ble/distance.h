#pragma once

/**
 * @file firmware/lib/vehicle/ble/distance.h
 * @brief BLE key distance estimation (threshold / formula / Kalman)
 * @author Tesla CAN Mod Contributors
 * @license GPL-3.0
 */

#include <math.h>
#include <stdint.h>
#include <string.h>
#include "core/types.h"

/**
 * @brief Default BLE TX power at 1 m used for log-distance path loss
 *
 * This is a representative value for Tesla vehicle BLE advertising observed
 * at 1 m in free space. Calibration (`bleDistanceCalOffset`) shifts this.
 */
static constexpr float BLE_DISTANCE_DEFAULT_TX_POWER_DBM = -59.0f;

/**
 * @brief RSSI buckets used by threshold mode (meters returned: 1 / 5 / 15)
 */
static constexpr int BLE_DISTANCE_NEAR_RSSI_DBM = -55;
static constexpr int BLE_DISTANCE_MID_RSSI_DBM  = -75;

/**
 * @brief Default process and measurement noise for the simple Kalman filter
 */
static constexpr float BLE_KALMAN_Q = 0.05f;
static constexpr float BLE_KALMAN_R = 4.0f;

/**
 * @brief Return a human-readable name for a BleDistanceMode value
 * @param m Distance estimation mode
 * @return Null-terminated mode name string
 */
inline const char *bleDistanceModeName(BleDistanceMode m)
{
	switch (m)
	{
	case BLE_DISTANCE_OFF:
		return "off";
	case BLE_DISTANCE_THRESHOLD:
		return "threshold";
	case BLE_DISTANCE_FORMULA:
		return "formula";
	case BLE_DISTANCE_KALMAN:
		return "kalman";
	default:
		return "off";
	}
}

/**
 * @brief Parse a string into a BleDistanceMode enum value
 * @param name Input string ("off", "threshold", "formula", "kalman")
 * @param out Output mode on success
 * @return True if parsing succeeded
 */
inline bool parseBleDistanceMode(const char *name, BleDistanceMode &out)
{
	if (strcmp(name, "off") == 0)
	{
		out = BLE_DISTANCE_OFF;
		return true;
	}
	if (strcmp(name, "threshold") == 0)
	{
		out = BLE_DISTANCE_THRESHOLD;
		return true;
	}
	if (strcmp(name, "formula") == 0)
	{
		out = BLE_DISTANCE_FORMULA;
		return true;
	}
	if (strcmp(name, "kalman") == 0)
	{
		out = BLE_DISTANCE_KALMAN;
		return true;
	}
	return false;
}

/**
 * @brief 1-D Kalman filter state for smoothing RSSI
 */
struct BleKalman
{
	float x; ///< Estimated RSSI (dBm)
	float p; ///< Error covariance
	bool initialized;

	BleKalman() : x(0.0f), p(1.0f), initialized(false) {}

	/**
	 * @brief Reset filter state
	 */
	void reset()
	{
		x = 0.0f;
		p = 1.0f;
		initialized = false;
	}

	/**
	 * @brief Update estimate with a new RSSI measurement
	 * @param rssi Raw RSSI measurement (dBm)
	 * @return Smoothed RSSI estimate (dBm)
	 */
	float update(int rssi)
	{
		if (!initialized)
		{
			x = (float)rssi;
			p = 1.0f;
			initialized = true;
			return x;
		}

		// Prediction
		p += BLE_KALMAN_Q;

		// Update
		float k = p / (p + BLE_KALMAN_R);
		x += k * ((float)rssi - x);
		p *= (1.0f - k);
		return x;
	}
};

/**
 * @brief Estimate distance from raw RSSI using current state mode/factor/calibration.
 * @param rssi Raw RSSI in dBm
 * @param txPower Advertised TX power at 1 m (dBm)
 * @param s Global state (mode, factor, calibration; not mutated)
 * @return Estimated distance in meters, or -1.0 when disabled
 *
 * This is the stateless core used by the command handler; the full estimator
 * object adds Kalman smoothing and writes state fields.
 */
static inline float estimateBleDistance(int rssi, int txPower, const State &s)
{
	if (s.bleDistanceMode == BLE_DISTANCE_OFF)
		return -1.0f;

	if (s.bleDistanceMode == BLE_DISTANCE_THRESHOLD)
	{
		if (rssi > BLE_DISTANCE_NEAR_RSSI_DBM)
			return 1.0f;
		if (rssi > BLE_DISTANCE_MID_RSSI_DBM)
			return 5.0f;
		return 15.0f;
	}

	float pathLossExponent = s.bleDistanceFactor;
	if (pathLossExponent <= 0.0f)
		pathLossExponent = 2.0f;

	float calibratedTxPower = (float)txPower + s.bleDistanceCalOffset;
	float ratio = (calibratedTxPower - (float)rssi) / (10.0f * pathLossExponent);
	float meters = powf(10.0f, ratio);

	if (meters < 0.1f)
		meters = 0.1f;
	if (meters > 100.0f)
		meters = 100.0f;
	return meters;
}

/**
 * @brief Distance estimator using threshold buckets, log-distance formula, or Kalman-smoothed formula
 */
struct BleDistanceEstimator
{
	BleKalman kalman;

	/**
	 * @brief Compute distance from raw RSSI using the configured mode
	 * @param rssi Raw RSSI in dBm
	 * @param state Global state (mode, factor, calibration)
	 * @return Estimated distance in meters, or -1.0 when disabled
	 *
	 * The function also writes `state.bleDistanceMeters`, `state.bleRssi`,
	 * and updates internal Kalman state when mode is Kalman.
	 */
	float update(int rssi, State &state)
	{
		state.bleRssi = rssi;
		BleDistanceMode mode = state.bleDistanceMode;

		if (mode == BLE_DISTANCE_OFF)
		{
			state.bleDistanceMeters = -1.0f;
			kalman.reset();
			return state.bleDistanceMeters;
		}

		if (mode == BLE_DISTANCE_THRESHOLD)
		{
			if (rssi > BLE_DISTANCE_NEAR_RSSI_DBM)
				state.bleDistanceMeters = 1.0f;
			else if (rssi > BLE_DISTANCE_MID_RSSI_DBM)
				state.bleDistanceMeters = 5.0f;
			else
				state.bleDistanceMeters = 15.0f;
			return state.bleDistanceMeters;
		}

		int smoothedRssi = rssi;
		if (mode == BLE_DISTANCE_KALMAN)
			smoothedRssi = (int)roundf(kalman.update(rssi));

		float txPower = BLE_DISTANCE_DEFAULT_TX_POWER_DBM + state.bleDistanceCalOffset;
		float pathLossExponent = state.bleDistanceFactor;
		if (pathLossExponent <= 0.0f)
			pathLossExponent = 2.0f;

		float ratio = (txPower - smoothedRssi) / (10.0f * pathLossExponent);
		float meters = powf(10.0f, ratio);

		// Clamp to sane physical range
		if (meters < 0.1f)
			meters = 0.1f;
		if (meters > 100.0f)
			meters = 100.0f;

		state.bleDistanceMeters = meters;
		return meters;
	}

	/**
	 * @brief Reset internal filter state
	 */
	void reset()
	{
		kalman.reset();
	}
};
