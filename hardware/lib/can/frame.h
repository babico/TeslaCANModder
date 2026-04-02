#pragma once

#include <stdint.h>

struct Frame
{
    uint32_t id = 0;
    uint8_t dlc = 0;
    uint8_t data[8] = {0};

    bool isExtended() const { return id & 0x80000000; }
    uint32_t getExtendedID() const { return id & 0x1FFFFFFF; }
};
