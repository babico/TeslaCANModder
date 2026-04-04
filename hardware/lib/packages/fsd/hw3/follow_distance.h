#pragma once

#include "can/helpers.h"
#include "packages/runtime/index.h"

namespace packages::fsd::hw3
{

class FollowDistance final : public Package
{
public:
    bool tryHandle(Frame &frame, Context &context) const override
    {
        if (frame.id != 1016)
        {
            return false;
        }

        if (!hasFrameBytes(frame, 6))
        {
            return true;
        }

        const int mappedProfile = mapHW3FollowDistanceToSpeedProfile(readFollowDistance(frame));
        if (mappedProfile >= 0)
        {
            context.speedProfile = mappedProfile;
        }
        return true;
    }
};

} // namespace packages::fsd::hw3
