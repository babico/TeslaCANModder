#pragma once
#include "core/forward.h"
#include "infra/can.h"

// ── BMS Battery Telemetry Decoder ────────────────────────────────────────────
// Decodes BMS CAN frames into human-readable battery telemetry.
// Sources: tuncasoftbildik, hypery11-flipper, J0811 legacy repos,
//          TESLA_CAN_BATTERY_REFERENCE.md (community DBC).

// CAN 0x132 (306) — BMS_hvBusStatus: Pack voltage & current
inline float decodeBmsVoltage(const uint8_t *data)
{
	uint16_t raw = ((uint16_t)data[0] << 8) | data[1];
	return raw * 0.01f; // Volts
}

inline float decodeBmsCurrent(const uint8_t *data)
{
	int16_t raw = (int16_t)(((uint16_t)data[2] << 8) | data[3]);
	return raw * 0.1f; // Amps (negative = discharging)
}

inline float decodeBmsPower(const uint8_t *data)
{
	return decodeBmsVoltage(data) * decodeBmsCurrent(data) * 0.001f; // kW
}

// CAN 0x292 (658) — BMS_socStatus: State of Charge
inline float decodeBmsSoc(const uint8_t *data)
{
	uint16_t raw = ((uint16_t)(data[0] & 0x03) << 8) | data[1];
	return raw * 0.1f; // Percent (0.0 - 100.0)
}

// CAN 0x312 (786) — BMS_thermalStatus: Battery temperatures
inline int8_t decodeBmsTempMin(const uint8_t *data)
{
	return (int8_t)(data[0]) - 40; // Celsius
}

inline int8_t decodeBmsTempMax(const uint8_t *data)
{
	return (int8_t)(data[1]) - 40; // Celsius
}

// CAN 0x33A (826) — UI_energyGraphData: Energy consumption
inline float decodeBmsWhPerKm(const uint8_t *data)
{
	uint16_t raw = ((uint16_t)data[0] << 8) | data[1];
	return raw * 0.1f; // Wh/km
}

// ── Enhanced BMS Decoders (Vehicle CAN) ──────────────────────────────────────

// CAN 0x352 (850) — BMS_energyStatus (mux=0): Degradation / Capacity
inline float decodeBmsNominalFullPack(const uint8_t *data)
{
	uint16_t raw = ((uint16_t)data[2] << 8) | data[3];
	return raw * 0.02f; // kWh — current full capacity
}

inline float decodeBmsNominalRemaining(const uint8_t *data)
{
	uint16_t raw = ((uint16_t)data[4] << 8) | data[5];
	return raw * 0.02f; // kWh — energy remaining
}

inline float decodeBmsIdealRemaining(const uint8_t *data)
{
	uint16_t raw = ((uint16_t)data[6] << 8) | data[7];
	return raw * 0.02f; // kWh — ideal energy remaining
}

// CAN 0x332 (818) — BMS_bmbMinMax (mux=1): Cell voltage min/max
inline float decodeBmsCellVoltageMax(const uint8_t *data)
{
	uint16_t raw = ((uint16_t)(data[0] & 0x3F) << 6) | (data[1] >> 2);
	return raw * 0.002f; // Volts
}

inline float decodeBmsCellVoltageMin(const uint8_t *data)
{
	uint16_t raw = ((uint16_t)data[2] << 4) | (data[3] >> 4);
	return raw * 0.002f; // Volts
}

// CAN 0x252 (594) — BMS_powerAvailable: Power limits
inline float decodeBmsMaxRegenPower(const uint8_t *data)
{
	uint16_t raw = ((uint16_t)data[0] << 8) | data[1];
	return raw * 0.01f; // kW
}

inline float decodeBmsMaxDischargePower(const uint8_t *data)
{
	uint16_t raw = ((uint16_t)data[2] << 8) | data[3];
	return raw * 0.01f; // kW
}

inline float decodeBmsStationaryHeatPower(const uint8_t *data)
{
	uint16_t raw = ((uint16_t)(data[4] & 0x03) << 8) | data[5];
	return raw * 0.01f; // kW
}

inline float decodeBmsHvacPowerBudget(const uint8_t *data)
{
	uint16_t raw = ((uint16_t)(data[6] & 0x03) << 8) | data[7];
	return raw * 0.02f; // kW
}

// ── Expanded 0x292 Decoders ──────────────────────────────────────────────────

// CAN 0x292 — BMS_socUI: SoC shown to user (bits 10-19)
inline float decodeBmsSocUI(const uint8_t *data)
{
	uint16_t raw = ((uint16_t)(data[1] & 0x3F) << 4) | (data[2] >> 4);
	return raw * 0.1f; // Percent
}

// CAN 0x292 — BMS_socMax (bits 20-29)
inline float decodeBmsSocMax(const uint8_t *data)
{
	uint16_t raw = ((uint16_t)(data[2] & 0x0F) << 6) | (data[3] >> 2);
	return raw * 0.1f; // Percent
}

// CAN 0x292 — BMS_socAvg (bits 30-39)
inline float decodeBmsSocAvg(const uint8_t *data)
{
	uint16_t raw = ((uint16_t)(data[3] & 0x03) << 8) | data[4];
	return raw * 0.1f; // Percent
}

// CAN 0x292 — BMS_initialFullPackEnergy (bits 40-49)
inline float decodeBmsInitialFullPack(const uint8_t *data)
{
	uint16_t raw = ((uint16_t)data[5] << 2) | (data[6] >> 6);
	return raw * 0.1f; // kWh
}

// ── Expanded 0x33A Decoders ──────────────────────────────────────────────────

// CAN 0x33A — UI_expectedRange (bits 0-9)
inline float decodeBmsExpectedRange(const uint8_t *data)
{
	uint16_t raw = ((uint16_t)data[0] << 2) | (data[1] >> 6);
	return raw * 1.6f; // km
}

// CAN 0x33A — UI_idealRange (bits 16-25)
inline float decodeBmsIdealRange(const uint8_t *data)
{
	uint16_t raw = ((uint16_t)data[2] << 2) | (data[3] >> 6);
	return raw * 1.6f; // km
}

// CAN 0x33A — UI_ratedConsumption (bits 32-41)
inline float decodeBmsRatedConsumption(const uint8_t *data)
{
	uint16_t raw = ((uint16_t)data[4] << 2) | (data[5] >> 6);
	return raw * 0.625f; // Wh/km
}

// CAN 0x33A — UI_actualSOC (bits 48-54)
inline uint8_t decodeBmsActualSocInt(const uint8_t *data)
{
	return data[6] & 0x7F; // integer %
}

// CAN 0x33A — UI_usableSOC (bits 56-62)
inline uint8_t decodeBmsUsableSocInt(const uint8_t *data)
{
	return data[7] & 0x7F; // integer %
}

// ── Expanded 0x312 Decoders ──────────────────────────────────────────────────

// CAN 0x312 — BMS_powerDissipation (bits 0-9): thermal power
inline float decodeBmsPowerDissipation(const uint8_t *data)
{
	uint16_t raw = ((uint16_t)data[0] << 2) | (data[1] >> 6);
	return raw * 0.02f; // kW
}

// CAN 0x312 — BMS_flowRequest (bits 10-16): coolant flow
inline float decodeBmsFlowRequest(const uint8_t *data)
{
	uint8_t raw = ((data[1] & 0x3F) << 1) | (data[2] >> 7);
	return raw * 0.3f; // LPM
}

// CAN 0x312 — BMS_inletActiveCoolTargetT (bits 17-25)
inline float decodeBmsCoolTarget(const uint8_t *data)
{
	uint16_t raw = ((uint16_t)(data[2] & 0x7F) << 2) | (data[3] >> 6);
	return raw * 0.25f - 25.0f; // °C
}

// CAN 0x312 — BMS_inletPassiveTargetT (bits 26-34)
inline float decodeBmsPassiveTarget(const uint8_t *data)
{
	uint16_t raw = ((uint16_t)(data[3] & 0x3F) << 3) | (data[4] >> 5);
	return raw * 0.25f - 25.0f; // °C
}

// CAN 0x312 — BMS_inletActiveHeatTargetT (bits 35-43)
inline float decodeBmsHeatTarget(const uint8_t *data)
{
	uint16_t raw = ((uint16_t)(data[4] & 0x1F) << 4) | (data[5] >> 4);
	return raw * 0.25f - 25.0f; // °C
}

// CAN 0x312 — BMS_packTMin (bits 44-52)
inline float decodeBmsPackTMin(const uint8_t *data)
{
	uint16_t raw = ((uint16_t)(data[5] & 0x0F) << 5) | (data[6] >> 3);
	return raw * 0.25f - 25.0f; // °C
}

// CAN 0x312 — BMS_packTMax (bits 53-61)
inline float decodeBmsPackTMax(const uint8_t *data)
{
	uint16_t raw = ((uint16_t)(data[6] & 0x07) << 6) | (data[7] >> 2);
	return raw * 0.25f - 25.0f; // °C
}

// ── 0x212 BMS_status Decoders ────────────────────────────────────────────────

// CAN 0x212 — BMS_preconditionAllowed (bit 3)
inline bool decodeBmsPrecondAllowed(const uint8_t *data)
{
	return (data[0] >> 3) & 0x01;
}

// CAN 0x212 — BMS_activeHeatingWorthwhile (bit 5)
inline bool decodeBmsHeatingWorthwhile(const uint8_t *data)
{
	return (data[0] >> 5) & 0x01;
}

// CAN 0x212 — BMS_contactorState (bits 8-10)
inline uint8_t decodeBmsContactorState(const uint8_t *data)
{
	return data[1] & 0x07;
}

// CAN 0x212 — BMS_hvState (bits 16-18)
inline uint8_t decodeBmsHvState(const uint8_t *data)
{
	return data[2] & 0x07;
}

// CAN 0x212 — BMS_minPackTemperature (bits 56-63)
inline float decodeBmsStatusMinTemp(const uint8_t *data)
{
	return (float)data[7] * 0.5f - 40.0f; // °C
}

// ── 0x2D2 BMS_driveLimits Decoders ──────────────────────────────────────────

inline float decodeBmsMinBusVoltage(const uint8_t *data)
{
	uint16_t raw = ((uint16_t)data[0] << 8) | data[1];
	return raw * 0.01f; // V
}

inline float decodeBmsMaxBusVoltage(const uint8_t *data)
{
	uint16_t raw = ((uint16_t)data[2] << 8) | data[3];
	return raw * 0.01f; // V
}

inline float decodeBmsMaxChargeCurrent(const uint8_t *data)
{
	uint16_t raw = ((uint16_t)data[4] << 6) | (data[5] >> 2);
	return raw * 0.1f; // A
}

inline float decodeBmsMaxDischargeCurrent(const uint8_t *data)
{
	uint16_t raw = ((uint16_t)data[6] << 6) | (data[7] >> 2);
	return raw * 0.128f; // A
}

// ── 0x352 mux=1 Decoders ────────────────────────────────────────────────────

inline float decodeBmsExpectedRemaining(const uint8_t *data)
{
	uint16_t raw = ((uint16_t)data[4] << 8) | data[5];
	return raw * 0.02f; // kWh
}

inline float decodeBmsEnergyBuffer(const uint8_t *data)
{
	uint16_t raw = ((uint16_t)data[2] << 8) | data[3];
	return raw * 0.01f; // kWh
}

inline float decodeBmsEnergyToChargeComplete(const uint8_t *data)
{
	uint16_t raw = ((uint16_t)data[6] << 8) | data[7];
	return raw * 0.02f; // kWh
}

inline bool decodeBmsFullyCharged(const uint8_t *data)
{
	return (data[1] >> 7) & 0x01; // bit 15
}

// ── 0x3D2 Lifetime Counters ─────────────────────────────────────────────────

inline float decodeBmsKwhDischargeTotal(const uint8_t *data)
{
	uint32_t raw = ((uint32_t)data[0] << 24) | ((uint32_t)data[1] << 16) | ((uint32_t)data[2] << 8) | data[3];
	return raw * 0.001f; // kWh
}

inline float decodeBmsKwhChargeTotal(const uint8_t *data)
{
	uint32_t raw = ((uint32_t)data[4] << 24) | ((uint32_t)data[5] << 16) | ((uint32_t)data[6] << 8) | data[7];
	return raw * 0.001f; // kWh
}

// ── 0x3F2 Multiplexed Counters ──────────────────────────────────────────────
// Byte 0 = mux index, bytes 1-4 = 32-bit counter
inline float decodeBmsKwhMuxCounter(const uint8_t *data)
{
	uint32_t raw = ((uint32_t)data[1] << 24) | ((uint32_t)data[2] << 16) | ((uint32_t)data[3] << 8) | data[4];
	return raw * 0.001f; // kWh
}

// ── 0x332 mux=0: Thermistor temps ───────────────────────────────────────────

inline float decodeBmsThermistorTMax(const uint8_t *data)
{
	return data[2] * 0.5f - 40.0f; // °C (bits 16-23)
}

inline float decodeBmsThermistorTMin(const uint8_t *data)
{
	return data[3] * 0.5f - 40.0f; // °C (bits 24-31)
}

inline float decodeBmsModelTMax(const uint8_t *data)
{
	return data[4] * 0.5f - 40.0f; // °C (bits 32-39)
}

inline float decodeBmsModelTMin(const uint8_t *data)
{
	return data[5] * 0.5f - 40.0f; // °C (bits 40-47)
}

// ── 0x132 Expanded Decoders ─────────────────────────────────────────────────

// BMS_chgTimeToFull (bits 48-59)
inline float decodeBmsChargeTimeToFull(const uint8_t *data)
{
	uint16_t raw = ((uint16_t)data[6] << 4) | (data[7] >> 4);
	return raw * 0.01667f; // hours
}

// ── BMS Telemetry Command ────────────────────────────────────────────────────
// Returns current BMS battery data as JSON.

bool execBmsCmd(const char *cmd, State &s)
{
	if (strcmp(cmd, "bms") != 0)
		return false;
	(void)s; // data is output as JSON by the serial layer
	return true;
}
