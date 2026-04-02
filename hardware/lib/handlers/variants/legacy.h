#pragma once

#include "handlers/runtime/stack.h"

class Legacy : public Stack<Legacy, LegacyState>
{
public:
    packages::fsd::legacy::StalkSpeed stalk;
    packages::fsd::legacy::FsdMux0 fsd;
    packages::fsd::legacy::Nag nag;

    const uint32_t *filterIdsArray() const
    {
        static const uint32_t kFilterIds[] = {69, 1006};
        return kFilterIds;
    }

    uint8_t filterIdCountValue() const
    {
        return 2;
    }

    const Package *const *packages() const
    {
        static const Package *const kPackages[] = {
            &stalk,
            &fsd,
            &nag,
        };
        return kPackages;
    }

    uint8_t packageCount() const
    {
        return 3;
    }

    void bind(Context &context)
    {
        (void)context;
    }
};
