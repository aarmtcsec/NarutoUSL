// Copyright (c) 2025 Maze Runner Studios. All Rights Reserved.

#include "Jutsu/Types/MedicalJutsuBase.h"
#include "Character/Base/NarutoCharacterBase.h"
#include "Character/Components/HealthComponent.h"
#include "Core/Interfaces/IDamageable.h"
#include "NarutoUSL.h"

FJutsuExecutionResult UMedicalJutsuBase::Execute_Implementation(const FJutsuExecutionContext& Context)
{
    FJutsuExecutionResult Result;
    if (!Context.IsValid()) return Result;

    // Medical jutsu can target self or allies
    AActor* Target = Context.PrimaryTarget.IsValid()
        ? Context.PrimaryTarget.Get()
        : Context.Caster.Get();

    if (!Target) { Result.FailureReason = TEXT("No target"); return Result; }

    switch (MedicalMode)
    {
        case EMedicalJutsuMode::InstantHeal:
            ApplyInstantHeal(Context, Target, Result);
            break;

        case EMedicalJutsuMode::HealOverTime:
        {
            const UJutsuData* Data = Context.JutsuData.Get();
            const float HPS = Data ? Data->Scaling.BaseValue / FMath::Max(1.0f, Data->ActiveDuration) : 10.0f;
            ApplyHealOverTime(Context, Target, HPS, Data ? Data->ActiveDuration : 5.0f, Result);
            break;
        }

        case EMedicalJutsuMode::Revive:
            ApplyRevive(Context, Target, Result);
            break;

        case EMedicalJutsuMode::PoisonCure:
            CureStatusEffects(Context, Target, Result);
            break;

        case EMedicalJutsuMode::ChakraRestore:
        {
            ANarutoCharacterBase* TargetChar = Cast<ANarutoCharacterBase>(Target);
            if (TargetChar)
            {
                const float RestoreAmount = ResolveHealing(Context);
                TargetChar->RestoreChakra_Implementation(RestoreAmount);
                Result.bSuccess = true;
                Result.bHitAnyTarget = true;
            }
            break;
        }

        default:
            break;
    }

    if (Result.bHitAnyTarget)
    {
        Result.bSuccess = true;
        SpawnImpactVFX(Context, Target->GetActorLocation());
        PlayImpactSound(Context, Target->GetActorLocation());
    }

    return Result;
}

void UMedicalJutsuBase::ApplyInstantHeal(const FJutsuExecutionContext& Context,
                                           AActor* Target,
                                           FJutsuExecutionResult& OutResult)
{
    const float HealAmount = ResolveHealing(Context);

    const float Applied = IDamageable::Execute_ApplyHealing(
        Target, HealAmount,
        FGameplayTag::RequestGameplayTag(TEXT("Heal.Medical.Instant")));

    OutResult.TotalHealingDone += Applied;
    OutResult.bHitAnyTarget = Applied > 0.0f;
    ++OutResult.TargetsHit;

    UE_LOG(LogNarutoJutsu, Log,
        TEXT("[MedicalJutsuBase] Healed %s for %.1f HP"), *Target->GetName(), Applied);
}

void UMedicalJutsuBase::ApplyHealOverTime(const FJutsuExecutionContext& Context,
                                            AActor* Target,
                                            float HealPerSecond,
                                            float Duration,
                                            FJutsuExecutionResult& OutResult)
{
    ANarutoCharacterBase* TargetChar = Cast<ANarutoCharacterBase>(Target);
    if (!TargetChar) return;

    UHealthComponent* Health = TargetChar->GetHealthComponent();
    if (!Health) return;

    // Set regen rate for the duration
    // In the full implementation, this creates a timed regen modifier
    // via the StatusEffectManager. For now, apply a direct regen rate boost.
    const float PreviousRegen = Health->GetRegenRate();
    Health->SetRegenRate(PreviousRegen + HealPerSecond);

    // Schedule removal after duration
    FTimerHandle TimerHandle;
    TargetChar->GetWorldTimerManager().SetTimer(
        TimerHandle,
        FTimerDelegate::CreateLambda([Health, PreviousRegen]()
        {
            if (Health)
            {
                Health->SetRegenRate(PreviousRegen);
            }
        }),
        Duration, false);

    OutResult.bHitAnyTarget = true;
    ++OutResult.TargetsHit;
}

void UMedicalJutsuBase::ApplyRevive(const FJutsuExecutionContext& Context,
                                      AActor* Target,
                                      FJutsuExecutionResult& OutResult)
{
    ANarutoCharacterBase* TargetChar = Cast<ANarutoCharacterBase>(Target);
    if (!TargetChar || !TargetChar->IsDead_Implementation()) return;

    UHealthComponent* Health = TargetChar->GetHealthComponent();
    if (!Health) return;

    const float ReviveHealth = Health->GetMaxHealth() * ReviveHealthFraction;
    Health->Revive(ReviveHealth);

    OutResult.TotalHealingDone += ReviveHealth;
    OutResult.bHitAnyTarget = true;
    ++OutResult.TargetsHit;

    UE_LOG(LogNarutoJutsu, Log,
        TEXT("[MedicalJutsuBase] Revived %s with %.1f HP"), *Target->GetName(), ReviveHealth);
}

void UMedicalJutsuBase::CureStatusEffects(const FJutsuExecutionContext& Context,
                                            AActor* Target,
                                            FJutsuExecutionResult& OutResult)
{
    // Full implementation calls StatusEffectManager::RemoveEffectsWithTags()
    // For now, log the cure
    UE_LOG(LogNarutoJutsu, Log,
        TEXT("[MedicalJutsuBase] Cured status effects on %s"), *Target->GetName());

    OutResult.bHitAnyTarget = true;
    ++OutResult.TargetsHit;
}
