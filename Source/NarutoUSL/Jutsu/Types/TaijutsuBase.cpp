// Copyright (c) 2025 Maze Runner Studios. All Rights Reserved.

#include "Jutsu/Types/TaijutsuBase.h"
#include "Character/Base/NarutoCharacterBase.h"
#include "Character/Components/CombatComponent.h"
#include "Core/Interfaces/IDamageable.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "NarutoUSL.h"

FJutsuExecutionResult UTaijutsuBase::Execute_Implementation(const FJutsuExecutionContext& Context)
{
    FJutsuExecutionResult Result;
    if (!Context.IsValid()) return Result;

    AActor* Target = Context.PrimaryTarget.Get();
    if (!Target) { Result.FailureReason = TEXT("No target"); return Result; }

    float Damage = ResolveDamage(Context);

    // Apply speed bonus
    Damage *= GetSpeedDamageBonus(Context);

    // Apply chakra enhancement
    if (bChakraEnhanced)
    {
        Damage *= ChakraEnhancementMultiplier;
    }

    ApplyTaijutsuDamage(Context, Target, Damage, Result);

    if (Result.bHitAnyTarget)
    {
        Result.bSuccess = true;
        SpawnImpactVFX(Context, Target->GetActorLocation());
        PlayImpactSound(Context, Target->GetActorLocation());
        TriggerCameraShake(Context);
    }

    return Result;
}

void UTaijutsuBase::OnActivated(const FJutsuExecutionContext& Context)
{
    // Taijutsu techniques often start with a dash
    if (Context.PrimaryTarget.IsValid())
    {
        ExecuteDash(Context, Context.PrimaryTarget.Get());
    }
}

void UTaijutsuBase::ApplyTaijutsuDamage(const FJutsuExecutionContext& Context,
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
    DamageEvent.DamageType     = EDamageType::Physical;
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

void UTaijutsuBase::ExecuteDash(const FJutsuExecutionContext& Context,
                                  AActor* Target,
                                  float DashSpeed)
{
    ANarutoCharacterBase* Caster = Context.Caster.Get();
    if (!Caster || !Target) return;

    const FVector Direction = (Target->GetActorLocation() - Caster->GetActorLocation()).GetSafeNormal();
    const FVector DashVelocity = Direction * DashSpeed;

    Caster->LaunchCharacter(DashVelocity, true, false);
}

void UTaijutsuBase::ExecuteMultiHit(const FJutsuExecutionContext& Context,
                                      int32 HitCount,
                                      float DamagePerHit,
                                      FJutsuExecutionResult& OutResult)
{
    AActor* Target = Context.PrimaryTarget.Get();
    if (!Target) return;

    for (int32 i = 0; i < HitCount; ++i)
    {
        ApplyTaijutsuDamage(Context, Target, DamagePerHit, OutResult);
    }
}

float UTaijutsuBase::GetSpeedDamageBonus(const FJutsuExecutionContext& Context) const
{
    ANarutoCharacterBase* Caster = Context.Caster.Get();
    if (!Caster) return 1.0f;

    const float Speed = Caster->GetResolvedStat_Implementation(ECharacterStat::Speed);

    // Speed bonus: every 100 speed above baseline (600) adds 2% damage
    // Capped at +50% bonus (at 3100 speed)
    const float SpeedBonus = FMath::Clamp((Speed - 600.0f) / 100.0f * 0.02f, 0.0f, 0.5f);
    return 1.0f + SpeedBonus;
}
