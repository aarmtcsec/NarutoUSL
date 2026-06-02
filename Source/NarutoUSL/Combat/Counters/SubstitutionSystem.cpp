// Copyright (c) 2025 Maze Runner Studios. All Rights Reserved.

#include "Combat/Counters/SubstitutionSystem.h"
#include "Character/Base/NarutoCharacterBase.h"
#include "Character/Components/CombatComponent.h"
#include "Character/Components/HealthComponent.h"
#include "Combat/Damage/DamageTypes.h"
#include "GameFramework/Character.h"
#include "NarutoUSL.h"

void USubstitutionSystem::Initialize()
{
    UE_LOG(LogNarutoCombat, Log, TEXT("[SubstitutionSystem] Initialized."));
}

void USubstitutionSystem::Shutdown() {}

// ============================================================
//  Substitution Request
// ============================================================

bool USubstitutionSystem::TryExecuteSubstitution(ANarutoCharacterBase* Defender,
                                                   ANarutoCharacterBase* Attacker,
                                                   const FNarutoDamageEvent& IncomingDamageEvent)
{
    if (!CanSubstitute(Defender, IncomingDamageEvent))
    {
        return false;
    }

    const FVector TeleportDest = CalculateTeleportDestination(Defender, Attacker);

    ApplySubstitutionEffects(Defender, Attacker, TeleportDest);

    FSubstitutionEvent Event;
    Event.Defender             = Defender;
    Event.Attacker             = Attacker;
    Event.TeleportDestination  = TeleportDest;
    Event.Timestamp            = Defender->GetWorld()->GetTimeSeconds();
    Event.bSuccessful          = true;

    OnSubstitutionExecuted.Broadcast(Event);

    UE_LOG(LogNarutoCombat, Log,
        TEXT("[SubstitutionSystem] %s substituted against %s. Charges remaining: %d"),
        *Defender->GetName(),
        Attacker ? *Attacker->GetName() : TEXT("Unknown"),
        Defender->GetCombatComponent()->GetSubstitutionCharges());

    return true;
}

// ============================================================
//  Validation
// ============================================================

bool USubstitutionSystem::CanSubstitute(ANarutoCharacterBase* Character,
                                         const FNarutoDamageEvent& IncomingDamageEvent) const
{
    if (!Character) return false;

    UCombatComponent* Combat = Character->GetCombatComponent();
    if (!Combat || !Combat->CanSubstitute()) return false;

    // Check if the damage type allows substitution
    // Genjutsu and True damage cannot be substituted
    if (IncomingDamageEvent.DamageType == EDamageType::Genjutsu ||
        IncomingDamageEvent.DamageType == EDamageType::True)
    {
        return false;
    }

    // Cannot substitute while dead
    if (Character->IsDead_Implementation()) return false;

    return true;
}

// ============================================================
//  Internal
// ============================================================

FVector USubstitutionSystem::CalculateTeleportDestination(ANarutoCharacterBase* Defender,
                                                           ANarutoCharacterBase* Attacker) const
{
    if (!Attacker)
    {
        // No attacker — teleport directly behind the defender's current facing
        return Defender->GetActorLocation() +
               (Defender->GetActorForwardVector() * -TeleportBehindDistance);
    }

    // Teleport to a position behind the attacker
    const FVector AttackerForward = Attacker->GetActorForwardVector();
    const FVector BehindAttacker  = Attacker->GetActorLocation() +
                                    (AttackerForward * -TeleportBehindDistance);

    // Ensure the destination is on the ground (simple Z adjustment)
    FVector Destination = BehindAttacker;
    Destination.Z = Defender->GetActorLocation().Z;

    return Destination;
}

void USubstitutionSystem::SpawnSubstitutionLog(const FVector& Location, UWorld* World) const
{
    // In the full implementation, this spawns a log actor with a dissolve VFX.
    // The log actor is a simple static mesh that fades out after ~1 second.
    // Placeholder: log the location for now.
    UE_LOG(LogNarutoCombat, Verbose,
        TEXT("[SubstitutionSystem] Log spawned at %s"), *Location.ToString());
}

void USubstitutionSystem::ApplySubstitutionEffects(ANarutoCharacterBase* Defender,
                                                    ANarutoCharacterBase* Attacker,
                                                    const FVector& TeleportDestination)
{
    // 1. Spawn log at defender's current position
    SpawnSubstitutionLog(Defender->GetActorLocation(), Defender->GetWorld());

    // 2. Teleport defender
    Defender->SetActorLocation(TeleportDestination, false, nullptr, ETeleportType::TeleportPhysics);

    // 3. Face the attacker
    if (Attacker)
    {
        const FVector ToAttacker = (Attacker->GetActorLocation() - TeleportDestination).GetSafeNormal();
        Defender->SetActorRotation(ToAttacker.Rotation());
    }

    // 4. Consume substitution charge
    UCombatComponent* Combat = Defender->GetCombatComponent();
    if (Combat)
    {
        Combat->ConsumeSubstitution();
    }

    // 5. Grant post-substitution invulnerability
    UHealthComponent* Health = Defender->GetHealthComponent();
    if (Health)
    {
        Health->GrantInvulnerability(
            PostSubstitutionInvulnerability,
            FGameplayTag::RequestGameplayTag(TEXT("Combat.Substitution.Invulnerability")));
    }

    // 6. Notify the defender's ICombatant interface
    Defender->OnSubstitutionActivated_Implementation(Cast<ICombatant>(Attacker));
}
