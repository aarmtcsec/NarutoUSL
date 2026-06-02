// Copyright (c) 2025 Maze Runner Studios. All Rights Reserved.
// Naruto: Ultimate Shinobi Legacy
// AIDirector — Orchestrates all AI in the world. Manages threat, coordination,
// encounter budgets, and adaptive difficulty.

#pragma once

#include "CoreMinimal.h"
#include "Core/Subsystems/NarutoSubsystem.h"
#include "Core/Types/NarutoTypes.h"
#include "AIDirector.generated.h"

class ANarutoCharacterBase;
class ANarutoEnemyBase;
class UNarutoEventBus;

// ============================================================
//  Threat Entry
// ============================================================

USTRUCT()
struct NARUTOUSL_API FThreatEntry
{
    GENERATED_BODY()

    TWeakObjectPtr<AActor> ThreatSource;
    float                  ThreatValue    = 0.0f;
    float                  LastUpdateTime = 0.0f;

    bool IsValid() const { return ThreatSource.IsValid() && ThreatValue > 0.0f; }
};

// ============================================================
//  Enemy Record
// ============================================================

USTRUCT()
struct NARUTOUSL_API FEnemyRecord
{
    GENERATED_BODY()

    TWeakObjectPtr<ANarutoEnemyBase> Enemy;
    TArray<FThreatEntry>             ThreatList;
    bool                             bIsEngaged    = false;
    bool                             bIsCoordinating = false;
    FVector                          LastKnownPlayerPos = FVector::ZeroVector;

    AActor* GetHighestThreatTarget() const;
    float   GetTotalThreat() const;
    void    AddThreat(AActor* Source, float Amount);
    void    DecayThreat(float DecayRate, float DeltaTime);
};

// ============================================================
//  Encounter Budget
// ============================================================

USTRUCT()
struct NARUTOUSL_API FEncounterBudget
{
    GENERATED_BODY()

    int32 MaxSimultaneousAttackers  = 3;
    int32 MaxSimultaneousFlank      = 2;
    int32 MaxSimultaneousRanged     = 4;
    int32 CurrentAttackers          = 0;
    int32 CurrentFlankers           = 0;
    int32 CurrentRanged             = 0;

    bool CanAddAttacker()  const { return CurrentAttackers < MaxSimultaneousAttackers; }
    bool CanAddFlanker()   const { return CurrentFlankers  < MaxSimultaneousFlank; }
    bool CanAddRanged()    const { return CurrentRanged    < MaxSimultaneousRanged; }
};

// ============================================================
//  AIDirector
// ============================================================

/**
 * UAIDirector
 *
 * The global AI orchestrator. Responsibilities:
 *
 *   Threat Management:
 *     - Maintains a threat table per enemy
 *     - Decays threat over time
 *     - Broadcasts threat changes to individual AI controllers
 *
 *   Encounter Budgeting:
 *     - Limits simultaneous attackers to prevent overwhelming the player
 *     - Rotates attackers in/out based on threat and position
 *     - Scales budget with difficulty setting
 *
 *   Coordination:
 *     - Signals flanking maneuvers to eligible enemies
 *     - Coordinates retreat/heal behaviors
 *     - Prevents all enemies from using the same jutsu simultaneously
 *
 *   Adaptive Difficulty:
 *     - Tracks player performance metrics (damage taken, deaths, combo length)
 *     - Adjusts enemy aggression, reaction time, and jutsu frequency
 *     - Rubber-bands difficulty within configured bounds
 */
UCLASS(NotBlueprintable)
class NARUTOUSL_API UAIDirector : public UNarutoSubsystem
{
    GENERATED_BODY()

public:

    UAIDirector();

    void Initialize(UNarutoEventBus* InEventBus);
    virtual void Tick(float DeltaTime) override;
    virtual void Shutdown() override;
    virtual TMap<FString, FString> GetDebugInfo() const override;

    // ------------------------------------------------------------------
    //  Enemy Registration
    // ------------------------------------------------------------------

    void RegisterEnemy(ANarutoEnemyBase* Enemy);
    void UnregisterEnemy(ANarutoEnemyBase* Enemy);

    // ------------------------------------------------------------------
    //  Threat API
    // ------------------------------------------------------------------

    void AddThreat(ANarutoEnemyBase* Enemy, AActor* ThreatSource, float Amount);
    void ClearThreat(ANarutoEnemyBase* Enemy, AActor* ThreatSource);
    AActor* GetHighestThreatTarget(ANarutoEnemyBase* Enemy) const;
    float   GetThreatValue(ANarutoEnemyBase* Enemy, AActor* ThreatSource) const;

    // ------------------------------------------------------------------
    //  Encounter Budget
    // ------------------------------------------------------------------

    bool RequestAttackSlot(ANarutoEnemyBase* Enemy);
    void ReleaseAttackSlot(ANarutoEnemyBase* Enemy);
    bool RequestFlankSlot(ANarutoEnemyBase* Enemy);
    void ReleaseFlankSlot(ANarutoEnemyBase* Enemy);

    // ------------------------------------------------------------------
    //  Coordination
    // ------------------------------------------------------------------

    /** Signals all enemies within radius to flank the player. */
    void SignalFlank(FVector PlayerLocation, float Radius);

    /** Signals nearby enemies to retreat and heal. */
    void SignalRetreat(ANarutoEnemyBase* Initiator, float Radius);

    /** Returns true if the specified jutsu is already being used by another enemy. */
    bool IsJutsuInUse(FGameplayTag JutsuTag) const;

    void RegisterJutsuInUse(ANarutoEnemyBase* Enemy, FGameplayTag JutsuTag);
    void UnregisterJutsuInUse(ANarutoEnemyBase* Enemy);

    // ------------------------------------------------------------------
    //  Adaptive Difficulty
    // ------------------------------------------------------------------

    void RecordPlayerDamageTaken(float Amount);
    void RecordPlayerDeath();
    void RecordPlayerComboLength(int32 ComboLength);

    float GetAdaptiveDifficultyScale() const { return AdaptiveDifficultyScale; }

    // ------------------------------------------------------------------
    //  Configuration
    // ------------------------------------------------------------------

    UPROPERTY(EditAnywhere, Category = "AI|Difficulty", meta = (ClampMin = "0.5", ClampMax = "2.0"))
    float MinDifficultyScale = 0.7f;

    UPROPERTY(EditAnywhere, Category = "AI|Difficulty", meta = (ClampMin = "0.5", ClampMax = "2.0"))
    float MaxDifficultyScale = 1.5f;

private:

    void TickThreatDecay(float DeltaTime);
    void TickEncounterBudget(float DeltaTime);
    void TickAdaptiveDifficulty(float DeltaTime);
    void UpdateEncounterBudgetScaling();

    FEnemyRecord* FindRecord(ANarutoEnemyBase* Enemy);

    TArray<FEnemyRecord>                    EnemyRecords;
    FEncounterBudget                        EncounterBudget;
    TWeakObjectPtr<UNarutoEventBus>         EventBus;

    // Jutsu coordination
    TMap<ANarutoEnemyBase*, FGameplayTag>   ActiveEnemyJutsu;

    // Adaptive difficulty
    float AdaptiveDifficultyScale   = 1.0f;
    float PlayerDamageTakenRecent   = 0.0f;
    int32 PlayerDeathCount          = 0;
    float PlayerAvgComboLength      = 0.0f;
    float DifficultyAdjustTimer     = 0.0f;

    static constexpr float DifficultyAdjustInterval = 30.0f;
    static constexpr float ThreatDecayRate           = 2.0f;
};
