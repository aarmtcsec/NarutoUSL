// Copyright (c) 2025 Maze Runner Studios. All Rights Reserved.
// Naruto: Ultimate Shinobi Legacy
// NinjutsuBase — Base executor for all Ninjutsu (chakra-based techniques).

#pragma once

#include "CoreMinimal.h"
#include "Jutsu/Core/JutsuExecutor.h"
#include "NinjutsuBase.generated.h"

/**
 * UNinjutsuBase
 *
 * Base class for all Ninjutsu executors. Handles:
 *   - Elemental damage application with nature affinity bonuses
 *   - Projectile spawning for ranged ninjutsu
 *   - AoE evaluation for area techniques
 *   - Elemental interaction checks (Fire + Wind = boosted, Water + Lightning = boosted)
 *
 * Concrete jutsu (Fireball, Rasengan, Shadow Clone, etc.) subclass this.
 */
UCLASS(Abstract, BlueprintType, Blueprintable)
class NARUTOUSL_API UNinjutsuBase : public UJutsuExecutorBase
{
    GENERATED_BODY()

public:

    virtual FJutsuExecutionResult Execute_Implementation(
        const FJutsuExecutionContext& Context) override;

    virtual void OnActivated(const FJutsuExecutionContext& Context) override;
    virtual void OnCompleted(const FJutsuExecutionContext& Context) override;

protected:

    /**
     * Applies ninjutsu damage to a single target.
     * Handles elemental interactions and nature affinity bonuses.
     */
    virtual void ApplyNinjutsuDamage(const FJutsuExecutionContext& Context,
                                      AActor* Target,
                                      float DamageAmount,
                                      FJutsuExecutionResult& OutResult);

    /**
     * Spawns a projectile for ranged ninjutsu.
     * Returns the spawned projectile actor.
     */
    virtual AActor* SpawnProjectile(const FJutsuExecutionContext& Context);

    /**
     * Evaluates AoE targets and applies damage to all of them.
     */
    virtual void ExecuteAoE(const FJutsuExecutionContext& Context,
                             FJutsuExecutionResult& OutResult);

    /**
     * Returns the elemental interaction multiplier between two natures.
     * e.g., Fire + Wind = 1.5x, Water + Lightning = 1.5x
     */
    float GetElementalInteractionMultiplier(EChakraNature AttackerNature,
                                             EDamageType DamageType) const;

    /**
     * Returns the nature affinity bonus for the caster.
     * Casters with matching nature affinity deal +10% damage.
     */
    float GetNatureAffinityBonus(const FJutsuExecutionContext& Context) const;
};
