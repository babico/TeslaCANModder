#pragma once

#include "can/helpers.h"
#include "packages/runtime/index.h"

namespace packages::fsd::hw3
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

        const int uiOffsetSteps = readHW3UiOffsetSteps(frame);
        if (context.speedOffset != nullptr)
        {
            *context.speedOffset = calculateHW3SpeedOffset(uiOffsetSteps);
        }

        const int mappedProfile = mapHW3UiOffsetToSpeedProfile(uiOffsetSteps);
        if (mappedProfile >= 0)
        {
            context.speedProfile = mappedProfile;
        }

        setBit(frame, 46, true);
        setSpeedProfileV12V13(frame, context.speedProfile);
        context.driver.send(frame);
        return true;
    }
};

} // namespace packages::fsd::hw3
