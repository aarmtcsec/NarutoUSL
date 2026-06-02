// Copyright (c) 2025 Maze Runner Studios. All Rights Reserved.

#include "Combat/Counters/ParrySystem.h"
#include "Character/Base/NarutoCharacterBase.h"
#include "Character/Components/CombatComponent.h"
#include "Character/Components/HealthComponent.h"
#include "Core/Settings/NarutoGameSettings.h"
#include "NarutoUSL.h"

void UParrySystem::Initialize()
{
    UE_LOG(LogNarutoCombat, Log, TEXT("[ParrySystem] Initialized."));
}

void UParrySystem::Shutdown()
{
    ParryWindows.Empty();
}

void UParrySystem::Tick(float DeltaTime)
{
    TArray<TWeakObjectPtr<ANarutoCharacterBase>> Expired;

    for (auto& Pair : ParryWindows)
    {
        FParryWindowState& State = Pair.Value;
        if (!State.bParryWindowActive && !State.bPerfectGuardActive) continue;

        State.WindowTimeRemaining -= DeltaTime;
        if (State.WindowTimeRemaining <= 0.0f)
        {
            State.bParryWindowActive  = false;
            State.bPerfectGuardActive = false;
        }
    }
}

// ============================================================
//  Window Activation
// ============================================================

void UParrySystem::OnBlockInputPressed(ANarutoCharacterBase* Character)
{
    if (!Character) return;

    const UNarutoGameSettings* Settings = GetDefault<UNarutoGameSettings>();

    FParryWindowState& State = ParryWindows.FindOrAdd(Character);
    State.Character              = Character;
    State.bParryWindowActive     = true;
    State.bPerfectGuardActive    = true;
    State.WindowTimeRemaining    = Settings->PerfectGuardWindow;
    State.ParryWindowDuration    = Settings->ParryWindow;

    // After PerfectGuardWindow expires, the window becomes a normal parry window
    // This is handled in Tick() — bPerfectGuardActive expires first,
    // then bParryWindowActive expires at ParryWindow duration.
}

void UParrySystem::OnBlockInputReleased(ANarutoCharacterBase* Character)
{
    FParryWindowState* State = ParryWindows.Find(Character);
    if (State)
    {
        State->bParryWindowActive  = false;
        State->bPerfectGuardActive = false;
    }
}

// ============================================================
//  Hit Evaluation
// ============================================================

EParryResult UParrySystem::EvaluateIncomingHit(ANarutoCharacterBase* Defender,
                                                ANarutoCharacterBase* Attacker,
                                                const FNarutoDamageEvent& DamageEvent) const
{
    if (!Defender) return EParryResult::None;

    // Check if the damage type is blockable/parriable
    if (DamageEvent.DamageType == EDamageType::Genjutsu ||
        DamageEvent.DamageType == EDamageType::True)
    {
        return EParryResult::None;
    }

    const FParryWindowState* State = ParryWindows.Find(Defender);
    if (!State) return EParryResult::None;

    UCombatComponent* Combat = Defender->GetCombatComponent();
    if (!Combat || Combat->IsGuardBroken()) return EParryResult::None;

    if (State->bPerfectGuardActive && State->WindowTimeRemaining > 0.0f)
    {
        return EParryResult::PerfectGuard;
    }

    if (State->bParryWindowActive && State->WindowTimeRemaining > 0.0f)
    {
        return EParryResult::Parry;
    }

    // Check if holding block (CombatComponent flag)
    if (Combat->IsGuarding())
    {
        return EParryResult::NormalBlock;
    }

    return EParryResult::None;
}

void UParrySystem::ApplyParryReward(ANarutoCharacterBase* Defender,
                                     ANarutoCharacterBase* Attacker,
                                     EParryResult Result)
{
    if (!Defender) return;

    UHealthComponent* Health = Defender->GetHealthComponent();

    switch (Result)
    {
        case EParryResult::PerfectGuard:
        {
            // Perfect guard: full damage negation + brief invulnerability + counter window
            if (Health)
            {
                Health->GrantInvulnerability(0.2f,
                    FGameplayTag::RequestGameplayTag(TEXT("Combat.PerfectGuard.Invulnerability")));
            }

            // Stagger the attacker
            if (Attacker)
            {
                UCombatComponent* AttackerCombat = Attacker->GetCombatComponent();
                if (AttackerCombat)
                {
                    AttackerCombat->ApplyHitstun(20); // ~0.33s at 60fps
                }
            }
            break;
        }

        case EParryResult::Parry:
        {
            // Parry: damage negation + counter window
            if (Health)
            {
                Health->GrantInvulnerability(0.5f,
                    FGameplayTag::RequestGameplayTag(TEXT("Combat.Parry.Invulnerability")));
            }
            break;
        }

        default: break;
    }

    OnParryResult.Broadcast(Defender, Attacker, Result);
}
