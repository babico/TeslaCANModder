#pragma once
#include "core/types.h"
#include "infra/can.h"

// ── CAN Simulation Mode ────────────────────────────────────────────────────
// Generates synthetic CAN frames for testing without a real vehicle.
// When enabled, periodically injects test data into the decode pipeline.
// This never transmits on the physical bus — it only feeds handleMessage().

// Forward declare handleMessage (defined in dispatch)
void handleMessage(Frame &f, uint8_t bus, State &s);

inline void canSimTick(State &s)
{
	if (!s.canSimEnabled)
		return;
	unsigned long now = millis();
	if (now - s.canSimLastMs < 200)
		return;
	s.canSimLastMs = now;
	s.canSimCounter++;

	Frame f;
	f.dlc = 8;
	memset(f.data, 0, 8);

	// Simulate BMS HV bus (0x132)
	f.id = CAN_ID_BMS_HV_BUS;
	f.data[0] = 0x10;
	f.data[1] = 0x68; // ~375V
	f.data[2] = 0x00;
	f.data[3] = 0x32; // ~5A
	handleMessage(f, BUS_VEHICLE, s);

	// Simulate BMS SoC (0x292)
	memset(f.data, 0, 8);
	f.id = CAN_ID_BMS_SOC;
	f.data[0] = 0x02;
	f.data[1] = 0xBC; // 70%
	handleMessage(f, BUS_VEHICLE, s);

	// Simulate TPMS (0x219) every 5th tick
	if (s.canSimCounter % 5 == 0)
	{
		memset(f.data, 0, 8);
		f.id = CAN_ID_TPMS;
		// ~2.5 bar all 4 tires (250 = 2.50 bar)
		f.data[0] = 250;
		f.data[1] = 250;
		f.data[2] = 250;
		f.data[3] = 250;
		// temps ~25°C
		f.data[4] = 25;
		f.data[5] = 25;
		f.data[6] = 25;
		f.data[7] = 25;
		handleMessage(f, BUS_VEHICLE, s);
	}

	// Simulate vehicle speed (0x257)
	memset(f.data, 0, 8);
	f.id = CAN_ID_VEHICLE_SPEED;
	uint16_t spd = (uint16_t)(6000 + (s.canSimCounter % 100) * 10); // 60-61 km/h
	f.data[2] = (spd >> 8) & 0xFF;
	f.data[3] = spd & 0xFF;
	handleMessage(f, BUS_VEHICLE, s);

	// Simulate DI state (0x118) gear=D, pedal=15%
	memset(f.data, 0, 8);
	f.id = CAN_ID_DI_STATE;
	f.data[0] = 0x08; // gear = D (4 << 1)
	f.data[1] = 15;	  // 15% pedal
	handleMessage(f, BUS_VEHICLE, s);
}

inline bool execCanSimCmd(const char *cmd, State &s)
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
