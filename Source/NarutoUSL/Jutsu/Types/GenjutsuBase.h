// Copyright (c) 2025 Maze Runner Studios. All Rights Reserved.
// Naruto: Ultimate Shinobi Legacy
// GenjutsuBase — Base executor for all Genjutsu (illusion techniques).

#pragma once

#include "CoreMinimal.h"
#include "Jutsu/Core/JutsuExecutor.h"
#include "GenjutsuBase.generated.h"

UENUM(BlueprintType)
enum class EGenjutsuEffect : uint8
{
    Paralysis       UMETA(DisplayName = "Paralysis"),
    Confusion       UMETA(DisplayName = "Confusion"),
    Fear            UMETA(DisplayName = "Fear"),
    Hallucination   UMETA(DisplayName = "Hallucination"),
    PainAmplify     UMETA(DisplayName = "Pain Amplification"),
    SensoryBlock    UMETA(DisplayName = "Sensory Block"),
    MindControl     UMETA(DisplayName = "Mind Control"),
    Tsukuyomi       UMETA(DisplayName = "Tsukuyomi"),
    Infinite_Tsukuyomi UMETA(DisplayName = "Infinite Tsukuyomi"),
};

/**
 * UGenjutsuBase
 *
 * Base class for all Genjutsu executors. Handles:
 *   - Genjutsu resistance checks (Sharingan users, Sage Mode users resist)
 *   - Genjutsu Kai (release) mechanics
 *   - Psychological damage (bypasses physical defense)
 *   - Duration-based effects (paralysis, confusion, fear)
 *   - Eye contact requirement for certain genjutsu
 *
 * Genjutsu bypasses block and parry (set in DamageTypes.h).
 * Cannot be substituted against.
 */
UCLASS(Abstract, BlueprintType, Blueprintable)
class NARUTOUSL_API UGenjutsuBase : public UJutsuExecutorBase
{
    GENERATED_BODY()

public:

    virtual FJutsuExecutionResult Execute_Implementation(
        const FJutsuExecutionContext& Context) override;

protected:

    /**
     * Checks if the target can resist this genjutsu.
     * Sharingan users, Sage Mode users, and certain characters are immune.
     */
    virtual bool CanTargetResist(const FJutsuExecutionContext& Context,
                                  AActor* Target) const;

    /**
     * Applies the genjutsu effect to the target.
     */
    virtual void ApplyGenjutsuEffect(const FJutsuExecutionContext& Context,
                                      AActor* Target,
                                      FJutsuExecutionResult& OutResult);

    /** The primary effect this genjutsu applies. */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Genjutsu")
    EGenjutsuEffect PrimaryEffect = EGenjutsuEffect::Paralysis;

    /** Whether this genjutsu requires eye contact. */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Genjutsu")
    bool bRequiresEyeContact = false;

    /** Whether this genjutsu can be released with Kai. */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Genjutsu")
    bool bKaiReleasable = true;

    /** Resistance tags that make a target immune to this genjutsu. */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Genjutsu")
    FGameplayTagContainer ImmunityTags;
};
