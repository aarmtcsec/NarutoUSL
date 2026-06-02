// Copyright (c) 2025 Maze Runner Studios. All Rights Reserved.
// Naruto: Ultimate Shinobi Legacy
// CombatComponent — Per-character combat state machine and frame data tracker.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Core/Types/NarutoTypes.h"
#include "CombatComponent.generated.h"

class UHealthComponent;
class UChakraComponent;

// ============================================================
//  Delegates
// ============================================================

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam (FOnCombatStateEntered,  ECombatPhase, Phase);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam (FOnCombatStateExited,   ECombatPhase, Phase);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnComboCountChanged,   int32, NewCount, int32, MaxCombo);
DECLARE_DYNAMIC_MULTICAST_DELEGATE          (FOnComboReset);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam (FOnSubstitutionUsed,    int32, RemainingSubstitutions);
DECLARE_DYNAMIC_MULTICAST_DELEGATE          (FOnAwakeningActivated);
DECLARE_DYNAMIC_MULTICAST_DELEGATE          (FOnAwakeningDeactivated);

// ============================================================
//  CombatComponent
// ============================================================

/**
 * UCombatComponent
 *
 * Tracks per-character combat state: current phase, active flags,
 * combo counter, substitution charges, guard state, and awakening.
 * The CombatManager reads and writes this component to drive the
 * combat simulation. Characters do not self-manage combat state —
 * the CombatManager is authoritative.
 *
 * Frame simulation:
 *   The component tracks the current frame within an active attack.
 *   At 60 FPS, one frame = 1/60 second. The CombatManager advances
 *   CurrentAttackFrame each tick and evaluates hitbox windows,
 *   cancel windows, and invulnerability windows against FrameData.
 */
UCLASS(ClassGroup = "NarutoUSL|Character", meta = (BlueprintSpawnableComponent))
class NARUTOUSL_API UCombatComponent : public UActorComponent
{
    GENERATED_BODY()

public:

    UCombatComponent();

    virtual void BeginPlay() override;

    // ------------------------------------------------------------------
    //  Initialization
    // ------------------------------------------------------------------

    void InitializeCombat(int32 InMaxSubstitutions, bool bHasAwakening);

    // ------------------------------------------------------------------
    //  Phase / State Queries
    // ------------------------------------------------------------------

    UFUNCTION(BlueprintPure, Category = "Combat")
    ECombatPhase GetCurrentPhase() const { return CurrentPhase; }

    UFUNCTION(BlueprintPure, Category = "Combat")
    ECombatFlags GetCombatFlags() const { return ActiveFlags; }

    UFUNCTION(BlueprintPure, Category = "Combat")
    bool HasFlag(ECombatFlags Flag) const;

    UFUNCTION(BlueprintPure, Category = "Combat")
    bool IsInCombat() const { return bIsInCombat; }

    UFUNCTION(BlueprintPure, Category = "Combat")
    ECombatStance GetStance() const { return CurrentStance; }

    UFUNCTION(BlueprintPure, Category = "Combat")
    int32 GetCurrentAttackFrame() const { return CurrentAttackFrame; }

    UFUNCTION(BlueprintPure, Category = "Combat")
    const FAttackFrameData& GetActiveFrameData() const { return ActiveFrameData; }

    // ------------------------------------------------------------------
    //  Phase Control (called by CombatManager)
    // ------------------------------------------------------------------

    void SetPhase(ECombatPhase NewPhase);
    void SetStance(ECombatStance NewStance);
    void SetInCombat(bool bActive);

    void AddFlag(ECombatFlags Flag);
    void RemoveFlag(ECombatFlags Flag);
    void ClearAllFlags();

    // ------------------------------------------------------------------
    //  Frame Data
    // ------------------------------------------------------------------

    /**
     * Begins tracking a new attack using the provided frame data.
     * Resets CurrentAttackFrame to 0.
     */
    void BeginAttack(const FAttackFrameData& FrameData);

    /**
     * Advances the frame counter by one. Called by CombatManager each tick.
     * Returns true if the attack is still active (not yet in recovery end).
     */
    bool AdvanceAttackFrame();

    /** Clears the active attack and resets frame tracking. */
    void EndAttack();

    UFUNCTION(BlueprintPure, Category = "Combat|FrameData")
    bool IsInStartupFrames() const;

    UFUNCTION(BlueprintPure, Category = "Combat|FrameData")
    bool IsInActiveFrames() const;

    UFUNCTION(BlueprintPure, Category = "Combat|FrameData")
    bool IsInRecoveryFrames() const;

    UFUNCTION(BlueprintPure, Category = "Combat|FrameData")
    bool IsInCancelWindow() const;

    UFUNCTION(BlueprintPure, Category = "Combat|FrameData")
    bool IsFrameInvulnerable() const;

    UFUNCTION(BlueprintPure, Category = "Combat|FrameData")
    bool HasFrameArmor() const;

    // ------------------------------------------------------------------
    //  Combo System
    // ------------------------------------------------------------------

    UFUNCTION(BlueprintPure, Category = "Combat|Combo")
    int32 GetComboCount() const { return ComboCount; }

    UFUNCTION(BlueprintPure, Category = "Combat|Combo")
    int32 GetMaxComboCount() const { return MaxComboCount; }

    UFUNCTION(BlueprintPure, Category = "Combat|Combo")
    float GetComboTimer() const { return ComboTimer; }

    void IncrementCombo();
    void ResetCombo();
    void SetMaxComboCount(int32 NewMax);

    /** Ticks the combo timer. Called by CombatManager. Resets combo on expiry. */
    void TickComboTimer(float DeltaTime);

    // ------------------------------------------------------------------
    //  Substitution
    // ------------------------------------------------------------------

    UFUNCTION(BlueprintPure, Category = "Combat|Substitution")
    int32 GetSubstitutionCharges() const { return SubstitutionCharges; }

    UFUNCTION(BlueprintPure, Category = "Combat|Substitution")
    int32 GetMaxSubstitutionCharges() const { return MaxSubstitutionCharges; }

    UFUNCTION(BlueprintPure, Category = "Combat|Substitution")
    bool CanSubstitute() const;

    UFUNCTION(BlueprintPure, Category = "Combat|Substitution")
    float GetSubstitutionCooldownRemaining() const { return SubstitutionCooldownRemaining; }

    bool ConsumeSubstitution();
    void TickSubstitutionCooldown(float DeltaTime);
    void RestoreSubstitutionCharge();

    // ------------------------------------------------------------------
    //  Guard / Parry
    // ------------------------------------------------------------------

    UFUNCTION(BlueprintPure, Category = "Combat|Guard")
    bool IsGuarding() const { return HasFlag(ECombatFlags::CanBlock); }

    UFUNCTION(BlueprintPure, Category = "Combat|Guard")
    bool IsGuardBroken() const { return HasFlag(ECombatFlags::IsGuardBroken); }

    UFUNCTION(BlueprintPure, Category = "Combat|Guard")
    float GetGuardHealth() const { return GuardHealth; }

    void DamageGuard(float Amount);
    void RestoreGuard(float Amount);
    void BreakGuard();
    void TickGuardRecovery(float DeltaTime);

    // ------------------------------------------------------------------
    //  Hitstun
    // ------------------------------------------------------------------

    UFUNCTION(BlueprintPure, Category = "Combat|Hitstun")
    bool IsInHitstun() const { return HitstunFramesRemaining > 0; }

    UFUNCTION(BlueprintPure, Category = "Combat|Hitstun")
    int32 GetHitstunFramesRemaining() const { return HitstunFramesRemaining; }

    void ApplyHitstun(int32 Frames);
    void TickHitstun();

    // ------------------------------------------------------------------
    //  Awakening
    // ------------------------------------------------------------------

    UFUNCTION(BlueprintPure, Category = "Combat|Awakening")
    bool IsAwakened() const { return bIsAwakened; }

    UFUNCTION(BlueprintPure, Category = "Combat|Awakening")
    bool CanAwaken() const;

    UFUNCTION(BlueprintPure, Category = "Combat|Awakening")
    int32 GetCurrentAwakeningIndex() const { return CurrentAwakeningIndex; }

    void ActivateAwakening(int32 AwakeningIndex);
    void DeactivateAwakening();

    // ------------------------------------------------------------------
    //  Events
    // ------------------------------------------------------------------

    UPROPERTY(BlueprintAssignable, Category = "Combat|Events")
    FOnCombatStateEntered OnCombatStateEntered;

    UPROPERTY(BlueprintAssignable, Category = "Combat|Events")
    FOnCombatStateExited OnCombatStateExited;

    UPROPERTY(BlueprintAssignable, Category = "Combat|Events")
    FOnComboCountChanged OnComboCountChanged;

    UPROPERTY(BlueprintAssignable, Category = "Combat|Events")
    FOnComboReset OnComboReset;

    UPROPERTY(BlueprintAssignable, Category = "Combat|Events")
    FOnSubstitutionUsed OnSubstitutionUsed;

    UPROPERTY(BlueprintAssignable, Category = "Combat|Events")
    FOnAwakeningActivated OnAwakeningActivated;

    UPROPERTY(BlueprintAssignable, Category = "Combat|Events")
    FOnAwakeningDeactivated OnAwakeningDeactivated;

protected:

    // ------------------------------------------------------------------
    //  State
    // ------------------------------------------------------------------

    UPROPERTY(BlueprintReadOnly, Category = "Combat")
    ECombatPhase CurrentPhase = ECombatPhase::Startup;

    UPROPERTY(BlueprintReadOnly, Category = "Combat")
    ECombatStance CurrentStance = ECombatStance::Neutral;

    UPROPERTY(BlueprintReadOnly, Category = "Combat")
    ECombatFlags ActiveFlags = ECombatFlags::None;

    UPROPERTY(BlueprintReadOnly, Category = "Combat")
    bool bIsInCombat = false;

    // Frame tracking
    FAttackFrameData ActiveFrameData;
    int32 CurrentAttackFrame = 0;
    bool bAttackActive = false;

    // Combo
    UPROPERTY(BlueprintReadOnly, Category = "Combat|Combo")
    int32 ComboCount = 0;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat|Combo", meta = (ClampMin = "1"))
    int32 MaxComboCount = 99;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat|Combo", meta = (ClampMin = "0.1"))
    float ComboResetTime = 2.0f;

    float ComboTimer = 0.0f;

    // Substitution
    UPROPERTY(BlueprintReadOnly, Category = "Combat|Substitution")
    int32 SubstitutionCharges = 4;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat|Substitution", meta = (ClampMin = "1"))
    int32 MaxSubstitutionCharges = 4;

    float SubstitutionCooldownRemaining = 0.0f;

    // Guard
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat|Guard", meta = (ClampMin = "1"))
    float MaxGuardHealth = 100.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Combat|Guard")
    float GuardHealth = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat|Guard", meta = (ClampMin = "0"))
    float GuardRegenRate = 10.0f;

    // Hitstun
    int32 HitstunFramesRemaining = 0;

    // Awakening
    UPROPERTY(BlueprintReadOnly, Category = "Combat|Awakening")
    bool bIsAwakened = false;

    UPROPERTY(BlueprintReadOnly, Category = "Combat|Awakening")
    int32 CurrentAwakeningIndex = -1;

    bool bHasAwakening = false;
};
