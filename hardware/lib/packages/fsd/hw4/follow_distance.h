#pragma once

#include "can/helpers.h"
#include "packages/runtime/index.h"

namespace packages::fsd::hw4
{

class FollowDistance final : public Package
{
public:
    bool tryHandle(Frame &frame, Context &context) const override
    {
        if (frame.id != can::ids::kFsdFollowDist)
        {
            return false;
        }

        if (!hasFrameBytes(frame, 6))
        {
            return true;
        }

        const int mappedProfile = mapHW4FollowDistanceToSpeedProfile(readFollowDistance(frame));
        if (mappedProfile >= 0)
        {
            context.speedProfile = mappedProfile;
        }
        return true;
    }
};

} // namespace packages::fsd::hw4
