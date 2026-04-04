#pragma once

#include "can/helpers.h"
#include "packages/runtime/index.h"

namespace packages::fsd::hw3
{

class SpeedOffset final : public Package
{
public:
    bool tryHandle(Frame &frame, Context &context) const override
    {
        if (frame.id != 1021 || readMuxID(frame) != 2)
        {
            return false;
        }

        if (!hasFrameBytes(frame, 2))
        {
            return true;
        }

        if (!context.fsdEnabled || context.speedOffset == nullptr)
        {
            return true;
        }

        writeHW3SpeedOffset(frame, *context.speedOffset);
        context.driver.send(frame);
        return true;
    }
};

} // namespace packages::fsd::hw3
