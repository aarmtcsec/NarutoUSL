// Copyright (c) 2025 Maze Runner Studios. All Rights Reserved.

#include "Jutsu/Core/JutsuExecutor.h"
#include "Character/Base/NarutoCharacterBase.h"
#include "Character/Components/HealthComponent.h"
#include "Core/Interfaces/IDamageable.h"
#include "Core/Interfaces/IFactionMember.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraFunctionLibrary.h"
#include "NarutoUSL.h"

// ============================================================
//  Damage / Healing Resolution
// ============================================================

float UJutsuExecutorBase::ResolveDamage(const FJutsuExecutionContext& Context) const
{
    if (!Context.IsValid()) return 0.0f;

    const UJutsuData* Data = Context.JutsuData.Get();
    const FJutsuScaling& Scaling = Data->Scaling;

    // Base + stat scaling
    float Damage = Scaling.BaseValue;
    Damage += Context.CasterChakraAttack   * Scaling.ChakraAttackScaling;
    Damage += Context.CasterPhysicalAttack * Scaling.PhysicalAttackScaling;

    // Mastery multiplier
    Damage *= Context.CasterMasteryDamageMultiplier;

    // Charge multiplier
    if (Data->bChargeable && Context.ChargeFraction > 0.0f)
    {
        const float ChargeMultiplier = FMath::Lerp(1.0f, Data->MaxChargeMultiplier,
                                                    Context.ChargeFraction);
        Damage *= ChargeMultiplier;
    }

    // Difficulty scale
    Damage *= Context.DifficultyScale;

    return FMath::Max(0.0f, Damage);
}

float UJutsuExecutorBase::ResolveHealing(const FJutsuExecutionContext& Context) const
{
    if (!Context.IsValid()) return 0.0f;

    const UJutsuData* Data = Context.JutsuData.Get();
    const FJutsuScaling& Scaling = Data->Scaling;

    float Healing = Scaling.BaseValue;
    Healing += Context.CasterChakraAttack * Scaling.ChakraAttackScaling;
    Healing *= Context.CasterMasteryDamageMultiplier;

    return FMath::Max(0.0f, Healing);
}

// ============================================================
//  AoE Target Query
// ============================================================

TArray<AActor*> UJutsuExecutorBase::QueryAoETargets(const FJutsuExecutionContext& Context,
                                                      bool bEnemiesOnly) const
{
    TArray<AActor*> Results;

    ANarutoCharacterBase* Caster = Context.Caster.Get();
    if (!Caster || !Context.JutsuData.IsValid()) return Results;

    const UJutsuData* Data = Context.JutsuData.Get();
    const float Radius = Data->AoEData.Radius;

    // Sphere overlap query
    TArray<FOverlapResult> Overlaps;
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(Caster);

    const FVector Origin = Context.AimLocation.IsNearlyZero()
        ? Caster->GetActorLocation()
        : Context.AimLocation;

    Caster->GetWorld()->OverlapMultiByChannel(
        Overlaps,
        Origin,
        FQuat::Identity,
        ECC_Pawn,
        FCollisionShape::MakeSphere(Radius),
        QueryParams
    );

    const EVillage CasterFaction = Caster->GetPrimaryFaction_Implementation();
    int32 HitCount = 0;

    for (const FOverlapResult& Overlap : Overlaps)
    {
        AActor* HitActor = Overlap.GetActor();
        if (!HitActor) continue;

        // Filter by team
        if (bEnemiesOnly)
        {
            IFactionMember* FactionMember = Cast<IFactionMember>(HitActor);
            if (!FactionMember) continue;
            if (!IFactionMember::Execute_IsHostileToFaction(HitActor, CasterFaction)) continue;
        }

        // Check line of sight if required
        if (Data->bRequiresLineOfSight)
        {
            FHitResult LOSResult;
            const bool bBlocked = Caster->GetWorld()->LineTraceSingleByChannel(
                LOSResult, Origin, HitActor->GetActorLocation(),
                ECC_Visibility, QueryParams);
            if (bBlocked && LOSResult.GetActor() != HitActor) continue;
        }

        Results.Add(HitActor);
        ++HitCount;

        if (Data->AoEData.MaxTargets > 0 && HitCount >= Data->AoEData.MaxTargets) break;
    }

    return Results;
}

// ============================================================
//  Status Effects
// ============================================================

void UJutsuExecutorBase::ApplyStatusEffects(const FJutsuExecutionContext& Context,
                                              AActor* Target) const
{
    if (!Target || !Context.JutsuData.IsValid()) return;

    const UJutsuData* Data = Context.JutsuData.Get();

    for (const FJutsuStatusEffect& Effect : Data->StatusEffects)
    {
        // Roll for application chance
        if (FMath::FRand() > Effect.ApplicationChance) continue;

        // In the full implementation, this calls the StatusEffectManager
        // to apply the effect. For now, log the application.
        UE_LOG(LogNarutoJutsu, Verbose,
            TEXT("[JutsuExecutor] Applied status %s to %s for %.1fs"),
            *Effect.StatusTag.ToString(), *Target->GetName(), Effect.Duration);
    }
}

// ============================================================
//  VFX / Audio / Camera
// ============================================================

void UJutsuExecutorBase::SpawnImpactVFX(const FJutsuExecutionContext& Context,
                                          const FVector& Location,
                                          const FRotator& Rotation) const
{
    if (!Context.JutsuData.IsValid()) return;

    const UJutsuData* Data = Context.JutsuData.Get();
    if (!Data->ImpactVFX.IsNull())
    {
        UNiagaraSystem* VFX = Data->ImpactVFX.LoadSynchronous();
        if (VFX && Context.Caster.IsValid())
        {
            UNiagaraFunctionLibrary::SpawnSystemAtLocation(
                Context.Caster->GetWorld(), VFX, Location, Rotation);
        }
    }
}

void UJutsuExecutorBase::PlayImpactSound(const FJutsuExecutionContext& Context,
                                          const FVector& Location) const
{
    if (!Context.JutsuData.IsValid()) return;

    const UJutsuData* Data = Context.JutsuData.Get();
    if (!Data->ImpactSound.IsNull() && Context.Caster.IsValid())
    {
        USoundBase* Sound = Data->ImpactSound.LoadSynchronous();
        if (Sound)
        {
            UGameplayStatics::PlaySoundAtLocation(Context.Caster->GetWorld(), Sound, Location);
        }
    }
}

void UJutsuExecutorBase::TriggerCameraShake(const FJutsuExecutionContext& Context) const
{
    if (!Context.JutsuData.IsValid() || !Context.Caster.IsValid()) return;

    const UJutsuData* Data = Context.JutsuData.Get();
    if (!Data->bTriggersCameraShake || Data->CameraShakeClass.IsNull()) return;

    TSubclassOf<UCameraShakeBase> ShakeClass = Data->CameraShakeClass.LoadSynchronous();
    if (ShakeClass)
    {
        UGameplayStatics::PlayWorldCameraShake(
            Context.Caster->GetWorld(),
            ShakeClass,
            Context.Caster->GetActorLocation(),
            0.0f, 2000.0f, 1.0f);
    }
}
