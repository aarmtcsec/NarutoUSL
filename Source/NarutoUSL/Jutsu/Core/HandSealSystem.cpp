// Copyright (c) 2025 Maze Runner Studios. All Rights Reserved.

#include "Jutsu/Core/HandSealSystem.h"
#include "Character/Base/NarutoCharacterBase.h"
#include "NarutoUSL.h"

void UHandSealSystem::Initialize()
{
    UE_LOG(LogNarutoJutsu, Log, TEXT("[HandSealSystem] Initialized."));
}

void UHandSealSystem::Shutdown()
{
    ActiveSequences.Empty();
}

// ============================================================
//  Sequence Management
// ============================================================

bool UHandSealSystem::BeginSequence(ANarutoCharacterBase* Caster,
                                     const UJutsuData* JutsuData,
                                     int32 MasteryLevel)
{
    if (!Caster || !JutsuData) return false;

    // Already has an active sequence
    if (HasActiveSequence(Caster)) return false;

    // No seals required — skip
    if (!JutsuData->RequiresHandSeals())
    {
        // Immediately broadcast completion
        OnHandSealSequenceCompleted.Broadcast(Caster, true);
        return true;
    }

    // At max mastery, skip seals
    if (MasteryLevel >= MasterySkipSealsLevel)
    {
        OnHandSealSequenceCompleted.Broadcast(Caster, true);
        return true;
    }

    const float Duration = CalculateSequenceDuration(JutsuData, MasteryLevel);

    FHandSealState& State = ActiveSequences.FindOrAdd(Caster);
    State.Caster           = Caster;
    State.RequiredSequence = JutsuData->HandSealSequence;
    State.CurrentSealIndex = 0;
    State.TotalDuration    = Duration;
    State.TimeRemaining    = Duration;
    State.bActive          = true;
    State.bCompleted       = false;

    UE_LOG(LogNarutoJutsu, Verbose,
        TEXT("[HandSealSystem] %s began seal sequence for %s (%d seals, %.2fs)"),
        *Caster->GetName(), *JutsuData->DisplayName.ToString(),
        JutsuData->HandSealSequence.Num(), Duration);

    return true;
}

void UHandSealSystem::InterruptSequence(ANarutoCharacterBase* Caster, FGameplayTag JutsuTag)
{
    FHandSealState* State = ActiveSequences.Find(Caster);
    if (!State || !State->bActive) return;

    State->Reset();
    OnHandSealSequenceInterrupted.Broadcast(Caster, JutsuTag);

    UE_LOG(LogNarutoJutsu, Log,
        TEXT("[HandSealSystem] %s seal sequence interrupted."), *Caster->GetName());
}

void UHandSealSystem::CancelSequence(ANarutoCharacterBase* Caster)
{
    FHandSealState* State = ActiveSequences.Find(Caster);
    if (State) State->Reset();
}

// ============================================================
//  Tick
// ============================================================

void UHandSealSystem::Tick(float DeltaTime)
{
    for (auto& Pair : ActiveSequences)
    {
        FHandSealState& State = Pair.Value;
        if (!State.bActive || State.bCompleted) continue;

        ANarutoCharacterBase* Caster = State.Caster.Get();
        if (!Caster)
        {
            State.Reset();
            continue;
        }

        State.TimeRemaining -= DeltaTime;

        // Advance seal index based on time progress
        const float SealInterval = State.TotalDuration /
            FMath::Max(1, State.RequiredSequence.Num());
        const int32 ExpectedSealIndex = FMath::FloorToInt(
            (1.0f - (State.TimeRemaining / State.TotalDuration)) *
            State.RequiredSequence.Num());

        while (State.CurrentSealIndex < ExpectedSealIndex &&
               State.CurrentSealIndex < State.RequiredSequence.Num())
        {
            const EHandSeal CurrentSeal = State.RequiredSequence[State.CurrentSealIndex];
            OnHandSealPerformed.Broadcast(Caster, CurrentSeal, State.CurrentSealIndex);
            ++State.CurrentSealIndex;
        }

        // Sequence complete
        if (State.TimeRemaining <= 0.0f)
        {
            State.bActive    = false;
            State.bCompleted = true;
            OnHandSealSequenceCompleted.Broadcast(Caster, true);

            UE_LOG(LogNarutoJutsu, Verbose,
                TEXT("[HandSealSystem] %s completed seal sequence."), *Caster->GetName());
        }
    }
}

// ============================================================
//  Queries
// ============================================================

bool UHandSealSystem::HasActiveSequence(ANarutoCharacterBase* Caster) const
{
    const FHandSealState* State = ActiveSequences.Find(Caster);
    return State && State->bActive;
}

float UHandSealSystem::GetSequenceProgress(ANarutoCharacterBase* Caster) const
{
    const FHandSealState* State = ActiveSequences.Find(Caster);
    return State ? State->GetProgress() : 0.0f;
}

int32 UHandSealSystem::GetCurrentSealIndex(ANarutoCharacterBase* Caster) const
{
    const FHandSealState* State = ActiveSequences.Find(Caster);
    return State ? State->CurrentSealIndex : 0;
}

// ============================================================
//  Internal
// ============================================================

float UHandSealSystem::CalculateSequenceDuration(const UJutsuData* JutsuData,
                                                   int32 MasteryLevel) const
{
    float Duration = JutsuData->HandSealDuration;

    // Each mastery level reduces duration by 5%, capped at 50% reduction at level 10
    const float MasteryReduction = FMath::Clamp(MasteryLevel * 0.05f, 0.0f, 0.5f);
    Duration *= (1.0f - MasteryReduction);

    return FMath::Max(0.1f, Duration);
}
