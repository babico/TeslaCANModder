#pragma once

/**
 * @file firmware/lib/vehicle/can/checksum.h
 * @brief Shared CAN frame checksum helpers used across feature handlers
 * @author Tesla CAN Mod Contributors
 * @license GPL-3.0
 */

#include "core/types.h"
#include "vehicle/can/ids.h"

/**
 * @brief Compute a Tesla CAN frame checksum for DAS frames.
 *
 * Sums the low and high bytes of the CAN ID plus all payload bytes except the
 * byte at position @p checksumByte, then returns the low 8 bits.
 *
 * @param canId        11/29-bit CAN arbitration ID.
 * @param data         Pointer to frame payload bytes.
 * @param len          Total payload length (DLC).
 * @param checksumByte Index of the checksum byte (excluded from the sum).
 * @return 8-bit checksum value.
 */
inline uint8_t dasChecksum(uint32_t canId, const uint8_t *data, uint8_t len, uint8_t checksumByte)
{
	uint8_t sum = 0;
	sum += canId & 0xFF;
	sum += (canId >> 8) & 0xFF;
	for (uint8_t i = 0; i < len; i++)
	{
		if (i != checksumByte)
			sum += data[i];
	}
	return sum;
}

/**
 * @brief Compute the HW4 ISA speed chime frame checksum.
 *
 * Sums bytes 0–6 of the frame payload plus both bytes of the CAN ID;
 * the checksum is stored in byte 7.
 *
 * @param f CAN frame with at least 8 data bytes.
 * @return Checksum byte, or 0 if DLC < 8.
 */
inline uint8_t computeHW4IsaChecksum(const Frame &f)
{
	if (f.dlc < 8)
		return 0;
	uint8_t sum = 0;
	for (int i = 0; i < 7; i++)
		sum += f.data[i];
	sum += (f.id & 0xFF) + (f.id >> 8);
	return sum;
}

/**
 * @brief Compute the 0x370 EPAS frame checksum used by nag strategies.
 *
 * Sums payload bytes 0–6 plus the CAN ID (0x370) split into its constituent
 * bytes (0x70 + 0x03).
 *
 * @param data Pointer to the 8-byte frame payload.
 * @return Checksum byte (sum & 0xFF).
 */
inline uint8_t nagChecksum(const uint8_t *data)
{
	uint16_t sum = 0;
	for (uint8_t i = 0; i < 7; i++)
		sum += data[i];
	sum += (CAN_ID_EPAS_TORQUE & 0xFF);
	sum += ((CAN_ID_EPAS_TORQUE >> 8) & 0xFF);
	return (uint8_t)(sum & 0xFF);
}
