// Copyright (c) 2025 Maze Runner Studios. All Rights Reserved.
// Naruto: Ultimate Shinobi Legacy
// BossPhaseManager — Drives multi-phase boss encounters.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Core/Types/NarutoTypes.h"
#include "GameplayTagContainer.h"
#include "BossPhaseManager.generated.h"

class ANarutoBossBase;

// ============================================================
//  Phase Definition
// ============================================================

USTRUCT(BlueprintType)
struct NARUTOUSL_API FBossPhaseData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadOnly) FText PhaseName;
    UPROPERTY(EditAnywhere, BlueprintReadOnly) int32 PhaseIndex = 0;

    /** Health threshold (0-1) at which this phase activates. */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (ClampMin = "0", ClampMax = "1"))
    float HealthThreshold = 0.5f;

    /** Stat multipliers applied during this phase. */
    UPROPERTY(EditAnywhere, BlueprintReadOnly) float AttackMultiplier  = 1.0f;
    UPROPERTY(EditAnywhere, BlueprintReadOnly) float SpeedMultiplier   = 1.0f;
    UPROPERTY(EditAnywhere, BlueprintReadOnly) float DefenseMultiplier = 1.0f;

    /** Tags granted to the boss during this phase. */
    UPROPERTY(EditAnywhere, BlueprintReadOnly) FGameplayTagContainer GrantedTags;

    /** Jutsu unlocked during this phase. */
    UPROPERTY(EditAnywhere, BlueprintReadOnly) TArray<FGameplayTag> UnlockedJutsuTags;

    /** Animation montage played on phase transition. */
    UPROPERTY(EditAnywhere, BlueprintReadOnly) TSoftObjectPtr<UAnimMontage> TransitionMontage;

    /** VFX spawned on phase transition. */
    UPROPERTY(EditAnywhere, BlueprintReadOnly) TSoftObjectPtr<UNiagaraSystem> TransitionVFX;

    /** Music track for this phase. */
    UPROPERTY(EditAnywhere, BlueprintReadOnly) TSoftObjectPtr<USoundBase> PhaseMusic;

    /** Whether the boss is invulnerable during the transition animation. */
    UPROPERTY(EditAnywhere, BlueprintReadOnly) bool bInvulnerableDuringTransition = true;

    /** Duration of the transition animation in seconds. */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (ClampMin = "0"))
    float TransitionDuration = 2.0f;

    /** Whether this phase triggers an enrage state. */
    UPROPERTY(EditAnywhere, BlueprintReadOnly) bool bIsEnragePhase = false;
};

// ============================================================
//  Delegates
// ============================================================

DECLARE_MULTICAST_DELEGATE_ThreeParams(FOnBossPhaseChanged,
    ANarutoBossBase* /*Boss*/,
    int32 /*OldPhase*/,
    int32 /*NewPhase*/);

DECLARE_MULTICAST_DELEGATE_OneParam(FOnBossEnraged, ANarutoBossBase* /*Boss*/);

// ============================================================
//  BossPhaseManager
// ============================================================

/**
 * UBossPhaseManager
 *
 * Attached to each boss. Monitors health percentage and triggers
 * phase transitions when thresholds are crossed.
 *
 * Phase transition sequence:
 *   1. Detect health threshold crossed
 *   2. Grant invulnerability (if configured)
 *   3. Play transition montage + VFX
 *   4. Apply new stat multipliers
 *   5. Unlock new jutsu
 *   6. Switch music track
 *   7. Remove invulnerability
 *   8. Notify AIDirector and BehaviorTree
 */
UCLASS(BlueprintType)
class NARUTOUSL_API UBossPhaseManager : public UObject
{
    GENERATED_BODY()

public:

    void Initialize(ANarutoBossBase* InBoss, const TArray<FBossPhaseData>& InPhases);

    /** Called each frame by the boss. Checks for phase transitions. */
    void Tick(float DeltaTime);

    /** Called when the boss takes damage. Checks health thresholds. */
    void OnHealthChanged(float NewHealthPercent);

    UFUNCTION(BlueprintPure, Category = "Boss|Phase")
    int32 GetCurrentPhaseIndex() const { return CurrentPhaseIndex; }

    UFUNCTION(BlueprintPure, Category = "Boss|Phase")
    const FBossPhaseData* GetCurrentPhase() const;

    UFUNCTION(BlueprintPure, Category = "Boss|Phase")
    bool IsInTransition() const { return bInTransition; }

    UFUNCTION(BlueprintPure, Category = "Boss|Phase")
    bool IsEnraged() const { return bIsEnraged; }

    FOnBossPhaseChanged OnPhaseChanged;
    FOnBossEnraged      OnEnraged;

private:

    void TriggerPhaseTransition(int32 NewPhaseIndex);
    void CompleteTransition();
    void ApplyPhaseStats(const FBossPhaseData& Phase);

    TWeakObjectPtr<ANarutoBossBase> Boss;
    TArray<FBossPhaseData>          Phases;
    int32                           CurrentPhaseIndex = 0;
    bool                            bInTransition     = false;
    bool                            bIsEnraged        = false;
    float                           TransitionTimer   = 0.0f;
    int32                           PendingPhaseIndex = -1;
};
