#pragma once

#include "can/helpers.h"
#include "packages/runtime/index.h"

namespace packages::fsd::legacy
{

class StalkSpeed final : public Package
{
public:
    bool tryHandle(Frame &frame, Context &context) const override
    {
        if (frame.id != can::ids::kLegacyStalk)
        {
            return false;
        }

        if (!hasFrameBytes(frame, 2))
        {
            return true;
        }

        context.speedProfile = mapLegacyStalkToSpeedProfile(readLegacyStalkPosition(frame));
        return true;
    }
};

} // namespace packages::fsd::legacy
