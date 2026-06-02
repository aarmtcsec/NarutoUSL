// Copyright (c) 2025 Maze Runner Studios. All Rights Reserved.
// Naruto: Ultimate Shinobi Legacy
// JutsuExecutor — Base class for all jutsu execution logic.
// One subclass per jutsu type (Ninjutsu, Taijutsu, etc.).
// Concrete jutsu implementations subclass the type base.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "GameplayTagContainer.h"
#include "Jutsu/Data/JutsuData.h"
#include "Core/Types/NarutoTypes.h"
#include "JutsuExecutor.generated.h"

class ANarutoCharacterBase;
class UCombatManager;
class UChakraComponent;

// ============================================================
//  Execution Context
// ============================================================

/**
 * FJutsuExecutionContext
 * Full context passed to a JutsuExecutor when a jutsu is activated.
 * Contains everything the executor needs to run without querying
 * external systems directly.
 */
USTRUCT()
struct NARUTOUSL_API FJutsuExecutionContext
{
    GENERATED_BODY()

    /** The character casting the jutsu. */
    TWeakObjectPtr<ANarutoCharacterBase> Caster;

    /** Primary target (may be null for AoE/self jutsu). */
    TWeakObjectPtr<AActor> PrimaryTarget;

    /** All targets in range (for AoE). */
    TArray<TWeakObjectPtr<AActor>> AllTargets;

    /** The jutsu data asset. */
    TWeakObjectPtr<UJutsuData> JutsuData;

    /** Caster's mastery level for this jutsu. */
    int32 MasteryLevel = 0;

    /** Charge fraction (0-1). 1 = fully charged. */
    float ChargeFraction = 1.0f;

    /** World-space aim direction. */
    FVector AimDirection = FVector::ForwardVector;

    /** World-space aim location (cursor/target position). */
    FVector AimLocation = FVector::ZeroVector;

    /** Whether this execution was triggered by AI. */
    bool bIsAIControlled = false;

    /** Difficulty scale from CombatManager. */
    float DifficultyScale = 1.0f;

    /** Resolved caster stats at time of cast. */
    float CasterChakraAttack   = 100.0f;
    float CasterPhysicalAttack = 100.0f;
    float CasterMasteryDamageMultiplier = 1.0f;
    float CasterMasteryCostMultiplier   = 1.0f;

    bool IsValid() const { return Caster.IsValid() && JutsuData.IsValid(); }
};

// ============================================================
//  Execution Result
// ============================================================

USTRUCT()
struct NARUTOUSL_API FJutsuExecutionResult
{
    GENERATED_BODY()

    bool bSuccess = false;
    bool bHitAnyTarget = false;

    float TotalDamageDealt  = 0.0f;
    float TotalHealingDone  = 0.0f;
    int32 TargetsHit        = 0;

    /** Tags applied to targets. */
    FGameplayTagContainer AppliedStatusTags;

    FString FailureReason;
};

// ============================================================
//  JutsuExecutorBase
// ============================================================

/**
 * UJutsuExecutorBase
 *
 * Abstract base for all jutsu executors. Subclasses implement
 * the actual jutsu logic in Execute(). The JutsuManager calls
 * Execute() after hand seals and chakra cost have been validated.
 *
 * Lifecycle:
 *   OnCastBegin()  → called when hand seals start
 *   OnActivated()  → called when jutsu activates (seals complete)
 *   Execute()      → performs the jutsu effect
 *   OnSustainTick() → called each tick for sustainable jutsu
 *   OnCompleted()  → called when jutsu finishes
 *   OnInterrupted() → called if jutsu is interrupted
 *
 * Executors are instanced per-cast (created by JutsuManager,
 * destroyed when the jutsu completes or is interrupted).
 */
UCLASS(Abstract, BlueprintType, Blueprintable)
class NARUTOUSL_API UJutsuExecutorBase : public UObject
{
    GENERATED_BODY()

public:

    // ------------------------------------------------------------------
    //  Lifecycle (called by JutsuManager)
    // ------------------------------------------------------------------

    /** Called when the caster begins hand seals. */
    virtual void OnCastBegin(const FJutsuExecutionContext& Context) {}

    /** Called when the jutsu activates (seals complete, chakra consumed). */
    virtual void OnActivated(const FJutsuExecutionContext& Context) {}

    /**
     * Primary execution method. Performs the jutsu's effect.
     * Called once on activation for instant jutsu.
     * Called each tick for sustained jutsu.
     */
    UFUNCTION(BlueprintNativeEvent, Category = "Jutsu")
    FJutsuExecutionResult Execute(const FJutsuExecutionContext& Context);
    virtual FJutsuExecutionResult Execute_Implementation(const FJutsuExecutionContext& Context)
    {
        return FJutsuExecutionResult();
    }

    /** Called each tick while the jutsu is sustained. */
    virtual void OnSustainTick(const FJutsuExecutionContext& Context, float DeltaTime) {}

    /** Called when the jutsu completes normally. */
    virtual void OnCompleted(const FJutsuExecutionContext& Context) {}

    /** Called when the jutsu is interrupted before completion. */
    virtual void OnInterrupted(const FJutsuExecutionContext& Context) {}

    // ------------------------------------------------------------------
    //  Helpers available to all executors
    // ------------------------------------------------------------------

protected:

    /**
     * Resolves the final damage value for this jutsu execution.
     * Applies scaling, mastery, charge, and difficulty.
     */
    float ResolveDamage(const FJutsuExecutionContext& Context) const;

    /**
     * Resolves the final healing value for this jutsu execution.
     */
    float ResolveHealing(const FJutsuExecutionContext& Context) const;

    /**
     * Queries all actors within the jutsu's AoE radius.
     * Filters by team (enemies only, allies only, or all).
     */
    TArray<AActor*> QueryAoETargets(const FJutsuExecutionContext& Context,
                                     bool bEnemiesOnly = true) const;

    /**
     * Applies status effects from the jutsu data to a target.
     */
    void ApplyStatusEffects(const FJutsuExecutionContext& Context, AActor* Target) const;

    /**
     * Spawns the impact VFX at the specified location.
     */
    void SpawnImpactVFX(const FJutsuExecutionContext& Context,
                        const FVector& Location,
                        const FRotator& Rotation = FRotator::ZeroRotator) const;

    /**
     * Plays the impact sound at the specified location.
     */
    void PlayImpactSound(const FJutsuExecutionContext& Context, const FVector& Location) const;

    /**
     * Applies a camera shake at the caster's location.
     */
    void TriggerCameraShake(const FJutsuExecutionContext& Context) const;
};
