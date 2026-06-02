// Copyright (c) 2025 Maze Runner Studios. All Rights Reserved.
// Naruto: Ultimate Shinobi Legacy
// TaijutsuBase — Base executor for all Taijutsu (physical combat techniques).

#pragma once

#include "CoreMinimal.h"
#include "Jutsu/Core/JutsuExecutor.h"
#include "TaijutsuBase.generated.h"

/**
 * UTaijutsuBase
 *
 * Base class for all Taijutsu executors. Handles:
 *   - Physical damage scaling (PhysicalAttack stat, not ChakraAttack)
 *   - Combo integration (Taijutsu attacks feed into the combo graph)
 *   - Movement-based attacks (dashes, teleports, multi-hit sequences)
 *   - Knockback and launch mechanics
 *   - Speed-based damage bonus (faster characters deal more Taijutsu damage)
 *
 * Concrete jutsu (Dynamic Entry, Leaf Hurricane, Eight Gates, etc.) subclass this.
 */
UCLASS(Abstract, BlueprintType, Blueprintable)
class NARUTOUSL_API UTaijutsuBase : public UJutsuExecutorBase
{
    GENERATED_BODY()

public:

    virtual FJutsuExecutionResult Execute_Implementation(
        const FJutsuExecutionContext& Context) override;

    virtual void OnActivated(const FJutsuExecutionContext& Context) override;

protected:

    /**
     * Applies Taijutsu damage to a target.
     * Uses PhysicalAttack scaling and speed bonus.
     */
    virtual void ApplyTaijutsuDamage(const FJutsuExecutionContext& Context,
                                      AActor* Target,
                                      float DamageAmount,
                                      FJutsuExecutionResult& OutResult);

    /**
     * Executes a dash toward the target before the hit.
     * Used by techniques like Dynamic Entry.
     */
    virtual void ExecuteDash(const FJutsuExecutionContext& Context,
                              AActor* Target,
                              float DashSpeed = 2000.0f);

    /**
     * Executes a multi-hit sequence (e.g., Leaf Whirlwind).
     * @param HitCount  Number of hits in the sequence.
     * @param DamagePerHit  Damage per individual hit.
     */
    virtual void ExecuteMultiHit(const FJutsuExecutionContext& Context,
                                  int32 HitCount,
                                  float DamagePerHit,
                                  FJutsuExecutionResult& OutResult);

    /**
     * Returns the speed-based damage bonus.
     * Characters with higher Speed stat deal more Taijutsu damage.
     */
    float GetSpeedDamageBonus(const FJutsuExecutionContext& Context) const;

    /** Whether this Taijutsu technique uses chakra-enhanced strikes. */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Taijutsu")
    bool bChakraEnhanced = false;

    /** Chakra enhancement damage multiplier. */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Taijutsu",
        meta = (EditCondition = "bChakraEnhanced", ClampMin = "1"))
    float ChakraEnhancementMultiplier = 1.5f;
};
