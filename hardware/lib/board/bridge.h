#pragma once

#include "board/commands.h"
#include "board/config.h"
#include "board/transport.h"
#include "can/frame.h"

class BoardBridge
{
public:
    void begin(BoardState &state)
    {
        transport_.begin();

        transport_.print("{\"t\":\"boot\",\"hw\":\"");
        transport_.print(BOARD_HW_NAME);
        transport_.print("\",\"can\":\"");
        transport_.print(BOARD_CAN_NAME);
        transport_.print("\",\"drv\":\"");
        transport_.print(BOARD_DRIVER_NAME);
        transport_.print("\",\"variant\":\"");
        transport_.print(state.variantName());
        transport_.print("\",\"cap\":\"");
        transport_.print(transport_.bluetoothEnabled() ? "usb+bluetooth" : "usb");
        transport_.print("\",\"ready\":\"");
        transport_.print(state.installReadinessName());
        transport_.print("\",\"btEnabled\":");
        transport_.print(transport_.bluetoothEnabled() ? "1" : "0");
        if (transport_.bluetoothEnabled())
        {
            transport_.print(",\"bt\":\"");
            transport_.print(BOARD_BT_NAME);
            transport_.print("\"");
        }
        transport_.print("}");
        transport_.println();
    }

    void tick(BoardState &state)
    {
        commandRouter_.update(transport_, state);
    }

    void sendLog(const char *message)
    {
        transport_.print("{\"t\":\"log\",\"msg\":\"");
        transport_.print(message);
        transport_.print("\"}");
        transport_.println();
    }

    void sendFrame(const Frame &frame, const char *direction)
    {
        if (!commandRouter_.isFrameStreamingEnabled())
        {
            return;
        }

        transport_.print("{\"t\":\"frame\",\"dir\":\"");
        transport_.print(direction);
        transport_.print("\",\"id\":");
        transport_.printNumber(frame.id);
        transport_.print(",\"dlc\":");
        transport_.printNumber(frame.dlc);
        transport_.print(",\"d\":\"");
        for (uint8_t index = 0; index < frame.dlc; index++)
        {
            transport_.printHexByte(frame.data[index]);
        }
        transport_.print("\"}");
        transport_.println();
    }

private:
    BoardTransport transport_;
    BoardCommandRouter commandRouter_;
};
