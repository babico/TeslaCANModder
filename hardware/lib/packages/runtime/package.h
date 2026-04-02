#pragma once

#include "can/frame.h"
#include "packages/runtime/context.h"

class Package
{
public:
    virtual ~Package() = default;

    virtual bool tryHandle(Frame &frame, Context &context) const = 0;
};
