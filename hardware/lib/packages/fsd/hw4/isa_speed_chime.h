#pragma once

#include "can/helpers.h"
#include "packages/runtime/index.h"

namespace packages::fsd::hw4
{

class IsaSpeedChime final : public Package
{
public:
    bool tryHandle(Frame &frame, Context &context) const override
    {
        if (frame.id != 921)
        {
            return false;
        }

        if (!hasFrameBytes(frame, 8))
        {
            return true;
        }

        // This package claims frame 921 entirely. When the feature toggle is off
        // we still consume the frame so no other package touches it accidentally.
        if (context.isaSpeedChimeSuppress == nullptr || !(*context.isaSpeedChimeSuppress))
        {
            return true;
        }

        frame.data[1] |= 0x20;
        frame.data[7] = computeHW4IsaChecksum(frame);
        context.driver.send(frame);
        return true;
    }
};

} // namespace packages::fsd::hw4
