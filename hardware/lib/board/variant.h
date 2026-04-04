#pragma once

#include <string.h>
#include "handlers/index.h"

namespace board
{

    enum class Variant
    {
        Hw4,
        Hw3,
        Legacy,
    };

    inline Variant defaultVariant()
    {
        return Variant::Hw4;
    }

    inline const char *variantName(Variant variant)
    {
        switch (variant)
        {
        case Variant::Hw4:
            return "hw4";
        case Variant::Hw3:
            return "hw3";
        case Variant::Legacy:
            return "legacy";
        default:
            return "hw4";
        }
    }

    inline bool parseVariantName(const char *text, Variant &variant)
    {
        if (text == nullptr)
        {
            return false;
        }

        if (strcmp(text, "hw4") == 0)
        {
            variant = Variant::Hw4;
            return true;
        }

        if (strcmp(text, "hw3") == 0)
        {
            variant = Variant::Hw3;
            return true;
        }

        if (strcmp(text, "legacy") == 0)
        {
            variant = Variant::Legacy;
            return true;
        }

        return false;
    }

    inline Handler *createHandler(Variant variant)
    {
        static Hw4 hw4Handler;
        static Hw3 hw3Handler;
        static Legacy legacyHandler;

        switch (variant)
        {
        case Variant::Hw4:
            return &hw4Handler;
        case Variant::Hw3:
            return &hw3Handler;
        case Variant::Legacy:
            return &legacyHandler;
        default:
            return &hw4Handler;
        }
    }

} // namespace board
