#pragma once

#include "packages/runtime/package.h"

inline void dispatchVehiclePackages(
    Frame &frame,
    Context &context,
    const Package *const *packages,
    size_t packageCount)
{
    for (size_t i = 0; i < packageCount; ++i)
    {
        const Package *package = packages[i];
        if (package->tryHandle(frame, context))
        {
            return;
        }
    }
}
