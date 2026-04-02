#pragma once

#include "can/helpers.h"
#include "packages/runtime/index.h"

namespace packages::fsd::legacy
{

class Nag final : public Package
{
public:
    bool tryHandle(Frame &frame, Context &context) const override
    {
        if (frame.id != 1006 || readMuxID(frame) != 1)
        {
            return false;
        }

        suppressNagBit(frame);
        context.driver.send(frame);
        return true;
    }
};

} // namespace packages::fsd::legacy
