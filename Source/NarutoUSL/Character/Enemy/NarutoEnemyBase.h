// Copyright (c) 2025 Maze Runner Studios. All Rights Reserved.
// Naruto: Ultimate Shinobi Legacy
// NarutoEnemyBase — Base class for all AI-controlled enemy characters.

#pragma once

#include "CoreMinimal.h"
#include "Character/Base/NarutoCharacterBase.h"
#include "NarutoEnemyBase.generated.h"

class UAIDirector;
class UBehaviorTree;
class UAIPerceptionComponent;
class UBlackboardComponent;

UENUM(BlueprintType)
enum class EEnemyState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Patrolling  UMETA(DisplayName = "Patrolling"),
    Investigating UMETA(DisplayName = "Investigating"),
    Engaging    UMETA(DisplayName = "Engaging"),
    Flanking    UMETA(DisplayName = "Flanking"),
    Retreating  UMETA(DisplayName = "Retreating"),
    Healing     UMETA(DisplayName = "Healing"),
    Dead        UMETA(DisplayName = "Dead"),
};

/**
 * ANarutoEnemyBase
 *
 * Base class for all AI-driven enemies. Bridges the character
 * component system with UE5's AIController / BehaviorTree pipeline.
 *
 * The BehaviorTree drives high-level decisions.
 * CombatManager drives frame-level combat.
 * AIDirector drives group coordination.
 */
UCLASS()
class NARUTOUSL_API ANarutoEnemyBase : public ANarutoCharacterBase
{
    GENERATED_BODY()

public:

    ANarutoEnemyBase(const FObjectInitializer& ObjectInitializer);

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // ------------------------------------------------------------------
    //  AI State
    // ------------------------------------------------------------------

    UFUNCTION(BlueprintCallable, Category = "AI")
    void SetEnemyState(EEnemyState NewState);

    UFUNCTION(BlueprintPure, Category = "AI")
    EEnemyState GetEnemyState() const { return CurrentEnemyState; }

    // ------------------------------------------------------------------
    //  Combat AI
    // ------------------------------------------------------------------

    /**
     * Called by the BehaviorTree to select and execute the best jutsu.
     * Queries AIDirector to avoid duplicating jutsu already in use.
     */
    UFUNCTION(BlueprintCallable, Category = "AI|Combat")
    bool TryExecuteBestJutsu(AActor* Target);

    /**
     * Called by the BehaviorTree when this enemy should perform a basic attack.
     */
    UFUNCTION(BlueprintCallable, Category = "AI|Combat")
    void ExecuteBasicAttack(AActor* Target);

    // ------------------------------------------------------------------
    //  Loot
    // ------------------------------------------------------------------

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Loot")
    TArray<FGameplayTag> LootTable;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Loot",
        meta = (ClampMin = "0"))
    int64 RyoDropMin = 10;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Loot",
        meta = (ClampMin = "0"))
    int64 RyoDropMax = 50;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Loot",
        meta = (ClampMin = "0"))
    float XPReward = 100.0f;

    // ------------------------------------------------------------------
    //  Overrides
    // ------------------------------------------------------------------

    virtual void OnDefeated_Implementation(ICombatant* Killer) override;
    virtual void OnCombatEntered_Implementation() override;
    virtual void OnCombatExited_Implementation() override;

protected:

    void RegisterWithAIDirector();
    void UnregisterFromAIDirector();
    void SpawnLoot();

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI")
    TObjectPtr<UBehaviorTree> BehaviorTree;

    UPROPERTY(BlueprintReadOnly, Category = "AI")
    EEnemyState CurrentEnemyState = EEnemyState::Idle;

    /** Patrol path points (world-space). */
    UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "AI")
    TArray<FVector> PatrolPoints;

    int32 CurrentPatrolIndex = 0;
};
