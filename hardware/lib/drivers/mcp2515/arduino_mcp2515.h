#pragma once

#include <SPI.h>
#include <mcp2515.h>
#include "drivers/mcp2515/support.h"
#include "drivers/runtime/driver.h"

#ifndef PIN_LED
#define PIN_LED 13
#endif

class ArduinoMCP2515 : public Driver
{
public:
    static constexpr bool kSupportsISR = true;

    explicit ArduinoMCP2515(uint8_t csPin = 10, uint8_t intPin = 2)
        : mcp_(csPin), intPin_(intPin) {}

    bool init() override
    {
        mcp_.reset();

        // The ordered SAMM MCP2515/TJA1050 board is 8 MHz. We keep an optional
        // fallback for bring-up in case a different shield gets attached later.
        if (!drivers::mcp2515::beginController(mcp_))
        {
            return false;
        }

        mcp_.setNormalMode();
        return true;
    }

    void setFilters(const uint32_t *ids, uint8_t count) override
    {
        drivers::mcp2515::configureStandardFilters(mcp_, ids, count);
    }

    bool enableInterrupt(void (*onReady)()) override
    {
        return drivers::mcp2515::enableInterrupt(intPin_, onReady);
    }

    bool read(Frame &frame) override
    {
        return drivers::mcp2515::readFrame(mcp_, frame);
    }

    void send(const Frame &frame) override
    {
        drivers::mcp2515::sendFrame(mcp_, frame);
    }

private:
    MCP2515 mcp_;
    uint8_t intPin_;
};
