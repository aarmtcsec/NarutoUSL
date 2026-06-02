// Copyright (c) 2025 Maze Runner Studios. All Rights Reserved.

#include "Combat/Damage/DamageCalculator.h"
#include "Core/Interfaces/ICombatant.h"
#include "Core/Interfaces/IDamageable.h"
#include "Character/Base/NarutoCharacterBase.h"
#include "Character/Components/ProgressionComponent.h"
#include "NarutoUSL.h"
#include "Math/UnrealMathUtility.h"

// ============================================================
//  Primary Entry Point
// ============================================================

FDamageResolutionResult UDamageCalculator::Resolve(const FDamageResolutionContext& Context)
{
    FDamageResolutionResult Result;

    AActor* AttackerActor = Context.Attacker.Get();
    AActor* DefenderActor = Context.Defender.Get();

    if (!AttackerActor || !DefenderActor)
    {
        UE_LOG(LogNarutoCombat, Warning,
            TEXT("[DamageCalculator] Resolve called with null attacker or defender."));
        return Result;
    }

    // True damage bypasses the entire pipeline except the floor clamp
    if (Context.DamageType == EDamageType::True)
    {
        Result.FinalDamage        = FMath::Max(MinimumDamageFloor, Context.RawDamage);
        Result.PreMitigationDamage = Result.FinalDamage;
        Result.ResolvedEvent      = BuildDamageEvent(Context, Result);
        return Result;
    }

    // ------------------------------------------------------------------
    //  Stage 1: Attacker stat scaling
    // ------------------------------------------------------------------
    float Damage = ApplyAttackerScaling(Context.RawDamage, Context);

    // ------------------------------------------------------------------
    //  Stage 2: Jutsu mastery scaling
    // ------------------------------------------------------------------
    Damage = ApplyMasteryScaling(Damage, Context);

    // ------------------------------------------------------------------
    //  Stage 3: Combo scaling
    // ------------------------------------------------------------------
    Damage = ApplyComboScaling(Damage, Context.ComboHitIndex);

    // ------------------------------------------------------------------
    //  Stage 4: Awakening bonus
    // ------------------------------------------------------------------
    Damage = ApplyAwakeningBonus(Damage, Context);

    // ------------------------------------------------------------------
    //  Stage 5: External multiplier (team attacks, world events)
    // ------------------------------------------------------------------
    Damage *= FMath::Max(0.0f, Context.ExternalMultiplier);

    // ------------------------------------------------------------------
    //  Stage 6: Critical hit roll
    // ------------------------------------------------------------------
    float CritMultiplier = 1.0f;
    Result.bIsCritical = RollCritical(Context, CritMultiplier);
    if (Result.bIsCritical)
    {
        Damage *= CritMultiplier;
    }

    Result.PreMitigationDamage = Damage;

    // ------------------------------------------------------------------
    //  Stage 7: Defender resistance
    // ------------------------------------------------------------------
    Damage = ApplyDefenderResistance(Damage, Context);

    // ------------------------------------------------------------------
    //  Stage 8: Block / Perfect Guard
    // ------------------------------------------------------------------
    if (Context.bIsParry)
    {
        Damage = ApplyParryReduction(Damage, true);
        Result.bIsParried     = true;
        Result.bIsBlocked     = false;
        Result.bIsPerfectGuard = false;
    }
    else if (Context.bDefenderIsBlocking)
    {
        float GuardDamage = 0.0f;
        Damage = ApplyBlockReduction(Damage, Context.bIsPerfectGuard, GuardDamage);
        Result.bIsBlocked      = true;
        Result.bIsPerfectGuard = Context.bIsPerfectGuard;
    }

    // ------------------------------------------------------------------
    //  Stage 9: Floor clamp
    // ------------------------------------------------------------------
    if (!Result.bIsParried)
    {
        Damage = FMath::Max(MinimumDamageFloor, Damage);
    }
    else
    {
        Damage = 0.0f;
    }

    Result.FinalDamage = Damage;

    // ------------------------------------------------------------------
    //  Check killing blow
    // ------------------------------------------------------------------
    IDamageable* Damageable = Cast<IDamageable>(DefenderActor);
    if (Damageable)
    {
        Result.bIsKillingBlow =
            (IDamageable::Execute_GetCurrentHealth(DefenderActor) - Damage) <= 0.0f;
    }

    // ------------------------------------------------------------------
    //  Build debug breakdown (non-shipping)
    // ------------------------------------------------------------------
#if !UE_BUILD_SHIPPING
    Result.DebugBreakdown = FString::Printf(
        TEXT("Raw:%.1f → AttScaled:%.1f → Mastery → Combo → Awakening → PreMit:%.1f → "
             "Resist → Block:%s → Parry:%s → Final:%.1f%s"),
        Context.RawDamage,
        ApplyAttackerScaling(Context.RawDamage, Context),
        Result.PreMitigationDamage,
        Result.bIsBlocked  ? TEXT("YES") : TEXT("NO"),
        Result.bIsParried  ? TEXT("YES") : TEXT("NO"),
        Result.FinalDamage,
        Result.bIsCritical ? TEXT(" [CRIT]") : TEXT("")
    );
#endif

    Result.ResolvedEvent = BuildDamageEvent(Context, Result);
    return Result;
}

// ============================================================
//  Fast Estimate (AI path)
// ============================================================

float UDamageCalculator::EstimateDamage(const FDamageResolutionContext& Context)
{
    float Damage = ApplyAttackerScaling(Context.RawDamage, Context);
    Damage = ApplyDefenderResistance(Damage, Context);
    return FMath::Max(MinimumDamageFloor, Damage);
}

// ============================================================
//  Build Damage Event
// ============================================================

FNarutoDamageEvent UDamageCalculator::BuildDamageEvent(
    const FDamageResolutionContext& Context,
    const FDamageResolutionResult& Result)
{
    FNarutoDamageEvent Event;
    Event.BaseDamage      = Context.RawDamage;
    Event.FinalDamage     = Result.FinalDamage;
    Event.DamageType      = Context.DamageType;
    Event.bIsCritical     = Result.bIsCritical;
    Event.bIsBlocked      = Result.bIsBlocked;
    Event.bIsParried      = Result.bIsParried;
    Event.HitLocation     = Context.HitLocation;
    Event.HitNormal       = Context.HitNormal;
    Event.Instigator      = Context.Attacker;
    Event.Target          = Context.Defender;
    Event.SourceJutsuTag  = Context.SourceJutsuTag;
    Event.HitboxData      = Context.HitboxData;
    return Event;
}

// ============================================================
//  Pipeline Stages
// ============================================================

float UDamageCalculator::ApplyAttackerScaling(float RawDamage, const FDamageResolutionContext& Context)
{
    AActor* AttackerActor = Context.Attacker.Get();
    if (!AttackerActor) return RawDamage;

    ICombatant* Combatant = Cast<ICombatant>(AttackerActor);
    if (!Combatant) return RawDamage;

    // Physical damage scales with PhysicalAttack, chakra damage with ChakraAttack
    const ECharacterStat AttackStat = (Context.DamageType == EDamageType::Physical ||
                                       Context.DamageType == EDamageType::Earth)
        ? ECharacterStat::PhysicalAttack
        : ECharacterStat::ChakraAttack;

    const float AttackValue = ICombatant::Execute_GetResolvedStat(AttackerActor, AttackStat);

    // Damage formula: Raw * (AttackStat / 100)
    // At 100 attack: 1.0x multiplier (baseline)
    // At 200 attack: 2.0x multiplier
    // At 50 attack:  0.5x multiplier
    return RawDamage * (AttackValue / 100.0f);
}

float UDamageCalculator::ApplyMasteryScaling(float Damage, const FDamageResolutionContext& Context)
{
    if (!Context.SourceJutsuTag.IsValid()) return Damage;

    AActor* AttackerActor = Context.Attacker.Get();
    if (!AttackerActor) return Damage;

    ANarutoCharacterBase* NarutoChar = Cast<ANarutoCharacterBase>(AttackerActor);
    if (!NarutoChar || !NarutoChar->GetProgressionComponent()) return Damage;

    const float MasteryMultiplier =
        NarutoChar->GetProgressionComponent()->GetJutsuDamageMultiplier(Context.SourceJutsuTag);

    return Damage * MasteryMultiplier;
}

float UDamageCalculator::ApplyComboScaling(float Damage, int32 ComboHitIndex)
{
    if (ComboHitIndex <= 0) return Damage;

    // Each combo hit beyond the first adds ComboScalingPerHit (2%) up to the cap
    const float ComboMultiplier = FMath::Min(
        1.0f + (ComboHitIndex * ComboScalingPerHit),
        MaxComboScalingMultiplier
    );

    return Damage * ComboMultiplier;
}

float UDamageCalculator::ApplyAwakeningBonus(float Damage, const FDamageResolutionContext& Context)
{
    if (!Context.bAttackerAwakened) return Damage;

    // Awakening attack multiplier is defined in the character's AwakeningData.
    // For the calculator, we use a flat 2.0x as the default.
    // The CombatManager passes the actual multiplier via ExternalMultiplier when awakened.
    return Damage;
}

bool UDamageCalculator::RollCritical(const FDamageResolutionContext& Context, float& OutMultiplier)
{
    AActor* AttackerActor = Context.Attacker.Get();
    if (!AttackerActor) return false;

    ICombatant* Combatant = Cast<ICombatant>(AttackerActor);
    if (!Combatant) return false;

    const float CritChance = ICombatant::Execute_GetResolvedStat(
        AttackerActor, ECharacterStat::CriticalChance);

    const float Roll = FMath::FRand();
    if (Roll < CritChance)
    {
        OutMultiplier = ICombatant::Execute_GetResolvedStat(
            AttackerActor, ECharacterStat::CriticalMultiplier);
        return true;
    }

    return false;
}

float UDamageCalculator::ApplyDefenderResistance(float Damage, const FDamageResolutionContext& Context)
{
    AActor* DefenderActor = Context.Defender.Get();
    if (!DefenderActor) return Damage;

    IDamageable* Damageable = Cast<IDamageable>(DefenderActor);
    if (!Damageable) return Damage;

    const float Resistance =
        IDamageable::Execute_GetDamageResistance(DefenderActor, Context.DamageType);

    // Resistance of 1.0 = no change, 0.5 = 50% reduction, 1.5 = 50% weakness
    return Damage * FMath::Max(0.0f, Resistance);
}

float UDamageCalculator::ApplyBlockReduction(float Damage, bool bIsPerfectGuard, float& OutGuardDamage)
{
    if (bIsPerfectGuard)
    {
        OutGuardDamage = 0.0f;
        return 0.0f;  // Perfect guard negates all damage
    }

    // Normal block: reduce damage by BlockDamageReduction, remainder hits guard health
    const float BlockedDamage   = Damage * BlockDamageReduction;
    const float PassthroughDamage = Damage - BlockedDamage;
    OutGuardDamage = BlockedDamage * 0.1f;  // 10% of blocked damage hits guard health
    return PassthroughDamage;
}

float UDamageCalculator::ApplyParryReduction(float Damage, bool bIsParry)
{
    return bIsParry ? 0.0f : Damage;
}
