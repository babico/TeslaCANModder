#pragma once

#include "handlers/runtime/stack.h"

class Hw4 : public Stack<Hw4, Hw4State>
{
public:
    packages::fsd::hw4::IsaSpeedChime isaSpeedChime;
    packages::fsd::hw4::FollowDistance followDistance;
    packages::fsd::hw4::FsdMux0 fsd;
    packages::fsd::hw4::Nag nag;
    packages::fsd::hw4::Profile profile;

    bool &isaSpeedChimeSuppressValue()
    {
        return state().isaSpeedChimeSuppress;
    }

    bool *isaSpeedChimeSuppress() override
    {
        return &state().isaSpeedChimeSuppress;
    }

    const bool *isaSpeedChimeSuppress() const override
    {
        return &state().isaSpeedChimeSuppress;
    }

    const uint32_t *filterIdsArray() const
    {
        static const uint32_t kFilterIds[] = {921, 1016, 1021};
        return kFilterIds;
    }

    uint8_t filterIdCountValue() const
    {
        return 3;
    }

    const Package *const *packages() const
    {
        static const Package *const kPackages[] = {
            &isaSpeedChime,
            &followDistance,
            &fsd,
            &nag,
            &profile,
        };
        return kPackages;
    }

    uint8_t packageCount() const
    {
        return 5;
    }

    void bind(Context &context)
    {
        context.isaSpeedChimeSuppress = &state().isaSpeedChimeSuppress;
    }
};
