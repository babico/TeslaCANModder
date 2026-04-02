#pragma once

#include "board/variant.h"
#include "handlers/runtime/handler.h"

class BoardState
{
public:
    enum class InstallReadiness
    {
        BenchReady,
        InstalledPowerReady,
        RuntimeReady,
    };

    int speedProfile() const
    {
        return speedProfile_;
    }

    void setSpeedProfile(int speedProfile)
    {
        speedProfile_ = speedProfile;
    }

    bool fsdEnabled() const
    {
        return fsdEnabled_;
    }

    void setFsdEnabled(bool fsdEnabled)
    {
        fsdEnabled_ = fsdEnabled;
    }

    board::Variant variant() const
    {
        return variant_;
    }

    const char *variantName() const
    {
        return board::variantName(variant_);
    }

    void setVariant(board::Variant variant)
    {
        const bool changed = variant_ != variant;
        variant_ = variant;
        variantDirty_ = variantDirty_ || changed;
    }

    bool setVariantByName(const char *name)
    {
        board::Variant parsedVariant;
        if (!board::parseVariantName(name, parsedVariant))
        {
            return false;
        }

        setVariant(parsedVariant);
        return true;
    }

    bool consumeVariantChange()
    {
        const bool changed = variantDirty_;
        variantDirty_ = false;
        return changed;
    }

    InstallReadiness installReadiness() const
    {
        return installReadiness_;
    }

    const char *installReadinessName() const
    {
        switch (installReadiness_)
        {
        case InstallReadiness::BenchReady:
            return "bench-ready";
        case InstallReadiness::InstalledPowerReady:
            return "installed-power-ready";
        case InstallReadiness::RuntimeReady:
            return "runtime-ready";
        default:
            return "bench-ready";
        }
    }

    void setInstallReadiness(InstallReadiness installReadiness)
    {
        installReadiness_ = installReadiness;
    }

    void syncFrom(Handler &handler)
    {
        speedProfile_ = handler.speedProfile();
        fsdEnabled_ = handler.fsdEnabled();
    }

    void applyTo(Handler &handler)
    {
        handler.speedProfile() = speedProfile_;
        handler.fsdEnabled() = fsdEnabled_;
    }

private:
    int speedProfile_ = 1;
    bool fsdEnabled_ = false;
    board::Variant variant_ = board::defaultVariant();
    bool variantDirty_ = false;
    InstallReadiness installReadiness_ = InstallReadiness::BenchReady;
};
