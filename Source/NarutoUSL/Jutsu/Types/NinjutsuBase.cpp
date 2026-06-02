// Copyright (c) 2025 Maze Runner Studios. All Rights Reserved.

#include "Jutsu/Types/NinjutsuBase.h"
#include "Character/Base/NarutoCharacterBase.h"
#include "Character/Components/HealthComponent.h"
#include "Core/Interfaces/IDamageable.h"
#include "NarutoUSL.h"

FJutsuExecutionResult UNinjutsuBase::Execute_Implementation(const FJutsuExecutionContext& Context)
{
    FJutsuExecutionResult Result;
    if (!Context.IsValid()) return Result;

    const UJutsuData* Data = Context.JutsuData.Get();

    switch (Data->TargetingMode)
    {
        case EJutsuTargetingMode::SingleTarget:
        {
            AActor* Target = Context.PrimaryTarget.Get();
            if (!Target) { Result.FailureReason = TEXT("No target"); return Result; }

            const float Damage = ResolveDamage(Context) *
                                 GetNatureAffinityBonus(Context);
            ApplyNinjutsuDamage(Context, Target, Damage, Result);
            break;
        }

        case EJutsuTargetingMode::AreaOfEffect:
        case EJutsuTargetingMode::Persistent:
        {
            ExecuteAoE(Context, Result);
            break;
        }

        case EJutsuTargetingMode::Projectile:
        {
            AActor* Projectile = SpawnProjectile(Context);
            Result.bSuccess = Projectile != nullptr;
            if (!Projectile) Result.FailureReason = TEXT("Failed to spawn projectile");
            return Result;
        }

        case EJutsuTargetingMode::Self:
        {
            // Self-targeting ninjutsu (e.g., Body Flicker) — subclass handles
            Result.bSuccess = true;
            break;
        }

        default:
            break;
    }

    if (Result.TargetsHit > 0 || Data->TargetingMode == EJutsuTargetingMode::Self)
    {
        Result.bSuccess = true;
        SpawnImpactVFX(Context, Context.AimLocation);
        PlayImpactSound(Context, Context.AimLocation);
        TriggerCameraShake(Context);
    }

    return Result;
}

void UNinjutsuBase::OnActivated(const FJutsuExecutionContext& Context)
{
    // Spawn cast VFX on the caster
    if (Context.Caster.IsValid() && Context.JutsuData.IsValid())
    {
        const UJutsuData* Data = Context.JutsuData.Get();
        if (!Data->CastVFX.IsNull())
        {
            UNiagaraSystem* VFX = Data->CastVFX.LoadSynchronous();
            if (VFX)
            {
                UNiagaraFunctionLibrary::SpawnSystemAttached(
                    VFX,
                    Context.Caster->GetMesh(),
                    NAME_None,
                    Data->CastVFXOffset,
                    FRotator::ZeroRotator,
                    EAttachLocation::KeepRelativeOffset,
                    true);
            }
        }
    }
}

void UNinjutsuBase::OnCompleted(const FJutsuExecutionContext& Context)
{
    UE_LOG(LogNarutoJutsu, Verbose,
        TEXT("[NinjutsuBase] %s completed jutsu: %s"),
        Context.Caster.IsValid() ? *Context.Caster->GetName() : TEXT("Unknown"),
        Context.JutsuData.IsValid() ? *Context.JutsuData->DisplayName.ToString() : TEXT("Unknown"));
}

// ============================================================
//  Damage Application
// ============================================================

void UNinjutsuBase::ApplyNinjutsuDamage(const FJutsuExecutionContext& Context,
                                          AActor* Target,
                                          float DamageAmount,
                                          FJutsuExecutionResult& OutResult)
{
    if (!Target || DamageAmount <= 0.0f) return;

    IDamageable* Damageable = Cast<IDamageable>(Target);
    if (!Damageable) return;

    FNarutoDamageEvent DamageEvent;
    DamageEvent.BaseDamage    = DamageAmount;
    DamageEvent.FinalDamage   = DamageAmount;
    DamageEvent.DamageType    = Context.JutsuData.IsValid()
        ? Context.JutsuData->DamageType : EDamageType::Chakra;
    DamageEvent.Instigator    = Context.Caster;
    DamageEvent.Target        = Target;
    DamageEvent.SourceJutsuTag = Context.JutsuData.IsValid()
        ? Context.JutsuData->JutsuTag : FGameplayTag();

    const float Applied = IDamageable::Execute_ApplyDamage(Target, DamageEvent);

    OutResult.TotalDamageDealt += Applied;
    ++OutResult.TargetsHit;
    OutResult.bHitAnyTarget = true;

    ApplyStatusEffects(Context, Target);
}

void UNinjutsuBase::ExecuteAoE(const FJutsuExecutionContext& Context,
                                 FJutsuExecutionResult& OutResult)
{
    TArray<AActor*> Targets = QueryAoETargets(Context, true);
    if (Targets.IsEmpty()) return;

    const UJutsuData* Data = Context.JutsuData.Get();
    const FVector Origin = Context.AimLocation.IsNearlyZero()
        ? Context.Caster->GetActorLocation()
        : Context.AimLocation;

    for (AActor* Target : Targets)
    {
        float Damage = ResolveDamage(Context) * GetNatureAffinityBonus(Context);

        // Apply distance falloff
        if (Data->AoEData.bFalloffEnabled)
        {
            const float Dist = FVector::Dist(Origin, Target->GetActorLocation());
            const float FalloffFraction = FMath::Clamp(Dist / Data->AoEData.Radius, 0.0f, 1.0f);
            const float FalloffMultiplier = FMath::Lerp(1.0f,
                Data->AoEData.EdgeDamageFraction, FalloffFraction);
            Damage *= FalloffMultiplier;
        }

        ApplyNinjutsuDamage(Context, Target, Damage, OutResult);
        SpawnImpactVFX(Context, Target->GetActorLocation());
    }
}

AActor* UNinjutsuBase::SpawnProjectile(const FJutsuExecutionContext& Context)
{
    if (!Context.IsValid() || !Context.JutsuData.IsValid()) return nullptr;

    const UJutsuData* Data = Context.JutsuData.Get();
    if (Data->ProjectileData.ProjectileClass.IsNull()) return nullptr;

    TSubclassOf<AActor> ProjClass = Data->ProjectileData.ProjectileClass.LoadSynchronous();
    if (!ProjClass) return nullptr;

    ANarutoCharacterBase* Caster = Context.Caster.Get();
    if (!Caster) return nullptr;

    FActorSpawnParameters SpawnParams;
    SpawnParams.Instigator = Caster;
    SpawnParams.Owner      = Caster;

    const FVector SpawnLocation = Caster->GetActorLocation() +
                                  Caster->GetActorForwardVector() * 100.0f;
    const FRotator SpawnRotation = Context.AimDirection.Rotation();

    AActor* Projectile = Caster->GetWorld()->SpawnActor<AActor>(
        ProjClass, SpawnLocation, SpawnRotation, SpawnParams);

    return Projectile;
}

// ============================================================
//  Elemental Interactions
// ============================================================

float UNinjutsuBase::GetElementalInteractionMultiplier(EChakraNature AttackerNature,
                                                         EDamageType DamageType) const
{
    // Elemental interaction table (Naruto lore-accurate)
    // Fire is boosted by Wind, weakened by Water
    // Lightning is boosted by Water, weakened by Earth
    // Earth is boosted by Lightning... etc.

    if (AttackerNature == EChakraNature::Wind && DamageType == EDamageType::Fire)   return 1.5f;
    if (AttackerNature == EChakraNature::Water && DamageType == EDamageType::Fire)  return 0.5f;
    if (AttackerNature == EChakraNature::Water && DamageType == EDamageType::Lightning) return 1.5f;
    if (AttackerNature == EChakraNature::Earth && DamageType == EDamageType::Lightning) return 0.5f;
    if (AttackerNature == EChakraNature::Fire && DamageType == EDamageType::Wind)   return 0.5f;
    if (AttackerNature == EChakraNature::Lightning && DamageType == EDamageType::Earth) return 1.5f;

    return 1.0f;
}

float UNinjutsuBase::GetNatureAffinityBonus(const FJutsuExecutionContext& Context) const
{
    if (!Context.Caster.IsValid() || !Context.JutsuData.IsValid()) return 1.0f;

    const UJutsuData* Data = Context.JutsuData.Get();
    ANarutoCharacterBase* Caster = Context.Caster.Get();

    // Map damage type to nature
    EChakraNature RequiredNature = Data->ChakraCost.RequiredNature;
    if (RequiredNature == EChakraNature::None) return 1.0f;

    return Caster->HasNatureAffinity_Implementation(RequiredNature) ? 1.1f : 1.0f;
}
