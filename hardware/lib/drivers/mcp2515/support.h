#pragma once

#include <Arduino.h>
#include <string.h>
#include <mcp2515.h>
#include "board/config.h"
#include "drivers/runtime/driver.h"

namespace drivers::mcp2515
{

    inline bool beginController(MCP2515 &controller)
    {
        if (controller.setBitrate(CAN_500KBPS, MCP_8MHZ) == MCP2515::ERROR_OK)
        {
            return true;
        }
#if BOARD_CAN_CLOCK_ALLOW_FALLBACK
        return controller.setBitrate(CAN_500KBPS, MCP_16MHZ) == MCP2515::ERROR_OK;
#else
        return false;
#endif
    }

    inline void configureStandardFilters(MCP2515 &controller, const uint32_t *ids, uint8_t count)
    {
        controller.setConfigMode();

        if (count == 0 || ids == nullptr)
        {
            controller.setNormalMode();
            return;
        }

        controller.setFilterMask(MCP2515::MASK0, false, 0x7FF);
        controller.setFilter(MCP2515::RXF0, false, ids[0]);
        controller.setFilter(MCP2515::RXF1, false, count > 1 ? ids[1] : ids[0]);

        controller.setFilterMask(MCP2515::MASK1, false, 0x7FF);
        controller.setFilter(MCP2515::RXF2, false, count > 2 ? ids[2] : ids[0]);
        controller.setFilter(MCP2515::RXF3, false, count > 3 ? ids[3] : ids[0]);
        controller.setFilter(MCP2515::RXF4, false, count > 4 ? ids[4] : ids[0]);
        controller.setFilter(MCP2515::RXF5, false, count > 5 ? ids[5] : ids[0]);

        controller.setNormalMode();
    }

    inline bool enableInterrupt(uint8_t intPin, void (*onReady)())
    {
        pinMode(intPin, INPUT_PULLUP);
        attachInterrupt(digitalPinToInterrupt(intPin), onReady, FALLING);
        return true;
    }

    inline bool readFrame(MCP2515 &controller, Frame &frame)
    {
        can_frame raw;
        if (controller.readMessage(&raw) != MCP2515::ERROR_OK)
        {
            return false;
        }

        frame.id = raw.can_id;
        frame.dlc = raw.can_dlc;
        memcpy(frame.data, raw.data, 8);
        return true;
    }

    inline void sendFrame(MCP2515 &controller, const Frame &frame)
    {
        can_frame raw;
        raw.can_id = frame.id;
        raw.can_dlc = frame.dlc;
        memcpy(raw.data, frame.data, 8);
        controller.sendMessage(&raw);
    }

} // namespace drivers::mcp2515
