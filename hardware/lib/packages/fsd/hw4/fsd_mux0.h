#pragma once

#include "board/config.h"
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

        if (!hasFrameBytes(frame, 8))
        {
            return true;
        }

        context.fsdEnabled = isFSDSelectedInUI(frame);
        if (!context.fsdEnabled)
        {
            return true;
        }

        setBit(frame, 46, true);
        setBit(frame, 60, true);
#if BOARD_HW4_ENABLE_EMERGENCY_BIT59
        setBit(frame, 59, true);
#endif
        context.driver.send(frame);
        return true;
    }
};

} // namespace packages::fsd::hw4
