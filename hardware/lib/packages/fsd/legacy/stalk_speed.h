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
        if (frame.id != 69)
        {
            return false;
        }

        context.speedProfile = mapLegacyStalkToSpeedProfile(readLegacyStalkPosition(frame));
        return true;
    }
};

} // namespace packages::fsd::legacy
