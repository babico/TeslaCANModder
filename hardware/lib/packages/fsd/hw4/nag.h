#pragma once

#include "can/helpers.h"
#include "packages/runtime/index.h"

namespace packages::fsd::hw4
{

class Nag final : public Package
{
public:
    bool tryHandle(Frame &frame, Context &context) const override
    {
        if (frame.id != 1021 || readMuxID(frame) != 1)
        {
            return false;
        }

        if (!hasFrameBytes(frame, 6))
        {
            return true;
        }

        suppressNagBit(frame);
        setBit(frame, 47, true);
        context.driver.send(frame);
        return true;
    }
};

} // namespace packages::fsd::hw4
