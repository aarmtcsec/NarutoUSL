// Copyright (c) 2025 Maze Runner Studios. All Rights Reserved.
// Naruto: Ultimate Shinobi Legacy
// SubstitutionSystem — Manages the substitution jutsu (Kawarimi no Jutsu).
// Handles input detection, charge consumption, teleport execution, and log.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Core/Types/NarutoTypes.h"
#include "SubstitutionSystem.generated.h"

class ANarutoCharacterBase;
class UCombatComponent;
class UHealthComponent;

// ============================================================
//  Substitution Event
// ============================================================

USTRUCT()
struct NARUTOUSL_API FSubstitutionEvent
{
    GENERATED_BODY()

    TWeakObjectPtr<ANarutoCharacterBase> Defender;
    TWeakObjectPtr<ANarutoCharacterBase> Attacker;
    FVector                              TeleportDestination = FVector::ZeroVector;
    float                                Timestamp           = 0.0f;
    bool                                 bSuccessful         = false;
};

DECLARE_MULTICAST_DELEGATE_OneParam(FOnSubstitutionExecuted, const FSubstitutionEvent&);

// ============================================================
//  SubstitutionSystem
// ============================================================

/**
 * USubstitutionSystem
 *
 * Owned by CombatManager. Processes substitution requests from
 * characters that are about to receive a hit.
 *
 * Substitution rules:
 *   1. Defender must have at least 1 substitution charge
 *   2. Defender must not be in guard-broken state
 *   3. Defender must not be in a state that prevents substitution
 *      (e.g., grabbed, certain boss mechanics)
 *   4. The incoming hit's damage type must be substitutable
 *   5. Substitution has a per-combat charge limit (default 4)
 *   6. Substitution has a cooldown between uses
 *
 * On successful substitution:
 *   - Defender teleports to a position behind the attacker
 *   - Defender gains brief invulnerability
 *   - A log (wooden dummy) appears at the original position
 *   - The attacker's hit is negated
 *   - Defender's substitution charge is consumed
 */
UCLASS(NotBlueprintable)
class NARUTOUSL_API USubstitutionSystem : public UObject
{
    GENERATED_BODY()

public:

    void Initialize();
    void Shutdown();

    // ------------------------------------------------------------------
    //  Substitution Request
    // ------------------------------------------------------------------

    /**
     * Attempts to execute a substitution for the defender against the attacker.
     * Called by CombatManager when a hit is about to land.
     *
     * @param Defender  The character attempting to substitute.
     * @param Attacker  The character that landed the hit.
     * @param IncomingDamageEvent  The damage event that triggered the substitution.
     * @return true if substitution was successfully executed.
     */
    bool TryExecuteSubstitution(ANarutoCharacterBase* Defender,
                                ANarutoCharacterBase* Attacker,
                                const FNarutoDamageEvent& IncomingDamageEvent);

    // ------------------------------------------------------------------
    //  Queries
    // ------------------------------------------------------------------

    bool CanSubstitute(ANarutoCharacterBase* Character,
                       const FNarutoDamageEvent& IncomingDamageEvent) const;

    // ------------------------------------------------------------------
    //  Events
    // ------------------------------------------------------------------

    FOnSubstitutionExecuted OnSubstitutionExecuted;

private:

    FVector CalculateTeleportDestination(ANarutoCharacterBase* Defender,
                                         ANarutoCharacterBase* Attacker) const;

    void SpawnSubstitutionLog(const FVector& Location, UWorld* World) const;

    void ApplySubstitutionEffects(ANarutoCharacterBase* Defender,
                                  ANarutoCharacterBase* Attacker,
                                  const FVector& TeleportDestination);

    /** Distance behind the attacker the defender teleports to. */
    static constexpr float TeleportBehindDistance = 200.0f;

    /** Invulnerability duration granted after substitution. */
    static constexpr float PostSubstitutionInvulnerability = 0.3f;
};
