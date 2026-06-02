// Copyright (c) 2025 Maze Runner Studios. All Rights Reserved.

#include "Jutsu/Types/SummoningJutsuBase.h"
#include "Character/Base/NarutoCharacterBase.h"
#include "Character/Components/HealthComponent.h"
#include "NarutoUSL.h"

FJutsuExecutionResult USummoningJutsuBase::Execute_Implementation(const FJutsuExecutionContext& Context)
{
    FJutsuExecutionResult Result;
    if (!Context.IsValid()) return Result;

    if (!ValidateSummoningContract(Context))
    {
        Result.FailureReason = TEXT("No summoning contract");
        return Result;
    }

    // Dismiss existing summon if one is active
    if (ActiveSummon.IsValid())
    {
        DismissSummon(ActiveSummon.Get());
    }

    // Apply blood cost
    ANarutoCharacterBase* Caster = Context.Caster.Get();
    if (Caster && BloodCostFraction > 0.0f)
    {
        UHealthComponent* Health = Caster->GetHealthComponent();
        if (Health)
        {
            const float BloodCost = Health->GetMaxHealth() * BloodCostFraction;
            FNarutoDamageEvent BloodEvent;
            BloodEvent.FinalDamage = BloodCost;
            BloodEvent.DamageType  = EDamageType::True;
            BloodEvent.Instigator  = Caster;
            BloodEvent.Target      = Caster;
            Health->ApplyDamage(BloodEvent);
        }
    }

    // Spawn the summon
    ANarutoCharacterBase* Summon = SpawnSummon(Context);
    if (!Summon)
    {
        Result.FailureReason = TEXT("Failed to spawn summon");
        return Result;
    }

    ActiveSummon = Summon;
    InitializeSummonAI(Summon, Context);

    // Schedule auto-dismiss if lifetime is set
    if (SummonLifetime > 0.0f && Caster)
    {
        FTimerHandle LifetimeTimer;
        Caster->GetWorldTimerManager().SetTimer(
            LifetimeTimer,
            FTimerDelegate::CreateUObject(this, &USummoningJutsuBase::DismissSummon, Summon),
            SummonLifetime, false);
    }

    SpawnImpactVFX(Context, Summon->GetActorLocation());
    PlayImpactSound(Context, Summon->GetActorLocation());
    TriggerCameraShake(Context);

    Result.bSuccess      = true;
    Result.bHitAnyTarget = true;
    ++Result.TargetsHit;

    UE_LOG(LogNarutoJutsu, Log,
        TEXT("[SummoningJutsuBase] %s summoned %s"),
        *Caster->GetName(), *Summon->GetName());

    return Result;
}

void USummoningJutsuBase::OnCompleted(const FJutsuExecutionContext& Context)
{
    // Summon persists after jutsu completes — managed by lifetime timer
}

ANarutoCharacterBase* USummoningJutsuBase::SpawnSummon(const FJutsuExecutionContext& Context)
{
    if (SummonClass.IsNull()) return nullptr;

    TSubclassOf<ANarutoCharacterBase> SummonClassLoaded = SummonClass.LoadSynchronous();
    if (!SummonClassLoaded) return nullptr;

    ANarutoCharacterBase* Caster = Context.Caster.Get();
    if (!Caster) return nullptr;

    const FVector SpawnLocation = Caster->GetActorLocation() +
                                  Caster->GetActorForwardVector() * 300.0f;
    const FRotator SpawnRotation = Caster->GetActorRotation();

    FActorSpawnParameters SpawnParams;
    SpawnParams.Instigator = Caster;
    SpawnParams.Owner      = Caster;

    return Caster->GetWorld()->SpawnActor<ANarutoCharacterBase>(
        SummonClassLoaded, SpawnLocation, SpawnRotation, SpawnParams);
}

void USummoningJutsuBase::InitializeSummonAI(ANarutoCharacterBase* Summon,
                                               const FJutsuExecutionContext& Context)
{
    if (!Summon) return;
    // AI initialization is handled by the summon's own BeginPlay and AIController
    // The behavior tree is set in the summon's CharacterData asset
}

void USummoningJutsuBase::DismissSummon(ANarutoCharacterBase* Summon)
{
    if (!Summon) return;

    UE_LOG(LogNarutoJutsu, Log, TEXT("[SummoningJutsuBase] Dismissing summon: %s"),
        *Summon->GetName());

    Summon->Destroy();
    ActiveSummon = nullptr;
}

bool USummoningJutsuBase::ValidateSummoningContract(const FJutsuExecutionContext& Context) const
{
    if (!RequiredContractTag.IsValid()) return true; // No contract required

    // Full implementation checks the caster's progression component for the contract tag
    // For now, allow all summons
    return true;
}
