// Copyright (c) 2025 Maze Runner Studios. All Rights Reserved.

#include "AI/Boss/BossPhaseManager.h"
#include "Character/Boss/NarutoBossBase.h"
#include "Character/Components/HealthComponent.h"
#include "Character/Components/ProgressionComponent.h"
#include "NarutoUSL.h"

void UBossPhaseManager::Initialize(ANarutoBossBase* InBoss,
                                    const TArray<FBossPhaseData>& InPhases)
{
    Boss   = InBoss;
    Phases = InPhases;

    // Sort phases by health threshold descending (phase 0 = full health)
    Phases.Sort([](const FBossPhaseData& A, const FBossPhaseData& B)
    {
        return A.HealthThreshold > B.HealthThreshold;
    });

    CurrentPhaseIndex = 0;
    UE_LOG(LogNarutoAI, Log, TEXT("[BossPhaseManager] Initialized with %d phases."),
        Phases.Num());
}

void UBossPhaseManager::Tick(float DeltaTime)
{
    if (!bInTransition) return;

    TransitionTimer -= DeltaTime;
    if (TransitionTimer <= 0.0f)
    {
        CompleteTransition();
    }
}

void UBossPhaseManager::OnHealthChanged(float NewHealthPercent)
{
    if (bInTransition) return;

    // Check if any phase threshold has been crossed
    for (int32 i = CurrentPhaseIndex + 1; i < Phases.Num(); ++i)
    {
        if (NewHealthPercent <= Phases[i].HealthThreshold)
        {
            TriggerPhaseTransition(i);
            return;
        }
    }
}

const FBossPhaseData* UBossPhaseManager::GetCurrentPhase() const
{
    return Phases.IsValidIndex(CurrentPhaseIndex) ? &Phases[CurrentPhaseIndex] : nullptr;
}

void UBossPhaseManager::TriggerPhaseTransition(int32 NewPhaseIndex)
{
    if (!Phases.IsValidIndex(NewPhaseIndex)) return;

    const FBossPhaseData& NewPhase = Phases[NewPhaseIndex];
    PendingPhaseIndex = NewPhaseIndex;
    bInTransition     = true;
    TransitionTimer   = NewPhase.TransitionDuration;

    ANarutoBossBase* BossActor = Boss.Get();
    if (!BossActor) return;

    UE_LOG(LogNarutoAI, Log,
        TEXT("[BossPhaseManager] %s entering phase %d: %s"),
        *BossActor->GetName(), NewPhaseIndex, *NewPhase.PhaseName.ToString());

    // Grant invulnerability during transition
    if (NewPhase.bInvulnerableDuringTransition)
    {
        UHealthComponent* Health = BossActor->GetHealthComponent();
        if (Health)
        {
            Health->GrantInvulnerability(NewPhase.TransitionDuration,
                FGameplayTag::RequestGameplayTag(TEXT("Boss.PhaseTransition.Invulnerability")));
        }
    }

    // Play transition montage
    if (!NewPhase.TransitionMontage.IsNull())
    {
        UAnimMontage* Montage = NewPhase.TransitionMontage.LoadSynchronous();
        if (Montage)
        {
            BossActor->PlayAnimMontage(Montage);
        }
    }

    // Spawn transition VFX
    if (!NewPhase.TransitionVFX.IsNull())
    {
        UNiagaraSystem* VFX = NewPhase.TransitionVFX.LoadSynchronous();
        if (VFX)
        {
            UNiagaraFunctionLibrary::SpawnSystemAtLocation(
                BossActor->GetWorld(), VFX, BossActor->GetActorLocation());
        }
    }
}

void UBossPhaseManager::CompleteTransition()
{
    if (!Phases.IsValidIndex(PendingPhaseIndex)) return;

    const int32 OldPhase = CurrentPhaseIndex;
    CurrentPhaseIndex    = PendingPhaseIndex;
    PendingPhaseIndex    = -1;
    bInTransition        = false;

    const FBossPhaseData& NewPhase = Phases[CurrentPhaseIndex];

    ApplyPhaseStats(NewPhase);

    if (NewPhase.bIsEnragePhase && !bIsEnraged)
    {
        bIsEnraged = true;
        OnEnraged.Broadcast(Boss.Get());
        UE_LOG(LogNarutoAI, Log, TEXT("[BossPhaseManager] Boss ENRAGED."));
    }

    OnPhaseChanged.Broadcast(Boss.Get(), OldPhase, CurrentPhaseIndex);
}

void UBossPhaseManager::ApplyPhaseStats(const FBossPhaseData& Phase)
{
    ANarutoBossBase* BossActor = Boss.Get();
    if (!BossActor) return;

    UProgressionComponent* Prog = BossActor->GetProgressionComponent();
    if (!Prog) return;

    // Remove previous phase modifiers
    Prog->RemoveStatModifiersFromSource(
        FGameplayTag::RequestGameplayTag(TEXT("Boss.PhaseModifier")));

    // Apply new phase multipliers
    const FGameplayTag PhaseTag =
        FGameplayTag::RequestGameplayTag(TEXT("Boss.PhaseModifier"));

    if (!FMath::IsNearlyEqual(Phase.AttackMultiplier, 1.0f))
    {
        FStatModifier AttackMod(ECharacterStat::PhysicalAttack, 0.0f,
            Phase.AttackMultiplier, PhaseTag);
        Prog->AddStatModifier(AttackMod);

        FStatModifier ChakraMod(ECharacterStat::ChakraAttack, 0.0f,
            Phase.AttackMultiplier, PhaseTag);
        Prog->AddStatModifier(ChakraMod);
    }

    if (!FMath::IsNearlyEqual(Phase.SpeedMultiplier, 1.0f))
    {
        FStatModifier SpeedMod(ECharacterStat::Speed, 0.0f,
            Phase.SpeedMultiplier, PhaseTag);
        Prog->AddStatModifier(SpeedMod);
    }
}
