// Copyright (c) 2025 Maze Runner Studios. All Rights Reserved.

#include "Character/Components/CombatComponent.h"
#include "Core/Settings/NarutoGameSettings.h"
#include "NarutoUSL.h"

UCombatComponent::UCombatComponent()
{
    PrimaryComponentTick.bCanEverTick = false; // Driven by CombatManager
}

void UCombatComponent::BeginPlay()
{
    Super::BeginPlay();
    GuardHealth = MaxGuardHealth;
}

void UCombatComponent::InitializeCombat(int32 InMaxSubstitutions, bool bInHasAwakening)
{
    MaxSubstitutionCharges = FMath::Max(1, InMaxSubstitutions);
    SubstitutionCharges    = MaxSubstitutionCharges;
    bHasAwakening          = bInHasAwakening;
    GuardHealth            = MaxGuardHealth;
}

// ============================================================
//  Phase / Flag Control
// ============================================================

bool UCombatComponent::HasFlag(ECombatFlags Flag) const
{
    return EnumHasAnyFlags(ActiveFlags, Flag);
}

void UCombatComponent::SetPhase(ECombatPhase NewPhase)
{
    if (CurrentPhase == NewPhase) return;

    OnCombatStateExited.Broadcast(CurrentPhase);
    CurrentPhase = NewPhase;
    OnCombatStateEntered.Broadcast(NewPhase);
}

void UCombatComponent::SetStance(ECombatStance NewStance)
{
    CurrentStance = NewStance;
}

void UCombatComponent::SetInCombat(bool bActive)
{
    bIsInCombat = bActive;
}

void UCombatComponent::AddFlag(ECombatFlags Flag)
{
    ActiveFlags |= Flag;
}

void UCombatComponent::RemoveFlag(ECombatFlags Flag)
{
    ActiveFlags &= ~Flag;
}

void UCombatComponent::ClearAllFlags()
{
    ActiveFlags = ECombatFlags::None;
}

// ============================================================
//  Frame Data
// ============================================================

void UCombatComponent::BeginAttack(const FAttackFrameData& FrameData)
{
    ActiveFrameData    = FrameData;
    CurrentAttackFrame = 0;
    bAttackActive      = true;

    SetPhase(ECombatPhase::Startup);

    // Apply invulnerability if the frame data specifies it
    if (FrameData.InvulnerabilityStart == 0 && FrameData.InvulnerabilityEnd > 0)
    {
        AddFlag(ECombatFlags::IsInvulnerable);
    }
}

bool UCombatComponent::AdvanceAttackFrame()
{
    if (!bAttackActive) return false;

    ++CurrentAttackFrame;

    // Update phase based on frame position
    if (ActiveFrameData.IsInStartup(CurrentAttackFrame))
    {
        SetPhase(ECombatPhase::Startup);
    }
    else if (ActiveFrameData.IsActive(CurrentAttackFrame))
    {
        SetPhase(ECombatPhase::Active);
    }
    else
    {
        SetPhase(ECombatPhase::Recovery);
    }

    // Update invulnerability flag
    if (ActiveFrameData.IsInvulnerable(CurrentAttackFrame))
    {
        AddFlag(ECombatFlags::IsInvulnerable);
    }
    else
    {
        RemoveFlag(ECombatFlags::IsInvulnerable);
    }

    // Update armor flag
    if (ActiveFrameData.HasArmor(CurrentAttackFrame))
    {
        AddFlag(ECombatFlags::HasSuperArmor);
    }
    else
    {
        RemoveFlag(ECombatFlags::HasSuperArmor);
    }

    // Attack ends when recovery is complete
    const bool bStillActive = CurrentAttackFrame < ActiveFrameData.TotalFrames();
    if (!bStillActive)
    {
        EndAttack();
    }

    return bStillActive;
}

void UCombatComponent::EndAttack()
{
    bAttackActive      = false;
    CurrentAttackFrame = 0;
    RemoveFlag(ECombatFlags::IsInvulnerable);
    RemoveFlag(ECombatFlags::HasSuperArmor);
    SetPhase(ECombatPhase::Recovery);
}

bool UCombatComponent::IsInStartupFrames() const
{
    return bAttackActive && ActiveFrameData.IsInStartup(CurrentAttackFrame);
}

bool UCombatComponent::IsInActiveFrames() const
{
    return bAttackActive && ActiveFrameData.IsActive(CurrentAttackFrame);
}

bool UCombatComponent::IsInRecoveryFrames() const
{
    return bAttackActive && ActiveFrameData.IsInRecovery(CurrentAttackFrame);
}

bool UCombatComponent::IsInCancelWindow() const
{
    return bAttackActive && ActiveFrameData.IsInCancelWindow(CurrentAttackFrame);
}

bool UCombatComponent::IsFrameInvulnerable() const
{
    return HasFlag(ECombatFlags::IsInvulnerable);
}

bool UCombatComponent::HasFrameArmor() const
{
    return HasFlag(ECombatFlags::HasSuperArmor);
}

// ============================================================
//  Combo
// ============================================================

void UCombatComponent::IncrementCombo()
{
    ComboCount = FMath::Min(ComboCount + 1, MaxComboCount);
    ComboTimer = ComboResetTime;
    OnComboCountChanged.Broadcast(ComboCount, MaxComboCount);
}

void UCombatComponent::ResetCombo()
{
    if (ComboCount == 0) return;
    ComboCount = 0;
    ComboTimer = 0.0f;
    OnComboReset.Broadcast();
    OnComboCountChanged.Broadcast(0, MaxComboCount);
}

void UCombatComponent::SetMaxComboCount(int32 NewMax)
{
    MaxComboCount = FMath::Max(1, NewMax);
}

void UCombatComponent::TickComboTimer(float DeltaTime)
{
    if (ComboCount == 0 || ComboTimer <= 0.0f) return;

    ComboTimer -= DeltaTime;
    if (ComboTimer <= 0.0f)
    {
        ResetCombo();
    }
}

// ============================================================
//  Substitution
// ============================================================

bool UCombatComponent::CanSubstitute() const
{
    return SubstitutionCharges > 0 && SubstitutionCooldownRemaining <= 0.0f
        && !HasFlag(ECombatFlags::IsGuardBroken);
}

bool UCombatComponent::ConsumeSubstitution()
{
    if (!CanSubstitute()) return false;

    --SubstitutionCharges;

    const UNarutoGameSettings* Settings = GetDefault<UNarutoGameSettings>();
    SubstitutionCooldownRemaining = Settings->SubstitutionCooldown;

    OnSubstitutionUsed.Broadcast(SubstitutionCharges);
    return true;
}

void UCombatComponent::TickSubstitutionCooldown(float DeltaTime)
{
    if (SubstitutionCooldownRemaining <= 0.0f) return;

    SubstitutionCooldownRemaining -= DeltaTime;
    if (SubstitutionCooldownRemaining <= 0.0f)
    {
        SubstitutionCooldownRemaining = 0.0f;
    }
}

void UCombatComponent::RestoreSubstitutionCharge()
{
    SubstitutionCharges = FMath::Min(SubstitutionCharges + 1, MaxSubstitutionCharges);
}

// ============================================================
//  Guard
// ============================================================

void UCombatComponent::DamageGuard(float Amount)
{
    if (IsGuardBroken()) return;

    GuardHealth = FMath::Max(0.0f, GuardHealth - Amount);
    if (GuardHealth <= 0.0f)
    {
        BreakGuard();
    }
}

void UCombatComponent::RestoreGuard(float Amount)
{
    if (IsGuardBroken()) return;
    GuardHealth = FMath::Min(MaxGuardHealth, GuardHealth + Amount);
}

void UCombatComponent::BreakGuard()
{
    GuardHealth = 0.0f;
    AddFlag(ECombatFlags::IsGuardBroken);
    RemoveFlag(ECombatFlags::CanBlock);
    UE_LOG(LogNarutoCombat, Log, TEXT("[CombatComponent] %s guard broken."), *GetOwner()->GetName());
}

void UCombatComponent::TickGuardRecovery(float DeltaTime)
{
    if (!IsGuardBroken()) return;

    // Guard recovers over time after being broken
    GuardHealth += GuardRegenRate * DeltaTime;
    if (GuardHealth >= MaxGuardHealth * 0.5f)
    {
        // Guard is restored at 50% — remove broken state
        RemoveFlag(ECombatFlags::IsGuardBroken);
        AddFlag(ECombatFlags::CanBlock);
    }
}

// ============================================================
//  Hitstun
// ============================================================

void UCombatComponent::ApplyHitstun(int32 Frames)
{
    HitstunFramesRemaining = FMath::Max(HitstunFramesRemaining, Frames);
    SetPhase(ECombatPhase::Hitstun);
    EndAttack(); // Interrupt any active attack
}

void UCombatComponent::TickHitstun()
{
    if (HitstunFramesRemaining <= 0) return;

    --HitstunFramesRemaining;
    if (HitstunFramesRemaining <= 0)
    {
        SetPhase(ECombatPhase::Recovery);
    }
}

// ============================================================
//  Awakening
// ============================================================

bool UCombatComponent::CanAwaken() const
{
    return bHasAwakening && !bIsAwakened;
}

void UCombatComponent::ActivateAwakening(int32 AwakeningIndex)
{
    if (!CanAwaken()) return;

    bIsAwakened          = true;
    CurrentAwakeningIndex = AwakeningIndex;

    AddFlag(ECombatFlags::IsAwakened);
    OnAwakeningActivated.Broadcast();

    UE_LOG(LogNarutoCombat, Log, TEXT("[CombatComponent] %s awakening activated (index %d)."),
        *GetOwner()->GetName(), AwakeningIndex);
}

void UCombatComponent::DeactivateAwakening()
{
    if (!bIsAwakened) return;

    bIsAwakened           = false;
    CurrentAwakeningIndex = -1;

    RemoveFlag(ECombatFlags::IsAwakened);
    OnAwakeningDeactivated.Broadcast();
}
