#pragma once

#include "can/frame.h"
#include "drivers/runtime/driver.h"

class Handler
{
public:
    virtual ~Handler() = default;

    virtual int &speedProfile() = 0;
    virtual bool &fsdEnabled() = 0;
    virtual int *speedOffset()
    {
        return nullptr;
    }
    virtual const int *speedOffset() const
    {
        return nullptr;
    }

    virtual bool *isaSpeedChimeSuppress()
    {
        return nullptr;
    }
    virtual const bool *isaSpeedChimeSuppress() const
    {
        return nullptr;
    }

    virtual bool supportsNagSuppression() const
    {
        return true;
    }

    virtual void handleMessage(Frame &frame, Driver &driver) = 0;
    virtual const uint32_t *filterIds() const = 0;
    virtual uint8_t filterIdCount() const = 0;
};
