// Copyright (c) 2025 Maze Runner Studios. All Rights Reserved.
// Naruto: Ultimate Shinobi Legacy
// MedicalJutsuBase — Base executor for all Medical Ninjutsu.

#pragma once

#include "CoreMinimal.h"
#include "Jutsu/Core/JutsuExecutor.h"
#include "MedicalJutsuBase.generated.h"

UENUM(BlueprintType)
enum class EMedicalJutsuMode : uint8
{
    InstantHeal     UMETA(DisplayName = "Instant Heal"),
    HealOverTime    UMETA(DisplayName = "Heal Over Time"),
    Revive          UMETA(DisplayName = "Revive"),
    PoisonCure      UMETA(DisplayName = "Cure Poison/Status"),
    ChakraRestore   UMETA(DisplayName = "Restore Chakra"),
    StatBoost       UMETA(DisplayName = "Stat Boost"),
    CellRegeneration UMETA(DisplayName = "Cell Regeneration"),
};

/**
 * UMedicalJutsuBase
 *
 * Base class for all Medical Ninjutsu executors. Handles:
 *   - Instant and over-time healing
 *   - Reviving downed allies
 *   - Status effect curing
 *   - Chakra restoration
 *   - Stat boosting (Strength of a Hundred Seal, etc.)
 *   - Healing scaling with ChakraAttack stat
 *
 * Medical jutsu can target self or allies.
 * Some medical jutsu (Chakra Scalpel) deal damage to enemies.
 */
UCLASS(Abstract, BlueprintType, Blueprintable)
class NARUTOUSL_API UMedicalJutsuBase : public UJutsuExecutorBase
{
    GENERATED_BODY()

public:

    virtual FJutsuExecutionResult Execute_Implementation(
        const FJutsuExecutionContext& Context) override;

protected:

    virtual void ApplyInstantHeal(const FJutsuExecutionContext& Context,
                                   AActor* Target,
                                   FJutsuExecutionResult& OutResult);

    virtual void ApplyHealOverTime(const FJutsuExecutionContext& Context,
                                    AActor* Target,
                                    float HealPerSecond,
                                    float Duration,
                                    FJutsuExecutionResult& OutResult);

    virtual void ApplyRevive(const FJutsuExecutionContext& Context,
                              AActor* Target,
                              FJutsuExecutionResult& OutResult);

    virtual void CureStatusEffects(const FJutsuExecutionContext& Context,
                                    AActor* Target,
                                    FJutsuExecutionResult& OutResult);

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Medical")
    EMedicalJutsuMode MedicalMode = EMedicalJutsuMode::InstantHeal;

    /** Fraction of max health restored on revive. */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Medical",
        meta = (ClampMin = "0.01", ClampMax = "1.0"))
    float ReviveHealthFraction = 0.3f;

    /** Status tags this jutsu can cure. Empty = cure all. */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Medical")
    FGameplayTagContainer CurableStatusTags;
};
