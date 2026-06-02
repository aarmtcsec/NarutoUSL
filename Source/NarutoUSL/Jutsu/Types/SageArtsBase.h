// Copyright (c) 2025 Maze Runner Studios. All Rights Reserved.
// Naruto: Ultimate Shinobi Legacy
// SageArtsBase — Base executor for all Sage Art techniques.

#pragma once

#include "CoreMinimal.h"
#include "Jutsu/Core/JutsuExecutor.h"
#include "SageArtsBase.generated.h"

UENUM(BlueprintType)
enum class ESageArtType : uint8
{
    FrogKata        UMETA(DisplayName = "Frog Kata"),
    SageRasengan    UMETA(DisplayName = "Sage Art Rasengan"),
    SerpentSage     UMETA(DisplayName = "Serpent Sage Art"),
    SlugSage        UMETA(DisplayName = "Slug Sage Art"),
    SixPaths        UMETA(DisplayName = "Six Paths Sage Art"),
    NaturalEnergy   UMETA(DisplayName = "Natural Energy Technique"),
};

/**
 * USageArtsBase
 *
 * Base class for all Sage Art executors. Handles:
 *   - Sage chakra requirement validation
 *   - Natural energy amplification (Sage Arts deal 1.5x base damage)
 *   - Invisible attack range (Frog Kata extends hitbox beyond physical reach)
 *   - Sage Mode stat bonuses applied to damage
 *   - Sage chakra consumption per use
 *
 * Sage Arts require the caster to be in Sage Mode (SageMode stance).
 * They consume Sage Chakra in addition to regular chakra.
 */
UCLASS(Abstract, BlueprintType, Blueprintable)
class NARUTOUSL_API USageArtsBase : public UJutsuExecutorBase
{
    GENERATED_BODY()

public:

    virtual FJutsuExecutionResult Execute_Implementation(
        const FJutsuExecutionContext& Context) override;

    virtual void OnCastBegin(const FJutsuExecutionContext& Context) override;

protected:

    virtual bool ValidateSageMode(const FJutsuExecutionContext& Context) const;

    virtual void ApplySageArtDamage(const FJutsuExecutionContext& Context,
                                     AActor* Target,
                                     float DamageAmount,
                                     FJutsuExecutionResult& OutResult);

    /** Returns the natural energy amplification multiplier. */
    float GetNaturalEnergyMultiplier(const FJutsuExecutionContext& Context) const;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SageArts")
    ESageArtType SageArtType = ESageArtType::FrogKata;

    /** Sage chakra consumed per use. */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SageArts", meta = (ClampMin = "0"))
    float SageChakraCost = 25.0f;

    /** Extended hitbox range for Frog Kata-style techniques. */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SageArts", meta = (ClampMin = "0"))
    float ExtendedHitboxRange = 0.0f;

    /** Base natural energy damage multiplier. */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SageArts", meta = (ClampMin = "1"))
    float NaturalEnergyMultiplier = 1.5f;
};
