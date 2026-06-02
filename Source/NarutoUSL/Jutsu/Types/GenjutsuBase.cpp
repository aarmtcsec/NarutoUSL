// Copyright (c) 2025 Maze Runner Studios. All Rights Reserved.

#include "Jutsu/Types/GenjutsuBase.h"
#include "Character/Base/NarutoCharacterBase.h"
#include "Character/Components/CombatComponent.h"
#include "Core/Interfaces/IDamageable.h"
#include "NarutoUSL.h"

FJutsuExecutionResult UGenjutsuBase::Execute_Implementation(const FJutsuExecutionContext& Context)
{
    FJutsuExecutionResult Result;
    if (!Context.IsValid()) return Result;

    AActor* Target = Context.PrimaryTarget.Get();
    if (!Target) { Result.FailureReason = TEXT("No target"); return Result; }

    // Check eye contact requirement
    if (bRequiresEyeContact && Context.Caster.IsValid())
    {
        const FVector ToTarget = (Target->GetActorLocation() -
                                   Context.Caster->GetActorLocation()).GetSafeNormal();
        const float DotProduct = FVector::DotProduct(
            Context.Caster->GetActorForwardVector(), ToTarget);

        if (DotProduct < 0.7f) // ~45 degree cone
        {
            Result.FailureReason = TEXT("Target not in eye contact range");
            return Result;
        }
    }

    // Check resistance
    if (CanTargetResist(Context, Target))
    {
        UE_LOG(LogNarutoJutsu, Log,
            TEXT("[GenjutsuBase] %s resisted genjutsu from %s"),
            *Target->GetName(),
            Context.Caster.IsValid() ? *Context.Caster->GetName() : TEXT("Unknown"));
        Result.FailureReason = TEXT("Target resisted");
        return Result;
    }

    ApplyGenjutsuEffect(Context, Target, Result);

    if (Result.bHitAnyTarget)
    {
        Result.bSuccess = true;
        SpawnImpactVFX(Context, Target->GetActorLocation());
        PlayImpactSound(Context, Target->GetActorLocation());
    }

    return Result;
}

bool UGenjutsuBase::CanTargetResist(const FJutsuExecutionContext& Context,
                                      AActor* Target) const
{
    // Sage Mode users are immune to genjutsu
    ANarutoCharacterBase* TargetChar = Cast<ANarutoCharacterBase>(Target);
    if (!TargetChar) return false;

    UCombatComponent* Combat = TargetChar->GetCombatComponent();
    if (Combat && Combat->GetStance() == ECombatStance::SageMode)
    {
        return true;
    }

    // Check immunity tags (e.g., Sharingan users have partial resistance)
    // Full implementation queries the character's active gameplay tag container
    return false;
}

void UGenjutsuBase::ApplyGenjutsuEffect(const FJutsuExecutionContext& Context,
                                          AActor* Target,
                                          FJutsuExecutionResult& OutResult)
{
    if (!Target) return;

    const UJutsuData* Data = Context.JutsuData.Get();
    const float Duration = Data ? Data->ActiveDuration : 3.0f;

    // Apply psychological damage (bypasses physical defense)
    const float PsychDamage = ResolveDamage(Context);
    if (PsychDamage > 0.0f)
    {
        FNarutoDamageEvent DamageEvent;
        DamageEvent.BaseDamage     = PsychDamage;
        DamageEvent.FinalDamage    = PsychDamage;
        DamageEvent.DamageType     = EDamageType::Genjutsu;
        DamageEvent.Instigator     = Context.Caster;
        DamageEvent.Target         = Target;
        DamageEvent.SourceJutsuTag = Data ? Data->JutsuTag : FGameplayTag();

        IDamageable::Execute_ApplyDamage(Target, DamageEvent);
        OutResult.TotalDamageDealt += PsychDamage;
    }

    // Apply the genjutsu status effect
    FGameplayTag EffectTag;
    switch (PrimaryEffect)
    {
        case EGenjutsuEffect::Paralysis:
            EffectTag = FGameplayTag::RequestGameplayTag(TEXT("Status.Genjutsu.Paralysis"));
            break;
        case EGenjutsuEffect::Confusion:
            EffectTag = FGameplayTag::RequestGameplayTag(TEXT("Status.Genjutsu.Confusion"));
            break;
        case EGenjutsuEffect::Fear:
            EffectTag = FGameplayTag::RequestGameplayTag(TEXT("Status.Genjutsu.Fear"));
            break;
        default:
            EffectTag = FGameplayTag::RequestGameplayTag(TEXT("Status.Genjutsu.Generic"));
            break;
    }

    // Apply hitstun equivalent for paralysis
    if (PrimaryEffect == EGenjutsuEffect::Paralysis)
    {
        ANarutoCharacterBase* TargetChar = Cast<ANarutoCharacterBase>(Target);
        if (TargetChar)
        {
            const int32 ParalysisFrames = FMath::RoundToInt(Duration * 60.0f);
            TargetChar->ApplyHitstun_Implementation(ParalysisFrames);
        }
    }

    ++OutResult.TargetsHit;
    OutResult.bHitAnyTarget = true;
    OutResult.AppliedStatusTags.AddTag(EffectTag);

    UE_LOG(LogNarutoJutsu, Log,
        TEXT("[GenjutsuBase] Applied %s to %s for %.1fs"),
        *EffectTag.ToString(), *Target->GetName(), Duration);
}
