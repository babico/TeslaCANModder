#pragma once

#include "drivers/runtime/driver.h"

struct Context
{
    int &speedProfile;
    bool &fsdEnabled;
    Driver &driver;

    int *speedOffset = nullptr;
    bool *isaSpeedChimeSuppress = nullptr;

    Context(int &speedProfileRef, bool &fsdEnabledRef, Driver &driverRef)
        : speedProfile(speedProfileRef),
          fsdEnabled(fsdEnabledRef),
          driver(driverRef)
    {
    }
};
