#pragma once

#if defined(ARDUINO)
#include <Arduino.h>
#include "board/config.h"

#if BOARD_ENABLE_BT
#include <SoftwareSerial.h>
#endif
#else
#include <stdint.h>
#include "board/config.h"
#endif

class BoardTransport
{
public:
#if !defined(ARDUINO)
    void begin()
    {
    }

    bool bluetoothEnabled() const
    {
        return false;
    }

    bool readUsbChar(char &character)
    {
        (void)character;
        return false;
    }

    bool readBluetoothChar(char &character)
    {
        (void)character;
        return false;
    }

    void print(const char *text)
    {
        (void)text;
    }

    void printNumber(long number)
    {
        (void)number;
    }

    void printHexByte(uint8_t value)
    {
        (void)value;
    }

    void println()
    {
    }
#else
#if BOARD_ENABLE_BT
    BoardTransport() : bluetoothSerial_(BOARD_BT_RX_PIN, BOARD_BT_TX_PIN) {}
#else
    BoardTransport() = default;
#endif

    void begin()
    {
#if BOARD_ENABLE_BT
        bluetoothSerial_.begin(BOARD_BT_BAUD);
#endif
    }

    bool bluetoothEnabled() const
    {
        return board::kBluetoothEnabled;
    }

    bool readUsbChar(char &character)
    {
        if (!Serial.available())
        {
            return false;
        }

        character = static_cast<char>(Serial.read());
        return true;
    }

    bool readBluetoothChar(char &character)
    {
#if BOARD_ENABLE_BT
        if (!bluetoothSerial_.available())
        {
            return false;
        }

        character = static_cast<char>(bluetoothSerial_.read());
        return true;
#else
        (void)character;
        return false;
#endif
    }

    void print(const char *text)
    {
        Serial.print(text);
#if BOARD_ENABLE_BT
        bluetoothSerial_.print(text);
#endif
    }

    void printNumber(long number)
    {
        Serial.print(number);
#if BOARD_ENABLE_BT
        bluetoothSerial_.print(number);
#endif
    }

    void printHexByte(uint8_t value)
    {
        if (value < 0x10)
        {
            print("0");
        }

        Serial.print(value, HEX);
#if BOARD_ENABLE_BT
        bluetoothSerial_.print(value, HEX);
#endif
    }

    void println()
    {
        Serial.println();
#if BOARD_ENABLE_BT
        bluetoothSerial_.println();
#endif
    }

private:
#if BOARD_ENABLE_BT
    SoftwareSerial bluetoothSerial_;
#endif
#endif
};
