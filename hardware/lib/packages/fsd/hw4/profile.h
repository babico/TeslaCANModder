#pragma once

#include "can/helpers.h"
#include "packages/runtime/index.h"

namespace packages::fsd::hw4
{

    class Profile final : public Package
    {
    public:
        bool tryHandle(Frame &frame, Context &context) const override
        {
            if (frame.id != 1021 || readMuxID(frame) != 2)
            {
                return false;
            }

            if (!hasFrameBytes(frame, 8))
            {
                return true;
            }

            writeHW4SpeedProfile(frame, context.speedProfile);
            context.driver.send(frame);
            return true;
        }
    };

} // namespace packages::fsd::hw4
