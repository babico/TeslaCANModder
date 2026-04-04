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
        writeBoot(transport_, state);
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

    void sendFrame(const Frame &frame, const char *direction, unsigned long captureMs)
    {
        writeFrame(transport_, frame, direction, captureMs);
    }

    template <typename Transport>
    void writeBoot(Transport &transport, BoardState &state)
    {
        const BoardState::Features &features = state.features();

        transport.print("{\"t\":\"boot\",\"hw\":\"");
        transport.print(BOARD_HW_NAME);
        transport.print("\",\"can\":\"");
        transport.print(BOARD_CAN_NAME);
        transport.print("\",\"drv\":\"");
        transport.print(BOARD_DRIVER_NAME);
        transport.print("\",\"variant\":\"");
        transport.print(state.variantName());
        transport.print("\",\"cap\":\"");
        transport.print(transport.bluetoothEnabled() ? "usb+bluetooth" : "usb");
        transport.print("\",\"ready\":\"");
        transport.print(state.installReadinessName());
        transport.print("\",\"btEnabled\":");
        transport.print(transport.bluetoothEnabled() ? "1" : "0");
        transport.print(",\"offset\":");
        transport.printNumber(state.speedOffset());
        transport.print(",\"isaChime\":");
        transport.print(state.isaSpeedChimeSuppress() ? "1" : "0");
        transport.print(",\"features\":{");
        transport.print("\"fsd\":");
        transport.print(features.fsd ? "1" : "0");
        transport.print(",\"profile\":");
        transport.print(features.profile ? "1" : "0");
        transport.print(",\"nag\":");
        transport.print(features.nag ? "1" : "0");
        transport.print(",\"speedOffset\":");
        transport.print(features.speedOffset ? "1" : "0");
        transport.print(",\"isaSpeedChime\":");
        transport.print(features.isaSpeedChime ? "1" : "0");
        transport.print(",\"forceFsd\":");
        transport.print(features.forceFsd ? "1" : "0");
        transport.print("}");
        transport.print(",\"stream\":{");
        transport.print("\"on\":0,\"emitted\":0}");
        transport.print(",\"rawCan\":");
        transport.print(state.rawCanListen() ? "1" : "0");
        if (transport.bluetoothEnabled())
        {
            transport.print(",\"bt\":\"");
            transport.print(BOARD_BT_NAME);
            transport.print("\"");
        }
        transport.print("}");
        transport.println();
    }

    template <typename Transport>
    void writeFrame(Transport &transport, const Frame &frame, const char *direction, unsigned long captureMs)
    {
        if (!commandRouter_.isFrameStreamingEnabled())
        {
            return;
        }

        commandRouter_.recordFrameEmitted();

        transport.print("{\"t\":\"frame\",\"dir\":\"");
        transport.print(direction);
        transport.print("\",\"id\":");
        transport.printNumber(static_cast<long>(frame.isExtended() ? frame.getExtendedID() : frame.id));
        transport.print(",\"seq\":");
        transport.printNumber(static_cast<long>(commandRouter_.streamedFrameCount()));
        transport.print(",\"ms\":");
        transport.printNumber(static_cast<long>(captureMs));
        transport.print(",\"ext\":");
        transport.print(frame.isExtended() ? "1" : "0");
        transport.print(",\"dlc\":");
        transport.printNumber(frame.dlc);
        transport.print(",\"d\":\"");
        for (uint8_t index = 0; index < frame.dlc; index++)
        {
            transport.printHexByte(frame.data[index]);
        }
        transport.print("\"}");
        transport.println();
    }

    BoardCommandRouter &routerForTest()
    {
        return commandRouter_;
    }

private:
    BoardTransport transport_;
    BoardCommandRouter commandRouter_;
};
