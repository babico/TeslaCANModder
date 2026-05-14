#pragma once

/**
 * @file firmware/lib/vehicle/can/feature/bms.h
 * @brief Decodes BMS CAN frames into human-readable battery telemetry
 * @author Tesla CAN Mod Contributors
 * @license GPL-3.0
 */

#include "core/forward.h"
#include "vehicle/can/ids.h"

/**
 * @brief Decode pack voltage from BMS HV bus status frame
 * @param data Raw CAN payload from frame 0x132 (306)
 * @return Pack voltage in Volts
 */
inline float decodeBmsVoltage(const uint8_t *data)
{
	uint16_t raw = ((uint16_t)data[0] << 8) | data[1]; // bytes 0-1: big-endian voltage
	return raw * 0.01f; // Volts
}

/**
 * @brief Decode pack current from BMS HV bus status frame
 * @param data Raw CAN payload from frame 0x132 (306)
 * @return Pack current in Amps (negative = discharging)
 */
inline float decodeBmsCurrent(const uint8_t *data)
{
	int16_t raw = (int16_t)(((uint16_t)data[2] << 8) | data[3]); // bytes 2-3: signed big-endian current
	return raw * 0.1f; // Amps (negative = discharging)
}

/**
 * @brief Compute instantaneous pack power from voltage and current
 * @param data Raw CAN payload from frame 0x132 (306)
 * @return Pack power in kilowatts
 */
inline float decodeBmsPower(const uint8_t *data)
{
	return decodeBmsVoltage(data) * decodeBmsCurrent(data) * 0.001f; // kW
}

/**
 * @brief Decode battery state of charge from BMS SoC status frame
 * @param data Raw CAN payload from frame 0x292 (658)
 * @return State of charge as percentage (0.0 - 100.0)
 */
inline float decodeBmsSoc(const uint8_t *data)
{
	uint16_t raw = ((uint16_t)(data[0] & 0x03) << 8) | data[1]; // bits 0-9: 10-bit SoC
	return raw * 0.1f; // Percent (0.0 - 100.0)
}

/**
 * @brief Decode minimum battery temperature from thermal status frame
 * @param data Raw CAN payload from frame 0x312 (786)
 * @return Minimum pack temperature in Celsius
 */
inline int8_t decodeBmsTempMin(const uint8_t *data)
{
	return (int8_t)(data[0]) - 40; // offset-40 encoding to Celsius
}

/**
 * @brief Decode maximum battery temperature from thermal status frame
 * @param data Raw CAN payload from frame 0x312 (786)
 * @return Maximum pack temperature in Celsius
 */
inline int8_t decodeBmsTempMax(const uint8_t *data)
{
	return (int8_t)(data[1]) - 40; // offset-40 encoding to Celsius
}

/**
 * @brief Decode energy consumption rate from UI energy graph frame
 * @param data Raw CAN payload from frame 0x33A (826)
 * @return Energy consumption in Wh/km
 */
inline float decodeBmsWhPerKm(const uint8_t *data)
{
	uint16_t raw = ((uint16_t)data[0] << 8) | data[1]; // bytes 0-1: big-endian consumption
	return raw * 0.1f; // Wh/km
}

/**
 * @brief Decode nominal full pack capacity from BMS energy status frame (mux=0)
 * @param data Raw CAN payload from frame 0x352 (850)
 * @return Current full pack capacity in kWh
 */
inline float decodeBmsNominalFullPack(const uint8_t *data)
{
	uint16_t raw = ((uint16_t)data[2] << 8) | data[3]; // bytes 2-3: full capacity
	return raw * 0.02f; // kWh
}

/**
 * @brief Decode nominal remaining energy from BMS energy status frame (mux=0)
 * @param data Raw CAN payload from frame 0x352 (850)
 * @return Remaining energy in kWh
 */
inline float decodeBmsNominalRemaining(const uint8_t *data)
{
	uint16_t raw = ((uint16_t)data[4] << 8) | data[5]; // bytes 4-5: remaining energy
	return raw * 0.02f; // kWh
}

/**
 * @brief Decode ideal remaining energy from BMS energy status frame (mux=0)
 * @param data Raw CAN payload from frame 0x352 (850)
 * @return Ideal remaining energy in kWh
 */
inline float decodeBmsIdealRemaining(const uint8_t *data)
{
	uint16_t raw = ((uint16_t)data[6] << 8) | data[7]; // bytes 6-7: ideal remaining
	return raw * 0.02f; // kWh
}

/**
 * @brief Decode maximum cell voltage from BMS cell min/max frame (mux=1)
 * @param data Raw CAN payload from frame 0x332 (818)
 * @return Maximum cell voltage in Volts
 */
inline float decodeBmsCellVoltageMax(const uint8_t *data)
{
	uint16_t raw = ((uint16_t)(data[0] & 0x3F) << 6) | (data[1] >> 2); // bits 2-13: 12-bit cell max
	return raw * 0.002f; // Volts
}

/**
 * @brief Decode minimum cell voltage from BMS cell min/max frame (mux=1)
 * @param data Raw CAN payload from frame 0x332 (818)
 * @return Minimum cell voltage in Volts
 */
inline float decodeBmsCellVoltageMin(const uint8_t *data)
{
	uint16_t raw = ((uint16_t)data[2] << 4) | (data[3] >> 4); // bits 16-27: 12-bit cell min
	return raw * 0.002f; // Volts
}

/**
 * @brief Decode maximum regen power limit from BMS power available frame
 * @param data Raw CAN payload from frame 0x252 (594)
 * @return Maximum regen power in kW
 */
inline float decodeBmsMaxRegenPower(const uint8_t *data)
{
	uint16_t raw = ((uint16_t)data[0] << 8) | data[1]; // bytes 0-1: regen limit
	return raw * 0.01f; // kW
}

/**
 * @brief Decode maximum discharge power limit from BMS power available frame
 * @param data Raw CAN payload from frame 0x252 (594)
 * @return Maximum discharge power in kW
 */
inline float decodeBmsMaxDischargePower(const uint8_t *data)
{
	uint16_t raw = ((uint16_t)data[2] << 8) | data[3]; // bytes 2-3: discharge limit
	return raw * 0.01f; // kW
}

/**
 * @brief Decode stationary heat power budget from BMS power available frame
 * @param data Raw CAN payload from frame 0x252 (594)
 * @return Stationary heating power in kW
 */
inline float decodeBmsStationaryHeatPower(const uint8_t *data)
{
	uint16_t raw = ((uint16_t)(data[4] & 0x03) << 8) | data[5]; // bits 38-47: 10-bit heat power
	return raw * 0.01f; // kW
}

/**
 * @brief Decode HVAC power budget from BMS power available frame
 * @param data Raw CAN payload from frame 0x252 (594)
 * @return HVAC power budget in kW
 */
inline float decodeBmsHvacPowerBudget(const uint8_t *data)
{
	uint16_t raw = ((uint16_t)(data[6] & 0x03) << 8) | data[7]; // bits 54-63: 10-bit HVAC budget
	return raw * 0.02f; // kW
}

/**
 * @brief Decode user-facing SoC from BMS SoC frame (bits 10-19)
 * @param data Raw CAN payload from frame 0x292 (658)
 * @return UI state of charge as percentage
 */
inline float decodeBmsSocUI(const uint8_t *data)
{
	uint16_t raw = ((uint16_t)(data[1] & 0x3F) << 4) | (data[2] >> 4); // bits 10-19
	return raw * 0.1f; // Percent
}

/**
 * @brief Decode maximum SoC limit from BMS SoC frame (bits 20-29)
 * @param data Raw CAN payload from frame 0x292 (658)
 * @return Maximum SoC limit as percentage
 */
inline float decodeBmsSocMax(const uint8_t *data)
{
	uint16_t raw = ((uint16_t)(data[2] & 0x0F) << 6) | (data[3] >> 2); // bits 20-29
	return raw * 0.1f; // Percent
}

/**
 * @brief Decode average SoC from BMS SoC frame (bits 30-39)
 * @param data Raw CAN payload from frame 0x292 (658)
 * @return Average state of charge as percentage
 */
inline float decodeBmsSocAvg(const uint8_t *data)
{
	uint16_t raw = ((uint16_t)(data[3] & 0x03) << 8) | data[4]; // bits 30-39
	return raw * 0.1f; // Percent
}

/**
 * @brief Decode initial full pack energy from BMS SoC frame (bits 40-49)
 * @param data Raw CAN payload from frame 0x292 (658)
 * @return Initial full pack energy in kWh
 */
inline float decodeBmsInitialFullPack(const uint8_t *data)
{
	uint16_t raw = ((uint16_t)data[5] << 2) | (data[6] >> 6); // bits 40-49
	return raw * 0.1f; // kWh
}

/**
 * @brief Decode expected range from UI energy graph frame (bits 0-9)
 * @param data Raw CAN payload from frame 0x33A (826)
 * @return Expected range in km
 */
inline float decodeBmsExpectedRange(const uint8_t *data)
{
	uint16_t raw = ((uint16_t)data[0] << 2) | (data[1] >> 6); // bits 0-9
	return raw * 1.6f; // km
}

/**
 * @brief Decode ideal range from UI energy graph frame (bits 16-25)
 * @param data Raw CAN payload from frame 0x33A (826)
 * @return Ideal range in km
 */
inline float decodeBmsIdealRange(const uint8_t *data)
{
	uint16_t raw = ((uint16_t)data[2] << 2) | (data[3] >> 6); // bits 16-25
	return raw * 1.6f; // km
}

/**
 * @brief Decode rated energy consumption from UI energy graph frame (bits 32-41)
 * @param data Raw CAN payload from frame 0x33A (826)
 * @return Rated consumption in Wh/km
 */
inline float decodeBmsRatedConsumption(const uint8_t *data)
{
	uint16_t raw = ((uint16_t)data[4] << 2) | (data[5] >> 6); // bits 32-41
	return raw * 0.625f; // Wh/km
}

/**
 * @brief Decode actual SoC integer from UI energy graph frame (bits 48-54)
 * @param data Raw CAN payload from frame 0x33A (826)
 * @return Actual SoC as integer percentage
 */
inline uint8_t decodeBmsActualSocInt(const uint8_t *data)
{
	return data[6] & 0x7F; // bits 48-54: 7-bit integer %
}

/**
 * @brief Decode usable SoC integer from UI energy graph frame (bits 56-62)
 * @param data Raw CAN payload from frame 0x33A (826)
 * @return Usable SoC as integer percentage
 */
inline uint8_t decodeBmsUsableSocInt(const uint8_t *data)
{
	return data[7] & 0x7F; // bits 56-62: 7-bit integer %
}

/**
 * @brief Decode thermal power dissipation from BMS thermal status (bits 0-9)
 * @param data Raw CAN payload from frame 0x312 (786)
 * @return Power dissipation in kW
 */
inline float decodeBmsPowerDissipation(const uint8_t *data)
{
	uint16_t raw = ((uint16_t)data[0] << 2) | (data[1] >> 6); // bits 0-9
	return raw * 0.02f; // kW
}

/**
 * @brief Decode coolant flow request from BMS thermal status (bits 10-16)
 * @param data Raw CAN payload from frame 0x312 (786)
 * @return Requested coolant flow in liters per minute
 */
inline float decodeBmsFlowRequest(const uint8_t *data)
{
	uint8_t raw = ((data[1] & 0x3F) << 1) | (data[2] >> 7); // bits 10-16: 7-bit flow
	return raw * 0.3f; // LPM
}

/**
 * @brief Decode active cooling target temperature from BMS thermal status (bits 17-25)
 * @param data Raw CAN payload from frame 0x312 (786)
 * @return Active cooling inlet target in Celsius
 */
inline float decodeBmsCoolTarget(const uint8_t *data)
{
	uint16_t raw = ((uint16_t)(data[2] & 0x7F) << 2) | (data[3] >> 6); // bits 17-25
	return raw * 0.25f - 25.0f; // offset-25 encoding to °C
}

/**
 * @brief Decode passive cooling target temperature from BMS thermal status (bits 26-34)
 * @param data Raw CAN payload from frame 0x312 (786)
 * @return Passive cooling inlet target in Celsius
 */
inline float decodeBmsPassiveTarget(const uint8_t *data)
{
	uint16_t raw = ((uint16_t)(data[3] & 0x3F) << 3) | (data[4] >> 5); // bits 26-34
	return raw * 0.25f - 25.0f; // offset-25 encoding to °C
}

/**
 * @brief Decode active heating target temperature from BMS thermal status (bits 35-43)
 * @param data Raw CAN payload from frame 0x312 (786)
 * @return Active heating inlet target in Celsius
 */
inline float decodeBmsHeatTarget(const uint8_t *data)
{
	uint16_t raw = ((uint16_t)(data[4] & 0x1F) << 4) | (data[5] >> 4); // bits 35-43
	return raw * 0.25f - 25.0f; // offset-25 encoding to °C
}

/**
 * @brief Decode pack minimum temperature from BMS thermal status (bits 44-52)
 * @param data Raw CAN payload from frame 0x312 (786)
 * @return Pack minimum temperature in Celsius
 */
inline float decodeBmsPackTMin(const uint8_t *data)
{
	uint16_t raw = ((uint16_t)(data[5] & 0x0F) << 5) | (data[6] >> 3); // bits 44-52
	return raw * 0.25f - 25.0f; // offset-25 encoding to °C
}

/**
 * @brief Decode pack maximum temperature from BMS thermal status (bits 53-61)
 * @param data Raw CAN payload from frame 0x312 (786)
 * @return Pack maximum temperature in Celsius
 */
inline float decodeBmsPackTMax(const uint8_t *data)
{
	uint16_t raw = ((uint16_t)(data[6] & 0x07) << 6) | (data[7] >> 2); // bits 53-61
	return raw * 0.25f - 25.0f; // offset-25 encoding to °C
}

/**
 * @brief Decode preconditioning allowed flag from BMS status frame (bit 3)
 * @param data Raw CAN payload from frame 0x212 (530)
 * @return True if battery preconditioning is allowed
 */
inline bool decodeBmsPrecondAllowed(const uint8_t *data)
{
	return (data[0] >> 3) & 0x01; // bit 3 of byte 0
}

/**
 * @brief Decode active heating worthwhile flag from BMS status frame (bit 5)
 * @param data Raw CAN payload from frame 0x212 (530)
 * @return True if active heating would meaningfully improve performance
 */
inline bool decodeBmsHeatingWorthwhile(const uint8_t *data)
{
	return (data[0] >> 5) & 0x01; // bit 5 of byte 0
}

/**
 * @brief Decode HV contactor state from BMS status frame (bits 8-10)
 * @param data Raw CAN payload from frame 0x212 (530)
 * @return Contactor state code (3-bit value)
 */
inline uint8_t decodeBmsContactorState(const uint8_t *data)
{
	return data[1] & 0x07; // bits 8-10: 3-bit contactor state
}

/**
 * @brief Decode HV system state from BMS status frame (bits 16-18)
 * @param data Raw CAN payload from frame 0x212 (530)
 * @return HV state code (3-bit value)
 */
inline uint8_t decodeBmsHvState(const uint8_t *data)
{
	return data[2] & 0x07; // bits 16-18: 3-bit HV state
}

/**
 * @brief Decode minimum pack temperature from BMS status frame (bits 56-63)
 * @param data Raw CAN payload from frame 0x212 (530)
 * @return Minimum pack temperature in Celsius
 */
inline float decodeBmsStatusMinTemp(const uint8_t *data)
{
	return (float)data[7] * 0.5f - 40.0f; // byte 7: 0.5 scale, offset-40 to °C
}

/**
 * @brief Decode minimum bus voltage limit from BMS drive limits frame
 * @param data Raw CAN payload from frame 0x2D2 (722)
 * @return Minimum allowed bus voltage in Volts
 */
inline float decodeBmsMinBusVoltage(const uint8_t *data)
{
	uint16_t raw = ((uint16_t)data[0] << 8) | data[1]; // bytes 0-1: min voltage
	return raw * 0.01f; // V
}

/**
 * @brief Decode maximum bus voltage limit from BMS drive limits frame
 * @param data Raw CAN payload from frame 0x2D2 (722)
 * @return Maximum allowed bus voltage in Volts
 */
inline float decodeBmsMaxBusVoltage(const uint8_t *data)
{
	uint16_t raw = ((uint16_t)data[2] << 8) | data[3]; // bytes 2-3: max voltage
	return raw * 0.01f; // V
}

/**
 * @brief Decode maximum charge current limit from BMS drive limits frame
 * @param data Raw CAN payload from frame 0x2D2 (722)
 * @return Maximum charge current in Amps
 */
inline float decodeBmsMaxChargeCurrent(const uint8_t *data)
{
	uint16_t raw = ((uint16_t)data[4] << 6) | (data[5] >> 2); // bits 32-45: 14-bit charge limit
	return raw * 0.1f; // A
}

/**
 * @brief Decode maximum discharge current limit from BMS drive limits frame
 * @param data Raw CAN payload from frame 0x2D2 (722)
 * @return Maximum discharge current in Amps
 */
inline float decodeBmsMaxDischargeCurrent(const uint8_t *data)
{
	uint16_t raw = ((uint16_t)data[6] << 6) | (data[7] >> 2); // bits 48-61: 14-bit discharge limit
	return raw * 0.128f; // A
}

/**
 * @brief Decode expected remaining energy from BMS energy status (mux=1)
 * @param data Raw CAN payload from frame 0x352 (850)
 * @return Expected remaining energy in kWh
 */
inline float decodeBmsExpectedRemaining(const uint8_t *data)
{
	uint16_t raw = ((uint16_t)data[4] << 8) | data[5]; // bytes 4-5: expected remaining
	return raw * 0.02f; // kWh
}

/**
 * @brief Decode energy buffer from BMS energy status (mux=1)
 * @param data Raw CAN payload from frame 0x352 (850)
 * @return Energy buffer in kWh
 */
inline float decodeBmsEnergyBuffer(const uint8_t *data)
{
	uint16_t raw = ((uint16_t)data[2] << 8) | data[3]; // bytes 2-3: energy buffer
	return raw * 0.01f; // kWh
}

/**
 * @brief Decode energy needed to complete charge from BMS energy status (mux=1)
 * @param data Raw CAN payload from frame 0x352 (850)
 * @return Energy to charge complete in kWh
 */
inline float decodeBmsEnergyToChargeComplete(const uint8_t *data)
{
	uint16_t raw = ((uint16_t)data[6] << 8) | data[7]; // bytes 6-7: energy to full
	return raw * 0.02f; // kWh
}

/**
 * @brief Decode fully charged flag from BMS energy status (mux=1, bit 15)
 * @param data Raw CAN payload from frame 0x352 (850)
 * @return True if battery is fully charged
 */
inline bool decodeBmsFullyCharged(const uint8_t *data)
{
	return (data[1] >> 7) & 0x01; // bit 15: fully charged flag
}

/**
 * @brief Decode lifetime total discharge energy from BMS counters frame
 * @param data Raw CAN payload from frame 0x3D2 (978)
 * @return Total discharged energy in kWh
 */
inline float decodeBmsKwhDischargeTotal(const uint8_t *data)
{
	// bytes 0-3: 32-bit big-endian lifetime discharge counter
	uint32_t raw = ((uint32_t)data[0] << 24) | ((uint32_t)data[1] << 16) | ((uint32_t)data[2] << 8) | data[3];
	return raw * 0.001f; // kWh
}

/**
 * @brief Decode lifetime total charge energy from BMS counters frame
 * @param data Raw CAN payload from frame 0x3D2 (978)
 * @return Total charged energy in kWh
 */
inline float decodeBmsKwhChargeTotal(const uint8_t *data)
{
	// bytes 4-7: 32-bit big-endian lifetime charge counter
	uint32_t raw = ((uint32_t)data[4] << 24) | ((uint32_t)data[5] << 16) | ((uint32_t)data[6] << 8) | data[7];
	return raw * 0.001f; // kWh
}

/**
 * @brief Decode multiplexed energy counter from BMS mux counters frame
 * @param data Raw CAN payload from frame 0x3F2 (1010); byte 0 = mux index
 * @return Counter value in kWh
 */
inline float decodeBmsKwhMuxCounter(const uint8_t *data)
{
	// bytes 1-4: 32-bit big-endian counter (byte 0 is mux selector)
	uint32_t raw = ((uint32_t)data[1] << 24) | ((uint32_t)data[2] << 16) | ((uint32_t)data[3] << 8) | data[4];
	return raw * 0.001f; // kWh
}

/**
 * @brief Decode maximum thermistor temperature from BMS cell data (mux=0)
 * @param data Raw CAN payload from frame 0x332 (818)
 * @return Maximum thermistor temperature in Celsius
 */
inline float decodeBmsThermistorTMax(const uint8_t *data)
{
	return data[2] * 0.5f - 40.0f; // byte 2 (bits 16-23): 0.5 scale, offset-40 to °C
}

/**
 * @brief Decode minimum thermistor temperature from BMS cell data (mux=0)
 * @param data Raw CAN payload from frame 0x332 (818)
 * @return Minimum thermistor temperature in Celsius
 */
inline float decodeBmsThermistorTMin(const uint8_t *data)
{
	return data[3] * 0.5f - 40.0f; // byte 3 (bits 24-31): 0.5 scale, offset-40 to °C
}

/**
 * @brief Decode maximum model temperature from BMS cell data (mux=0)
 * @param data Raw CAN payload from frame 0x332 (818)
 * @return Maximum modeled temperature in Celsius
 */
inline float decodeBmsModelTMax(const uint8_t *data)
{
	return data[4] * 0.5f - 40.0f; // byte 4 (bits 32-39): 0.5 scale, offset-40 to °C
}

/**
 * @brief Decode minimum model temperature from BMS cell data (mux=0)
 * @param data Raw CAN payload from frame 0x332 (818)
 * @return Minimum modeled temperature in Celsius
 */
inline float decodeBmsModelTMin(const uint8_t *data)
{
	return data[5] * 0.5f - 40.0f; // byte 5 (bits 40-47): 0.5 scale, offset-40 to °C
}

/**
 * @brief Decode estimated time to full charge from BMS HV bus frame (bits 48-59)
 * @param data Raw CAN payload from frame 0x132 (306)
 * @return Estimated charge time remaining in hours
 */
inline float decodeBmsChargeTimeToFull(const uint8_t *data)
{
	uint16_t raw = ((uint16_t)data[6] << 4) | (data[7] >> 4); // bits 48-59: 12-bit time
	return raw * 0.01667f; // hours (1/60 scale)
}

/**
 * @brief Execute the BMS telemetry command
 * @param cmd Command string to match ("bms")
 * @param s Device state reference (unused; JSON output handled by serial layer)
 * @return True if command was handled
 */
static bool executeBmsCmd(const char *cmd, State &s)
{
	if (strcmp(cmd, "bms") != 0)
		return false;
	(void)s; // data is output as JSON by the serial layer
	return true;
}
