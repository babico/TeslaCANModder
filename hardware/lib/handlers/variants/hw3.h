#pragma once

#include "can/helpers.h"
#include "handlers/runtime/stack.h"

class Hw3 : public Stack<Hw3, Hw3State>
{
public:
    packages::fsd::hw3::FollowDistance followDistance;
    packages::fsd::hw3::FsdMux0 fsd;
    packages::fsd::hw3::Nag nag;
    packages::fsd::hw3::SpeedOffset speedOffsetUnit;

    int &speedOffsetValue()
    {
        return state().speedOffset;
    }

    int *speedOffset() override
    {
        return &state().speedOffset;
    }

    const int *speedOffset() const override
    {
        return &state().speedOffset;
    }

    const uint32_t *filterIdsArray() const
    {
        static const uint32_t kFilterIds[] = {
            can::ids::kFsdFollowDist,
            can::ids::kFsdMux,
        };
        return kFilterIds;
    }

    uint8_t filterIdCountValue() const
    {
        return 2;
    }

    const Package *const *packages() const
    {
        static const Package *const kPackages[] = {
            &followDistance,
            &fsd,
            &nag,
            &speedOffsetUnit,
        };
        return kPackages;
    }

    uint8_t packageCount() const
    {
        return 4;
    }

    void bind(Context &context)
    {
        context.speedOffset = &state().speedOffset;
    }
};
