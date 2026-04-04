#pragma once

#include "can/helpers.h"
#include "packages/runtime/index.h"

namespace packages::fsd::legacy
{

class FsdMux0 final : public Package
{
public:
    bool tryHandle(Frame &frame, Context &context) const override
    {
        if (frame.id != can::ids::kLegacyFsdMux || readMuxID(frame) != 0)
        {
            return false;
        }

        if (!hasFrameBytes(frame, 7))
        {
            return true;
        }

        context.fsdEnabled = isFSDSelectedInUI(frame);
        if (!context.fsdEnabled)
        {
            return true;
        }

        setBit(frame, 46, true);
        setSpeedProfileV12V13(frame, context.speedProfile);
        context.driver.send(frame);
        return true;
    }
};

} // namespace packages::fsd::legacy
