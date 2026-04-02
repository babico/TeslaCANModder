#pragma once

#include "can/helpers.h"
#include "packages/runtime/index.h"

namespace packages::fsd::hw3
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

        suppressNagBit(frame);
        context.driver.send(frame);
        return true;
    }
};

} // namespace packages::fsd::hw3
