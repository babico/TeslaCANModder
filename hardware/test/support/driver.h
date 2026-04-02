#pragma once

#include <vector>
#include "can/frame.h"
#include "drivers/runtime/driver.h"

class TestDriver : public Driver
{
public:
    static constexpr bool kSupportsISR = false;

    std::vector<Frame> sent;

    bool init() override { return true; }
    void setFilters(const uint32_t * /*ids*/, uint8_t /*count*/) override {}
    bool enableInterrupt(void (* /*onReady*/)()) override { return false; }
    bool read(Frame & /*frame*/) override { return false; }

    void send(const Frame &frame) override
    {
        sent.push_back(frame);
    }

    void reset()
    {
        sent.clear();
    }
};
