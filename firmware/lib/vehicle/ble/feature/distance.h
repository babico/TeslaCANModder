#pragma once

/**
 * @file firmware/lib/vehicle/ble/feature/distance.h
 * @brief BLE key distance estimator command handlers
 * @author Tesla CAN Mod Contributors
 * @license GPL-3.0
 */

#include "core/forward.h"
#include "core/util/parse.h"
#include "vehicle/ble/distance.h"

/**
 * @brief Execute a `blekey:distance:*` command.
 * @param cmd Full command string.
 * @param s Global state (mutated with new mode/factor/calibration).
 * @return True if the command was recognized and executed.
 *
 * Supported commands:
 *   blekey:distance              — query (no state change)
 *   blekey:distance:off          — disable distance estimation
 *   blekey:distance:threshold    — near/mid/far threshold mode
 *   blekey:distance:formula      — log-distance path-loss formula (default)
 *   blekey:distance:kalman       — formula + 1D Kalman RSSI smoothing
 *   blekey:distance:factor:<N>   — set path-loss exponent (1.0..4.0)
 *   blekey:distance:calibrate:<M>— calibrate current reading to M meters
 */
static bool executeBleKeyDistanceCmd(const char *cmd, State &s)
{
	if (strcmp(cmd, "blekey:distance") == 0)
	{
		return true;
	}

	if (strcmp(cmd, "blekey:distance:off") == 0)
	{
		s.bleDistanceMode = BLE_DISTANCE_OFF;
		s.bleDistanceMeters = -1.0f;
		saveSettings(s);
		return true;
	}

	BleDistanceMode parsed = BLE_DISTANCE_OFF;
	if (parseBleDistanceMode(cmd + strlen("blekey:distance:"), parsed))
	{
		s.bleDistanceMode = parsed;
		if (parsed == BLE_DISTANCE_OFF)
			s.bleDistanceMeters = -1.0f;
		saveSettings(s);
		return true;
	}

	if (strncmp(cmd, "blekey:distance:factor:", 23) == 0)
	{
		float f = (float)atof(cmd + 23);
		if (f >= 1.0f && f <= 4.0f)
		{
			s.bleDistanceFactor = f;
			saveSettings(s);
			return true;
		}
		return false;
	}

	if (strncmp(cmd, "blekey:distance:calibrate:", 26) == 0)
	{
		float actualMeters = (float)atof(cmd + 26);
		if (actualMeters < 0.1f || actualMeters > 50.0f)
			return false;

		// Compute what the current raw formula would yield, then store an
		// offset that shifts TX power so the current RSSI maps to actualMeters.
		float currentMeters = estimateBleDistance(s.bleRssi, (int)BLE_DISTANCE_DEFAULT_TX_POWER_DBM, s);
		if (currentMeters <= 0.0f)
			currentMeters = 0.1f;

		float ratioActual = log10f(actualMeters);
		float ratioCurrent = log10f(currentMeters);
		s.bleDistanceCalOffset = 10.0f * s.bleDistanceFactor * (ratioActual - ratioCurrent);
		s.bleDistanceCalibrated = true;
		saveSettings(s);
		return true;
	}

	return false;
}

