// Copyright (c) 2025 Maze Runner Studios. All Rights Reserved.
// Naruto: Ultimate Shinobi Legacy
// DamageCalculator — Stateless damage resolution pipeline.
//
// Takes a raw damage event and resolves it through the full pipeline:
//   Raw → Attacker scaling → Defender resistance → Block/Parry reduction
//   → Critical → Status effects → Final
//
// Completely stateless — all inputs come from the event and the
// combatant interfaces. Safe to call from any thread.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Core/Types/NarutoTypes.h"
#include "DamageCalculator.generated.h"

class ICombatant;
class IDamageable;
class UNarutoDamageTypeBase;

// ============================================================
//  Damage Resolution Context
// ============================================================

/**
 * FDamageResolutionContext
 * Full input context for a single damage resolution pass.
 * Built by CombatManager before calling DamageCalculator::Resolve().
 */
USTRUCT()
struct NARUTOUSL_API FDamageResolutionContext
{
    GENERATED_BODY()

    /** The actor dealing damage. Must implement ICombatant. */
    TWeakObjectPtr<AActor> Attacker;

    /** The actor receiving damage. Must implement IDamageable. */
    TWeakObjectPtr<AActor> Defender;

    /** Raw base damage before any modifiers. */
    float RawDamage = 0.0f;

    /** Damage type for resistance lookups. */
    EDamageType DamageType = EDamageType::Physical;

    /** Hitbox that triggered this damage event. */
    FHitboxData HitboxData;

    /** Jutsu tag for mastery scaling and status effect lookups. */
    FGameplayTag SourceJutsuTag;

    /** World-space hit location. */
    FVector HitLocation = FVector::ZeroVector;

    /** World-space hit normal. */
    FVector HitNormal   = FVector::UpVector;

    /** Whether the defender is currently blocking. */
    bool bDefenderIsBlocking = false;

    /** Whether this hit occurred during a perfect guard window. */
    bool bIsPerfectGuard = false;

    /** Whether this hit occurred during a parry window. */
    bool bIsParry = false;

    /** Whether the attacker is in an awakened state. */
    bool bAttackerAwakened = false;

    /** Combo hit index (0 = first hit). Higher combos scale damage. */
    int32 ComboHitIndex = 0;

    /** External damage multiplier (e.g., from team attack bonus). */
    float ExternalMultiplier = 1.0f;
};

// ============================================================
//  Damage Resolution Result
// ============================================================

USTRUCT()
struct NARUTOUSL_API FDamageResolutionResult
{
    GENERATED_BODY()

    /** Final damage to apply to the defender's health. */
    float FinalDamage = 0.0f;

    /** Damage before the final mitigation step (for UI display). */
    float PreMitigationDamage = 0.0f;

    bool bIsCritical    = false;
    bool bIsBlocked     = false;
    bool bIsPerfectGuard = false;
    bool bIsParried     = false;
    bool bIsKillingBlow = false;

    /** Resolved damage event ready to pass to HealthComponent. */
    FNarutoDamageEvent ResolvedEvent;

    /** Debug breakdown string (non-shipping builds only). */
    FString DebugBreakdown;
};

// ============================================================
//  DamageCalculator
// ============================================================

/**
 * UDamageCalculator
 *
 * Stateless utility class. All methods are static.
 * Called exclusively by CombatManager::ResolveDamageEvent().
 *
 * Pipeline stages:
 *   1. AttackerScaling   — apply attacker's PhysicalAttack or ChakraAttack stat
 *   2. MasteryScaling    — apply jutsu mastery damage multiplier
 *   3. ComboScaling      — apply combo hit scaling curve
 *   4. AwakeningBonus    — apply awakening attack multiplier
 *   5. ExternalMult      — apply team attack / world event multipliers
 *   6. CriticalRoll      — roll for critical hit
 *   7. DefenderResist    — apply defender's resistance for this damage type
 *   8. BlockReduction    — apply block or perfect guard reduction
 *   9. ParryReduction    — if parried, reduce to 0 and flag
 *  10. TrueDamageBypass  — True damage skips steps 7-9
 *  11. Clamp             — ensure final damage >= 0
 */
UCLASS(NotBlueprintable)
class NARUTOUSL_API UDamageCalculator : public UObject
{
    GENERATED_BODY()

public:

    /**
     * Primary entry point. Resolves a full damage event from context.
     * Returns a fully populated FDamageResolutionResult.
     */
    static FDamageResolutionResult Resolve(const FDamageResolutionContext& Context);

    /**
     * Quick damage estimate for AI decision-making.
     * Skips critical rolls and status effects. Fast path.
     */
    static float EstimateDamage(const FDamageResolutionContext& Context);

    /**
     * Builds a FNarutoDamageEvent from a resolved result.
     * Called after Resolve() to produce the event passed to HealthComponent.
     */
    static FNarutoDamageEvent BuildDamageEvent(
        const FDamageResolutionContext& Context,
        const FDamageResolutionResult& Result);

private:

    // ------------------------------------------------------------------
    //  Pipeline Stages
    // ------------------------------------------------------------------

    static float ApplyAttackerScaling(float RawDamage, const FDamageResolutionContext& Context);
    static float ApplyMasteryScaling(float Damage, const FDamageResolutionContext& Context);
    static float ApplyComboScaling(float Damage, int32 ComboHitIndex);
    static float ApplyAwakeningBonus(float Damage, const FDamageResolutionContext& Context);
    static bool  RollCritical(const FDamageResolutionContext& Context, float& OutMultiplier);
    static float ApplyDefenderResistance(float Damage, const FDamageResolutionContext& Context);
    static float ApplyBlockReduction(float Damage, bool bIsPerfectGuard, float& OutGuardDamage);
    static float ApplyParryReduction(float Damage, bool bIsParry);

    // ------------------------------------------------------------------
    //  Constants
    // ------------------------------------------------------------------

    /** Block reduces damage by this fraction. */
    static constexpr float BlockDamageReduction     = 0.75f;

    /** Perfect guard reduces damage by this fraction. */
    static constexpr float PerfectGuardReduction    = 1.00f;  // Full block

    /** Parry negates all damage and grants counter window. */
    static constexpr float ParryDamageReduction     = 1.00f;

    /** Combo scaling: each hit beyond the first adds this multiplier. */
    static constexpr float ComboScalingPerHit       = 0.02f;

    /** Maximum combo scaling cap (prevents infinite scaling). */
    static constexpr float MaxComboScalingMultiplier = 1.5f;

    /** Minimum damage floor — hits always deal at least 1 damage. */
    static constexpr float MinimumDamageFloor       = 1.0f;
};
