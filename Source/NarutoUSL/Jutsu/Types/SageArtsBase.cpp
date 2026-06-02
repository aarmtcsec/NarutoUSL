// Copyright (c) 2025 Maze Runner Studios. All Rights Reserved.

#include "Jutsu/Types/SageArtsBase.h"
#include "Character/Base/NarutoCharacterBase.h"
#include "Character/Components/CombatComponent.h"
#include "Character/Components/ChakraComponent.h"
#include "Core/Interfaces/IDamageable.h"
#include "NarutoUSL.h"

FJutsuExecutionResult USageArtsBase::Execute_Implementation(const FJutsuExecutionContext& Context)
{
    FJutsuExecutionResult Result;
    if (!Context.IsValid()) return Result;

    if (!ValidateSageMode(Context))
    {
        Result.FailureReason = TEXT("Not in Sage Mode");
        return Result;
    }

    // Consume sage chakra
    ANarutoCharacterBase* Caster = Context.Caster.Get();
    if (Caster && SageChakraCost > 0.0f)
    {
        UChakraComponent* Chakra = Caster->GetChakraComponent();
        if (Chakra)
        {
            Chakra->ConsumeSageChakra(SageChakraCost);
        }
    }

    // Determine targets
    TArray<AActor*> Targets;
    if (Context.PrimaryTarget.IsValid())
    {
        Targets.Add(Context.PrimaryTarget.Get());
    }
    else if (ExtendedHitboxRange > 0.0f)
    {
        // Frog Kata: query extended range
        Targets = QueryAoETargets(Context, true);
    }

    if (Targets.IsEmpty())
    {
        Result.FailureReason = TEXT("No targets in range");
        return Result;
    }

    const float NaturalEnergyMult = GetNaturalEnergyMultiplier(Context);

    for (AActor* Target : Targets)
    {
        float Damage = ResolveDamage(Context) * NaturalEnergyMult;
        ApplySageArtDamage(Context, Target, Damage, Result);
    }

    if (Result.bHitAnyTarget)
    {
        Result.bSuccess = true;
        SpawnImpactVFX(Context, Context.AimLocation);
        PlayImpactSound(Context, Context.AimLocation);
        TriggerCameraShake(Context);
    }

    return Result;
}

void USageArtsBase::OnCastBegin(const FJutsuExecutionContext& Context)
{
    // Sage Arts require Sage Mode — validate and log
    if (!ValidateSageMode(Context))
    {
        UE_LOG(LogNarutoJutsu, Warning,
            TEXT("[SageArtsBase] %s attempted Sage Art without Sage Mode."),
            Context.Caster.IsValid() ? *Context.Caster->GetName() : TEXT("Unknown"));
    }
}

bool USageArtsBase::ValidateSageMode(const FJutsuExecutionContext& Context) const
{
    ANarutoCharacterBase* Caster = Context.Caster.Get();
    if (!Caster) return false;

    UCombatComponent* Combat = Caster->GetCombatComponent();
    if (!Combat) return false;

    return Combat->GetStance() == ECombatStance::SageMode ||
           Combat->GetStance() == ECombatStance::BijuuMode; // Bijuu mode includes sage power
}

void USageArtsBase::ApplySageArtDamage(const FJutsuExecutionContext& Context,
                                         AActor* Target,
                                         float DamageAmount,
                                         FJutsuExecutionResult& OutResult)
{
    if (!Target || DamageAmount <= 0.0f) return;

    IDamageable* Damageable = Cast<IDamageable>(Target);
    if (!Damageable) return;

    FNarutoDamageEvent DamageEvent;
    DamageEvent.BaseDamage     = DamageAmount;
    DamageEvent.FinalDamage    = DamageAmount;
    DamageEvent.DamageType     = Context.JutsuData.IsValid()
        ? Context.JutsuData->DamageType : EDamageType::Chakra;
    DamageEvent.Instigator     = Context.Caster;
    DamageEvent.Target         = Target;
    DamageEvent.SourceJutsuTag = Context.JutsuData.IsValid()
        ? Context.JutsuData->JutsuTag : FGameplayTag();

    const float Applied = IDamageable::Execute_ApplyDamage(Target, DamageEvent);

    OutResult.TotalDamageDealt += Applied;
    ++OutResult.TargetsHit;
    OutResult.bHitAnyTarget = true;

    ApplyStatusEffects(Context, Target);
}

float USageArtsBase::GetNaturalEnergyMultiplier(const FJutsuExecutionContext& Context) const
{
    ANarutoCharacterBase* Caster = Context.Caster.Get();
    if (!Caster) return NaturalEnergyMultiplier;

    UChakraComponent* Chakra = Caster->GetChakraComponent();
    if (!Chakra) return NaturalEnergyMultiplier;

    // Full sage chakra grants the full multiplier
    // Partial sage chakra scales the bonus proportionally
    const float SageFraction = Chakra->MaxSageChakra > 0.0f
        ? FMath::Clamp(Chakra->GetSageChakra() / Chakra->GetMaxSageChakra(), 0.0f, 1.0f)
        : 1.0f;

    return FMath::Lerp(1.0f, NaturalEnergyMultiplier, SageFraction);
}
