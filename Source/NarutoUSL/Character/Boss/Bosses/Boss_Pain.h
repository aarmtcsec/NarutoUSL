// Copyright (c) 2025 Maze Runner Studios. All Rights Reserved.
// Naruto: Ultimate Shinobi Legacy
// Boss_Pain — Nagato / Six Paths of Pain boss encounter.
//
// Phases:
//   Phase 0 (100-60% HP): All six Paths active. Deva Path leads.
//   Phase 1 (60-30% HP):  Three Paths defeated. Gravitational mechanics introduced.
//   Phase 2 (30-0% HP):   Deva Path alone. Shinra Tensei on cooldown, Chibaku Tensei threat.
//
// Unique Mechanics:
//   0 — Shinra Tensei (massive repulsion AoE, 5s cooldown)
//   1 — Chibaku Tensei (gravitational sphere, traps player)
//   2 — Summon Path (calls giant summons)
//   3 — Preta Path Chakra Absorption (nullifies jutsu)
//   4 — Naraka Path Revival (revives defeated Paths at 20% HP)

#pragma once

#include "CoreMinimal.h"
#include "Character/Boss/NarutoBossBase.h"
#include "Boss_Pain.generated.h"

UCLASS()
class NARUTOUSL_API ABoss_Pain : public ANarutoBossBase
{
    GENERATED_BODY()

public:

    ABoss_Pain(const FObjectInitializer& ObjectInitializer);

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    virtual void ExecuteUniqueMechanic(int32 MechanicIndex, AActor* Target) override;
    virtual bool CanExecuteUniqueMechanic(int32 MechanicIndex) const override;

protected:

    virtual void InitializePhases() override;
    virtual void OnPhaseTransition(int32 OldPhase, int32 NewPhase) override;
    virtual void OnEnraged() override;

private:

    // ------------------------------------------------------------------
    //  Unique Mechanics
    // ------------------------------------------------------------------

    /** Mechanic 0: Shinra Tensei — massive repulsion blast. */
    void ExecuteShinraTensei(AActor* Target);

    /** Mechanic 1: Chibaku Tensei — gravitational sphere trap. */
    void ExecuteChibakuTensei(AActor* Target);

    /** Mechanic 2: Summon Path — call a giant boss summon. */
    void ExecuteSummonPath();

    /** Mechanic 3: Preta Path — absorb player's next jutsu. */
    void ActivatePretaPath();

    /** Mechanic 4: Naraka Path — attempt to revive a defeated path. */
    void ExecuteNarakaRevival();

    // ------------------------------------------------------------------
    //  Six Paths Management
    // ------------------------------------------------------------------

    /** Spawns the six Paths as subordinate enemies in the arena. */
    void SpawnSixPaths();

    /** Called when a Path body is defeated. */
    UFUNCTION()
    void OnPathDefeated(ANarutoCharacterBase* DefeatedPath,
                        const FNarutoDamageEvent& KillingBlow);

    // ------------------------------------------------------------------
    //  State
    // ------------------------------------------------------------------

    UPROPERTY()
    TArray<TObjectPtr<ANarutoEnemyBase>> ActivePaths;

    int32 DefeatedPathCount   = 0;
    bool  bPretaPathActive    = false;
    bool  bChibakuActive      = false;

    // Mechanic cooldowns (seconds)
    static constexpr float ShinraTenseiCooldown   = 12.0f;
    static constexpr float ChibakuTenseiCooldown  = 45.0f;
    static constexpr float SummonPathCooldown      = 30.0f;
    static constexpr float PretaPathCooldown       = 20.0f;
    static constexpr float NarakaRevivalCooldown   = 35.0f;

    // Mechanic cooldown indices map to MechanicCooldowns array in base class
    static constexpr int32 IDX_SHINRA    = 0;
    static constexpr int32 IDX_CHIBAKU   = 1;
    static constexpr int32 IDX_SUMMON    = 2;
    static constexpr int32 IDX_PRETA     = 3;
    static constexpr int32 IDX_NARAKA    = 4;

    // Arena-specific Pain spawn points
    UPROPERTY(EditInstanceOnly, Category = "Boss|Pain")
    TArray<FVector> PathSpawnPoints;
};
