#pragma once

#include <stdlib.h>
#include <string.h>
#include "board/config.h"
#include "board/state.h"

#if !defined(ARDUINO)
unsigned long millis();
#endif

class BoardCommandRouter
{
public:
    template <typename Transport>
    void update(Transport &transport, BoardState &state)
    {
        const unsigned long now = millis();
        if (now - lastStatusMs_ >= 500)
        {
            sendStatus(transport, state, now);
        }

        char character = '\0';
        while (transport.readUsbChar(character))
        {
            handleCharacter(transport, state, usbBuffer_, usbLength_, character);
        }

        while (transport.readBluetoothChar(character))
        {
            handleCharacter(transport, state, bluetoothBuffer_, bluetoothLength_, character);
        }
    }

    bool isFrameStreamingEnabled() const
    {
        return streamFrames_;
    }

    template <typename Transport>
    void runCommand(Transport &transport, BoardState &state, const char *command, unsigned long now)
    {
        executeCommand(transport, state, command, now);
    }

    template <typename Transport>
    void sendStatus(Transport &transport, BoardState &state, unsigned long now)
    {
        lastStatusMs_ = now;

        transport.print("{\"t\":\"status\",\"hw\":\"");
        transport.print(BOARD_HW_NAME);
        transport.print("\",\"can\":\"");
        transport.print(BOARD_CAN_NAME);
        transport.print("\",\"drv\":\"");
        transport.print(BOARD_DRIVER_NAME);
        transport.print("\",\"variant\":\"");
        transport.print(state.variantName());
        transport.print("\",\"cap\":\"");
#if BOARD_ENABLE_BT
        transport.print("usb+bluetooth");
#else
        transport.print("usb");
#endif
        transport.print("\",\"ready\":\"");
        transport.print(state.installReadinessName());
        transport.print("\",\"bt\":");
#if BOARD_ENABLE_BT
        transport.print("1");
#else
        transport.print("0");
#endif
        transport.print(",\"fsd\":");
        transport.print(state.fsdEnabled() ? "1" : "0");
        transport.print(",\"sp\":");
        transport.printNumber(state.speedProfile());
        transport.print(",\"up\":");
        transport.printNumber(now);
        transport.print("}");
        transport.println();
    }

private:
    unsigned long lastStatusMs_ = 0;
    bool streamFrames_ = false;

    char usbBuffer_[32] = {};
    uint8_t usbLength_ = 0;
    char bluetoothBuffer_[32] = {};
    uint8_t bluetoothLength_ = 0;

    template <typename Transport>
    void sendAck(Transport &transport, const char *command)
    {
        transport.print("{\"t\":\"ack\",\"cmd\":\"");
        transport.print(command);
        transport.print("\"}");
        transport.println();
    }

    template <typename Transport>
    void sendError(Transport &transport, const char *message)
    {
        transport.print("{\"t\":\"error\",\"msg\":\"");
        transport.print(message);
        transport.print("\"}");
        transport.println();
    }

    template <typename Transport>
    void handleCharacter(
        Transport &transport,
        BoardState &state,
        char *buffer,
        uint8_t &length,
        char character)
    {
        if (character == '\n' || length >= 31)
        {
            buffer[length] = '\0';
            executeCommand(transport, state, buffer, millis());
            length = 0;
            return;
        }

        if (character != '\r')
        {
            buffer[length++] = character;
        }
    }

    template <typename Transport>
    bool applyProfileCommand(Transport &transport, BoardState &state, const char *command, unsigned long now)
    {
        const char *valueText = nullptr;

        if (strncmp(command, "profile:", 8) == 0)
        {
            valueText = command + 8;
        }
        else if (strncmp(command, "sp:", 3) == 0)
        {
            valueText = command + 3;
        }
        else
        {
            return false;
        }

        char *endPtr = nullptr;
        const long parsed = strtol(valueText, &endPtr, 10);
        if (endPtr == valueText || *endPtr != '\0' || parsed < 0 || parsed > 4)
        {
            sendError(transport, "Invalid speed profile");
            return true;
        }

        state.setSpeedProfile(static_cast<int>(parsed));
        sendAck(transport, command);
        sendStatus(transport, state, now);
        return true;
    }

    template <typename Transport>
    void executeCommand(Transport &transport, BoardState &state, const char *command, unsigned long now)
    {
        if (strcmp(command, "stream:on") == 0)
        {
            streamFrames_ = true;
            sendAck(transport, "stream:on");
            return;
        }

        if (strcmp(command, "stream:off") == 0)
        {
            streamFrames_ = false;
            sendAck(transport, "stream:off");
            return;
        }

        if (strcmp(command, "ping") == 0)
        {
            transport.print("{\"t\":\"pong\",\"v\":1}");
            transport.println();
            return;
        }

        if (strcmp(command, "status") == 0)
        {
            sendStatus(transport, state, now);
            return;
        }

        if (strcmp(command, "fsd:on") == 0)
        {
            state.setFsdEnabled(true);
            sendAck(transport, "fsd:on");
            sendStatus(transport, state, now);
            return;
        }

        if (strcmp(command, "fsd:off") == 0)
        {
            state.setFsdEnabled(false);
            sendAck(transport, "fsd:off");
            sendStatus(transport, state, now);
            return;
        }

        if (strcmp(command, "fsd:toggle") == 0)
        {
            state.setFsdEnabled(!state.fsdEnabled());
            sendAck(transport, "fsd:toggle");
            sendStatus(transport, state, now);
            return;
        }

        if (applyProfileCommand(transport, state, command, now))
        {
            return;
        }

        if (strncmp(command, "variant:", 8) == 0)
        {
            if (!state.setVariantByName(command + 8))
            {
                sendError(transport, "Invalid variant");
                return;
            }

            sendAck(transport, command);
            sendStatus(transport, state, now);
            return;
        }

        if (command[0] != '\0')
        {
            sendError(transport, "Unknown command");
        }
    }
};
