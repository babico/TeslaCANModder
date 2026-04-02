#pragma once

#include "handlers/runtime/handler.h"
#include "handlers/runtime/state.h"
#include "packages/index.h"

template <typename Derived, typename UnitState>
class Stack : public Handler
{
public:
    int &speedProfile() override
    {
        return state_.speedProfile;
    }

    bool &fsdEnabled() override
    {
        return state_.fsdEnabled;
    }

    UnitState &state()
    {
        return state_;
    }

    const UnitState &state() const
    {
        return state_;
    }

    void handleMessage(Frame &frame, Driver &driver) override
    {
        Context context{
            state_.speedProfile,
            state_.fsdEnabled,
            driver,
        };

        static_cast<Derived *>(this)->bind(context);
        dispatchVehiclePackages(
            frame,
            context,
            static_cast<const Derived *>(this)->packages(),
            static_cast<const Derived *>(this)->packageCount());
    }

    const uint32_t *filterIds() const override
    {
        return static_cast<const Derived *>(this)->filterIdsArray();
    }

    uint8_t filterIdCount() const override
    {
        return static_cast<const Derived *>(this)->filterIdCountValue();
    }

protected:
    UnitState state_{};
};
