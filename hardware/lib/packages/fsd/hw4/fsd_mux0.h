#pragma once

#include "can/helpers.h"
#include "packages/runtime/index.h"

namespace packages::fsd::hw4
{

class FsdMux0 final : public Package
{
public:
    bool tryHandle(Frame &frame, Context &context) const override
    {
        if (frame.id != 1021 || readMuxID(frame) != 0)
        {
            return false;
        }

        context.fsdEnabled = isFSDSelectedInUI(frame);
        if (!context.fsdEnabled)
        {
            return true;
        }

        setBit(frame, 46, true);
        setBit(frame, 60, true);
        setBit(frame, 59, true);
        context.driver.send(frame);
        return true;
    }
};

} // namespace packages::fsd::hw4
