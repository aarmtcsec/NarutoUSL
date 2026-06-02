// Copyright (c) 2025 Maze Runner Studios. All Rights Reserved.
// Naruto: Ultimate Shinobi Legacy
// NarutoBossBase — Base class for all 75+ boss characters.

#pragma once

#include "CoreMinimal.h"
#include "Character/Enemy/NarutoEnemyBase.h"
#include "AI/Boss/BossPhaseManager.h"
#include "NarutoBossBase.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnBossDefeated, ANarutoBossBase*, Boss);

/**
 * ANarutoBossBase
 *
 * Extends NarutoEnemyBase with boss-specific systems:
 *   - Multi-phase management via BossPhaseManager
 *   - Unique mechanic hooks (subclasses override)
 *   - Boss health bar UI integration
 *   - Cinematic intro/outro sequences
 *   - Achievement and loot table integration
 *   - Arena boundary enforcement
 *
 * Each of the 75+ bosses subclasses this and overrides:
 *   - InitializePhases() — define phase thresholds and stats
 *   - ExecuteUniqueMechanic() — boss-specific special attacks
 *   - OnPhaseTransition() — phase-specific behavior changes
 */
UCLASS(Abstract)
class NARUTOUSL_API ANarutoBossBase : public ANarutoEnemyBase
{
    GENERATED_BODY()

public:

    ANarutoBossBase(const FObjectInitializer& ObjectInitializer);

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // ------------------------------------------------------------------
    //  Phase System
    // ------------------------------------------------------------------

    UFUNCTION(BlueprintPure, Category = "Boss|Phase")
    UBossPhaseManager* GetPhaseManager() const { return PhaseManager; }

    UFUNCTION(BlueprintPure, Category = "Boss|Phase")
    int32 GetCurrentPhase() const;

    UFUNCTION(BlueprintPure, Category = "Boss|Phase")
    bool IsEnraged() const;

    // ------------------------------------------------------------------
    //  Unique Mechanics
    // ------------------------------------------------------------------

    /**
     * Called by the BehaviorTree when the boss should execute its
     * signature mechanic. Subclasses implement the actual logic.
     */
    UFUNCTION(BlueprintCallable, Category = "Boss|Mechanics")
    virtual void ExecuteUniqueMechanic(int32 MechanicIndex, AActor* Target);

    /**
     * Returns true if the unique mechanic at the given index is
     * currently available (cooldown expired, phase requirement met, etc.)
     */
    UFUNCTION(BlueprintPure, Category = "Boss|Mechanics")
    virtual bool CanExecuteUniqueMechanic(int32 MechanicIndex) const;

    // ------------------------------------------------------------------
    //  Arena
    // ------------------------------------------------------------------

    UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "Boss|Arena")
    FVector ArenaCenter = FVector::ZeroVector;

    UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "Boss|Arena",
        meta = (ClampMin = "500"))
    float ArenaRadius = 3000.0f;

    // ------------------------------------------------------------------
    //  Rewards
    // ------------------------------------------------------------------

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Boss|Rewards")
    TArray<FGameplayTag> GuaranteedLoot;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Boss|Rewards")
    TArray<FGameplayTag> AchievementTags;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Boss|Rewards",
        meta = (ClampMin = "0"))
    float BossXPReward = 5000.0f;

    // ------------------------------------------------------------------
    //  Events
    // ------------------------------------------------------------------

    UPROPERTY(BlueprintAssignable, Category = "Boss|Events")
    FOnBossDefeated OnBossDefeated;

    // ------------------------------------------------------------------
    //  Overrides
    // ------------------------------------------------------------------

    virtual void OnDefeated_Implementation(ICombatant* Killer) override;

protected:

    /**
     * Called during BeginPlay. Subclasses populate PhaseDefinitions
     * and call Super::InitializePhases() to register them.
     */
    virtual void InitializePhases();

    virtual void OnPhaseTransition(int32 OldPhase, int32 NewPhase) {}
    virtual void OnEnraged() {}

    UFUNCTION()
    void HandlePhaseChanged(ANarutoBossBase* Boss, int32 OldPhase, int32 NewPhase);

    UFUNCTION()
    void HandleEnraged(ANarutoBossBase* Boss);

    UFUNCTION()
    void HandleHealthChanged(float NewHealth, float MaxHealth);

    UPROPERTY()
    TObjectPtr<UBossPhaseManager> PhaseManager;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Boss|Phases")
    TArray<FBossPhaseData> PhaseDefinitions;

    TArray<float> MechanicCooldowns;
};
