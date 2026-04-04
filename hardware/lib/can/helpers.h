#pragma once

#include "can/frame.h"

// ── Known Tesla CAN frame IDs ────────────────────────────────────────────────
namespace can
{
    namespace ids
    {
        constexpr uint32_t kLegacyStalk = 69u;     // 0x045 — Legacy stalk-speed frame
        constexpr uint32_t kSpeedSrc = 921u;       // 0x399 — Speed source (HW4 filter)
        constexpr uint32_t kLegacyFsdMux = 1006u;  // 0x3EE — Legacy FSD mux control
        constexpr uint32_t kFsdFollowDist = 1016u; // 0x3F8 — HW3/HW4 follow-distance
        constexpr uint32_t kFsdMux = 1021u;        // 0x3FD — HW3/HW4 FSD mux control
    } // namespace ids
} // namespace can
// ────────────────────────────────────────────────────────────────────────────

inline uint8_t readMuxID(const Frame &frame)
{
    if (frame.dlc < 1)
    {
        return 0;
    }
    return frame.data[0] & 0x07;
}

inline uint8_t readLegacyStalkPosition(const Frame &frame)
{
    if (frame.dlc < 2)
    {
        return 0;
    }
    return frame.data[1] >> 5;
}

inline uint8_t readFollowDistance(const Frame &frame)
{
    if (frame.dlc < 6)
    {
        return 0;
    }
    return (frame.data[5] & 0b11100000) >> 5;
}

inline bool isFSDSelectedInUI(const Frame &frame)
{
    if (frame.dlc < 5)
    {
        return false;
    }
    return (frame.data[4] >> 6) & 0x01;
}

inline bool hasFrameBytes(const Frame &frame, uint8_t requiredDlc)
{
    return frame.dlc >= requiredDlc;
}

inline int mapLegacyStalkToSpeedProfile(uint8_t stalkPosition)
{
    if (stalkPosition <= 1)
    {
        return 2;
    }
    if (stalkPosition == 2)
    {
        return 1;
    }
    return 0;
}

inline int mapHW3FollowDistanceToSpeedProfile(uint8_t followDistance)
{
    switch (followDistance)
    {
    case 1:
        return 2;
    case 2:
        return 1;
    case 3:
        return 0;
    default:
        return -1;
    }
}

inline int mapHW4FollowDistanceToSpeedProfile(uint8_t followDistance)
{
    switch (followDistance)
    {
    case 1:
        return 3;
    case 2:
        return 2;
    case 3:
        return 1;
    case 4:
        return 0;
    case 5:
        return 4;
    default:
        return -1;
    }
}

inline int readHW3UiOffsetSteps(const Frame &frame)
{
    return static_cast<int>((frame.data[3] >> 1) & 0x3F) - 30;
}

inline int calculateHW3SpeedOffset(int uiOffsetSteps)
{
    const int speedOffset = uiOffsetSteps * 5;
    if (speedOffset < 0)
    {
        return 0;
    }
    if (speedOffset > 100)
    {
        return 100;
    }
    return speedOffset;
}

inline int mapHW3UiOffsetToSpeedProfile(int uiOffsetSteps)
{
    switch (uiOffsetSteps)
    {
    case 2:
        return 2;
    case 1:
        return 1;
    case 0:
        return 0;
    default:
        return -1;
    }
}

inline void setSpeedProfileV12V13(Frame &frame, int profile)
{
    if (!hasFrameBytes(frame, 7))
    {
        return;
    }

    frame.data[6] &= ~0x06;
    frame.data[6] |= (profile << 1);
}

inline void setBit(Frame &frame, int bit, bool value)
{
    if (bit < 0 || bit >= 64)
    {
        return;
    }

    const int byteIndex = bit / 8;
    if (frame.dlc <= byteIndex)
    {
        return;
    }

    const int bitIndex = bit % 8;
    const uint8_t mask = static_cast<uint8_t>(1U << bitIndex);
    if (value)
    {
        frame.data[byteIndex] |= mask;
    }
    else
    {
        frame.data[byteIndex] &= static_cast<uint8_t>(~mask);
    }
}

inline void suppressNagBit(Frame &frame)
{
    setBit(frame, 19, false);
}

inline void writeHW3SpeedOffset(Frame &frame, int speedOffset)
{
    if (!hasFrameBytes(frame, 2))
    {
        return;
    }

    frame.data[0] &= ~(0b11000000);
    frame.data[1] &= ~(0b00111111);
    frame.data[0] |= static_cast<uint8_t>((speedOffset & 0x03) << 6);
    frame.data[1] |= static_cast<uint8_t>(speedOffset >> 2);
}

inline void writeHW4SpeedProfile(Frame &frame, int speedProfile)
{
    if (!hasFrameBytes(frame, 8))
    {
        return;
    }

    frame.data[7] &= ~(0x07 << 4);
    frame.data[7] |= static_cast<uint8_t>((speedProfile & 0x07) << 4);
}

inline uint8_t computeHW4IsaChecksum(const Frame &frame)
{
    if (!hasFrameBytes(frame, 8))
    {
        return 0;
    }

    uint8_t sum = 0;
    for (int index = 0; index < 7; index++)
    {
        sum += frame.data[index];
    }
    sum += static_cast<uint8_t>(frame.id & 0xFF);
    sum += static_cast<uint8_t>(frame.id >> 8);
    return static_cast<uint8_t>(sum & 0xFF);
}
