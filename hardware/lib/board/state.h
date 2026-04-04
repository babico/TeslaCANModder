#pragma once

#include "board/variant.h"
#include "handlers/runtime/handler.h"

class BoardState
{
public:
    struct Features
    {
        bool fsd = true;
        bool profile = true;
        bool nag = true;
        bool speedOffset = false;
        bool isaSpeedChime = false;
        bool forceFsd = false;
    };

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

    int speedOffset() const
    {
        return speedOffset_;
    }

    void setSpeedOffset(int speedOffset)
    {
        speedOffset_ = speedOffset;
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

    bool isaSpeedChimeSuppress() const
    {
        return isaSpeedChimeSuppress_;
    }

    void setIsaSpeedChimeSuppress(bool enabled)
    {
        isaSpeedChimeSuppress_ = enabled;
    }

    const Features &features() const
    {
        return features_;
    }

    void refreshFeatures(const Handler &handler)
    {
        features_.fsd = true;
        features_.profile = true;
        features_.nag = handler.supportsNagSuppression();
        features_.speedOffset = handler.speedOffset() != nullptr;
        features_.isaSpeedChime = handler.isaSpeedChimeSuppress() != nullptr;
        features_.forceFsd = false;
    }

    void syncFrom(Handler &handler)
    {
        speedProfile_ = handler.speedProfile();
        fsdEnabled_ = handler.fsdEnabled();

        if (int *speedOffset = handler.speedOffset())
        {
            speedOffset_ = *speedOffset;
        }

        if (bool *isaSpeedChimeSuppress = handler.isaSpeedChimeSuppress())
        {
            isaSpeedChimeSuppress_ = *isaSpeedChimeSuppress;
        }

        refreshFeatures(handler);
    }

    void applyTo(Handler &handler)
    {
        handler.speedProfile() = speedProfile_;
        handler.fsdEnabled() = fsdEnabled_;

        if (int *speedOffset = handler.speedOffset())
        {
            *speedOffset = speedOffset_;
        }

        if (bool *isaSpeedChimeSuppress = handler.isaSpeedChimeSuppress())
        {
            *isaSpeedChimeSuppress = isaSpeedChimeSuppress_;
        }

        refreshFeatures(handler);
    }

private:
    int speedProfile_ = 1;
    int speedOffset_ = 0;
    bool fsdEnabled_ = false;
    bool isaSpeedChimeSuppress_ = true;
    board::Variant variant_ = board::defaultVariant();
    bool variantDirty_ = false;
    InstallReadiness installReadiness_ = InstallReadiness::BenchReady;
    Features features_{};
};
