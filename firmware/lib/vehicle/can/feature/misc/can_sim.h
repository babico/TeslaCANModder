#pragma once

/**
 * @file firmware/lib/vehicle/can/feature/can_sim.h
 * @brief CAN simulation mode for testing without a real vehicle
 * @author Tesla CAN Mod Contributors
 * @license GPL-3.0
 */

#include "core/types.h"
#include "vehicle/can/ids.h"

/**
 * @brief Forward declaration of the CAN frame dispatch handler
 * @param f CAN frame to process
 * @param bus Bus index the frame originated from
 * @param s Device state
 */
void handleMessage(Frame &f, uint8_t bus, State &s);

/**
 * @brief Generate synthetic CAN frames at a fixed interval for offline testing
 *
 * When simulation mode is enabled, this function injects test frames into the
 * decode pipeline every 200 ms. It never transmits on the physical bus — frames
 * are fed directly to handleMessage().
 *
 * Simulated signals: BMS voltage/current (0x132), BMS SoC (0x292),
 * TPMS (0x219 every 5th tick), vehicle speed (0x257), DI state (0x118).
 *
 * @param s Device state containing simulation enable flag and timing counters
 */
inline void canSimTick(State &s)
{
	if (!s.canSimEnabled)
		return;
	unsigned long now = millis();
	if (now - s.canSimLastMs < 200) // 200 ms interval between synthetic bursts
		return;
	s.canSimLastMs = now;
	s.canSimCounter++;

	Frame f;
	f.dlc = 8;
	memset(f.data, 0, 8);

	// Simulate BMS HV bus status (0x132): ~375V, ~5A
	f.id = CAN_ID_BMS_HV_BUS;
	f.data[0] = 0x10;
	f.data[1] = 0x68; // 0x1068 * 0.01 = 419.76V approx
	f.data[2] = 0x00;
	f.data[3] = 0x32; // 0x0032 * 0.1 = 5.0A
	handleMessage(f, BUS_VEHICLE, s);

	// Simulate BMS SoC (0x292): ~70%
	memset(f.data, 0, 8);
	f.id = CAN_ID_BMS_SOC;
	f.data[0] = 0x02;
	f.data[1] = 0xBC; // 0x02BC & 0x03FF = 700 -> 70.0%
	handleMessage(f, BUS_VEHICLE, s);

	// Simulate TPMS (0x219) every 5th tick: 2.50 bar, 25°C all tires
	if (s.canSimCounter % 5 == 0)
	{
		memset(f.data, 0, 8);
		f.id = CAN_ID_TPMS;
		f.data[0] = 250; // FL pressure: 250 = 2.50 bar
		f.data[1] = 250; // FR pressure
		f.data[2] = 250; // RL pressure
		f.data[3] = 250; // RR pressure
		f.data[4] = 25;  // FL temp °C
		f.data[5] = 25;  // FR temp °C
		f.data[6] = 25;  // RL temp °C
		f.data[7] = 25;  // RR temp °C
		handleMessage(f, BUS_VEHICLE, s);
	}

	// Simulate vehicle speed (0x257): oscillates 60-61 km/h
	memset(f.data, 0, 8);
	f.id = CAN_ID_VEHICLE_SPEED;
	uint16_t spd = (uint16_t)(6000 + (s.canSimCounter % 100) * 10); // raw speed value
	f.data[2] = (spd >> 8) & 0xFF; // big-endian speed in bytes 2-3
	f.data[3] = spd & 0xFF;
	handleMessage(f, BUS_VEHICLE, s);

	// Simulate DI state (0x118): gear=D, pedal=15%
	memset(f.data, 0, 8);
	f.id = CAN_ID_DI_STATE;
	f.data[0] = 0x08; // gear field: D = 4 shifted left by 1 bit
	f.data[1] = 15;   // accelerator pedal position: 15%
	handleMessage(f, BUS_VEHICLE, s);
}

/**
 * @brief Execute simulation start/stop commands (simu:start, simu:stop)
 * @param cmd Command string to match
 * @param s Device state; canSimEnabled, canSimCounter, canSimLastMs are updated
 * @return True if the command was recognized and executed
 */
static bool executeCanSimCmd(const char *cmd, State &s)
{
	if (strcmp(cmd, "simu:start") == 0)
	{
		s.canSimEnabled = true;
		s.canSimCounter = 0;
		s.canSimLastMs = 0;
		return true;
	}
	if (strcmp(cmd, "simu:stop") == 0)
	{
		s.canSimEnabled = false;
		return true;
	}
	return false;
}
