#pragma once

/**
 * @file firmware/lib/transport/can/handler/frame_readers.h
 * @brief Stateless CAN frame decoders for DAS and gateway signals
 * @author Tesla CAN Mod Contributors
 * @license GPL-3.0
 */

#include "core/types.h"

/**
 * @brief Read DAS autopilot status from a 0x39B frame
 * @param f Reference to the CAN frame to decode
 * @return Autopilot status nibble from byte 0 bits [3:0], or 0 if frame too short
 */
inline uint8_t readDASAutopilotStatus(const Frame &f)
{
	return f.dlc >= 1 ? (f.data[0] & 0x0F) : 0;
}

/**
 * @brief Read DAS autopilot state from a 0x39B frame
 *
 * Values: 0=UNAVAIL, 1=AVAIL, 2=ACTIVE_NOMINAL, 3=ACTIVE_MIN_DRIVER, etc.
 * Used by AP-First mode to delay 0x3FD injection until AP is running.
 *
 * @param f Reference to the CAN frame to decode
 * @return Autopilot state nibble from byte 1 bits [7:4], or 0 if frame too short
 */
inline uint8_t readDASAutopilotState(const Frame &f)
{
	return f.dlc >= 2 ? ((f.data[1] >> 4) & 0x0F) : 0;
}

/**
 * @brief Check whether the DAS autopilot status indicates an active engagement
 * @param status Autopilot status value previously read from a DAS frame
 * @return True if status is in the active range (3-5)
 */
inline bool isDASAutopilotActive(uint8_t status)
{
	return status >= 3 && status <= 5;
}

/**
 * @brief Read autopilot hardware tier from a GTW_carConfig mux=2 frame
 *
 * Extracts byte 5 bits [4:2] which encode the autopilot hardware tier.
 *
 * @param f Reference to the CAN frame to decode
 * @return Hardware tier (0-7), or -1 if the frame is not mux=2 or is too short
 */
inline int8_t readGtwAutopilotTier(const Frame &f)
{
	if (f.dlc < 6)
		return -1;
	if (readMuxID(f) != 2)
		return -1;
	return (int8_t)((f.data[5] >> 2) & 0x07);
}
