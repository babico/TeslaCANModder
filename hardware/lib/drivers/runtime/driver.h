#pragma once

#include <stdint.h>
#include "can/frame.h"

class Driver
{
public:
    virtual ~Driver() = default;

    virtual bool init() = 0;
    virtual void setFilters(const uint32_t *ids, uint8_t count) = 0;
    virtual bool enableInterrupt(void (*onReady)()) = 0;

    virtual bool read(Frame &frame) = 0;
    virtual void send(const Frame &frame) = 0;
};
