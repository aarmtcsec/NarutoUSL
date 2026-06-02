// Copyright (c) 2025 Maze Runner Studios. All Rights Reserved.

#include "Character/Boss/Bosses/Boss_Pain.h"
#include "Character/Enemy/NarutoEnemyBase.h"
#include "Character/Components/HealthComponent.h"
#include "Character/Components/CombatComponent.h"
#include "Combat/Core/CombatManager.h"
#include "Jutsu/Core/JutsuManager.h"
#include "Core/GameInstance/NarutoGameInstance.h"
#include "NiagaraFunctionLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "NarutoUSL.h"

ABoss_Pain::ABoss_Pain(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    BossXPReward = 15000.0f;
    RyoDropMin   = 5000;
    RyoDropMax   = 10000;
}

void ABoss_Pain::BeginPlay()
{
    Super::BeginPlay();
    SpawnSixPaths();
}

void ABoss_Pain::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // If Preta Path is active, nullify incoming jutsu
    // Full implementation hooks into JutsuManager's cast validation
}

// ============================================================
//  Phase Initialization
// ============================================================

void ABoss_Pain::InitializePhases()
{
    PhaseDefinitions.Empty();

    // Phase 0 — All Six Paths (100-60%)
    FBossPhaseData Phase0;
    Phase0.PhaseName        = FText::FromString(TEXT("Six Paths of Pain"));
    Phase0.PhaseIndex       = 0;
    Phase0.HealthThreshold  = 1.0f;
    Phase0.AttackMultiplier = 1.0f;
    Phase0.SpeedMultiplier  = 1.0f;
    PhaseDefinitions.Add(Phase0);

    // Phase 1 — Three Paths Remain (60-30%)
    FBossPhaseData Phase1;
    Phase1.PhaseName        = FText::FromString(TEXT("Paths of Destruction"));
    Phase1.PhaseIndex       = 1;
    Phase1.HealthThreshold  = 0.6f;
    Phase1.AttackMultiplier = 1.3f;
    Phase1.SpeedMultiplier  = 1.2f;
    Phase1.UnlockedJutsuTags.Add(
        FGameplayTag::RequestGameplayTag(TEXT("Jutsu.Pain.GravitationalAnomaly")));
    PhaseDefinitions.Add(Phase1);

    // Phase 2 — Deva Path Alone / Enrage (30-0%)
    FBossPhaseData Phase2;
    Phase2.PhaseName        = FText::FromString(TEXT("God of the Ninja World"));
    Phase2.PhaseIndex       = 2;
    Phase2.HealthThreshold  = 0.3f;
    Phase2.AttackMultiplier = 1.8f;
    Phase2.SpeedMultiplier  = 1.5f;
    Phase2.bIsEnragePhase   = true;
    Phase2.UnlockedJutsuTags.Add(
        FGameplayTag::RequestGameplayTag(TEXT("Jutsu.Pain.ChibakuTensei")));
    PhaseDefinitions.Add(Phase2);

    Super::InitializePhases();
}

void ABoss_Pain::OnPhaseTransition(int32 OldPhase, int32 NewPhase)
{
    UE_LOG(LogNarutoAI, Log,
        TEXT("[Boss_Pain] Phase transition %d → %d"), OldPhase, NewPhase);

    if (NewPhase == 1)
    {
        // Kill three randomly chosen paths
        for (int32 i = 0; i < 3 && i < ActivePaths.Num(); ++i)
        {
            if (ANarutoEnemyBase* Path = ActivePaths[i])
            {
                Path->GetHealthComponent()->ForceKill(this);
            }
        }
    }
    else if (NewPhase == 2)
    {
        // Kill all remaining paths — Deva Path (this actor) fights alone
        for (ANarutoEnemyBase* Path : ActivePaths)
        {
            if (Path && Path->IsAlive_Implementation())
            {
                Path->GetHealthComponent()->ForceKill(this);
            }
        }
        ActivePaths.Empty();
    }
}

void ABoss_Pain::OnEnraged()
{
    UE_LOG(LogNarutoAI, Log, TEXT("[Boss_Pain] ENRAGED — Shinra Tensei cooldown halved."));

    // Halve Shinra Tensei cooldown during enrage
    MechanicCooldowns[IDX_SHINRA] = FMath::Min(
        MechanicCooldowns[IDX_SHINRA], ShinraTenseiCooldown * 0.5f);
}

// ============================================================
//  Unique Mechanics
// ============================================================

bool ABoss_Pain::CanExecuteUniqueMechanic(int32 MechanicIndex) const
{
    if (!Super::CanExecuteUniqueMechanic(MechanicIndex)) return false;

    // Chibaku Tensei only available in Phase 2
    if (MechanicIndex == IDX_CHIBAKU && GetCurrentPhase() < 2) return false;

    // Naraka Revival only available if there are defeated paths and we're not Phase 2
    if (MechanicIndex == IDX_NARAKA && DefeatedPathCount == 0) return false;
    if (MechanicIndex == IDX_NARAKA && GetCurrentPhase() >= 2) return false;

    return true;
}

void ABoss_Pain::ExecuteUniqueMechanic(int32 MechanicIndex, AActor* Target)
{
    switch (MechanicIndex)
    {
        case IDX_SHINRA:  ExecuteShinraTensei(Target);  break;
        case IDX_CHIBAKU: ExecuteChibakuTensei(Target); break;
        case IDX_SUMMON:  ExecuteSummonPath();           break;
        case IDX_PRETA:   ActivatePretaPath();           break;
        case IDX_NARAKA:  ExecuteNarakaRevival();        break;
        default: break;
    }

    // Set mechanic cooldown
    static const float Cooldowns[] = {
        ShinraTenseiCooldown, ChibakuTenseiCooldown,
        SummonPathCooldown,   PretaPathCooldown, NarakaRevivalCooldown
    };

    if (MechanicCooldowns.IsValidIndex(MechanicIndex))
    {
        MechanicCooldowns[MechanicIndex] = Cooldowns[MechanicIndex];
    }
}

void ABoss_Pain::ExecuteShinraTensei(AActor* Target)
{
    UE_LOG(LogNarutoAI, Log, TEXT("[Boss_Pain] Shinra Tensei!"));

    const FVector Origin = GetActorLocation();
    const float   Radius = 1500.0f;
    const float   Force  = 3000.0f;

    // Apply radial knockback to all actors in range
    TArray<FOverlapResult> Overlaps;
    FCollisionQueryParams Params;
    Params.AddIgnoredActor(this);

    GetWorld()->OverlapMultiByChannel(Overlaps, Origin, FQuat::Identity,
        ECC_Pawn, FCollisionShape::MakeSphere(Radius), Params);

    for (const FOverlapResult& R : Overlaps)
    {
        ANarutoCharacterBase* Char = Cast<ANarutoCharacterBase>(R.GetActor());
        if (!Char || !Char->IsAlive_Implementation()) continue;

        const FVector Direction = (Char->GetActorLocation() - Origin).GetSafeNormal();
        const float   Distance  = FVector::Dist(Origin, Char->GetActorLocation());
        const float   FallOff   = FMath::Clamp(1.0f - (Distance / Radius), 0.2f, 1.0f);

        Char->ApplyKnockback_Implementation(Direction, Force * FallOff);

        // Apply damage
        FNarutoDamageEvent DmgEvent;
        DmgEvent.BaseDamage   = 200.0f * FallOff;
        DmgEvent.FinalDamage  = DmgEvent.BaseDamage;
        DmgEvent.DamageType   = EDamageType::Chakra;
        DmgEvent.Instigator   = this;
        DmgEvent.Target       = Char;
        DmgEvent.SourceJutsuTag =
            FGameplayTag::RequestGameplayTag(TEXT("Jutsu.Pain.ShinraTensei"));
        Char->ApplyDamage_Implementation(DmgEvent);
    }

    // Spawn VFX
    UGameplayStatics::PlaySoundAtLocation(GetWorld(), nullptr, Origin); // SFX set in BP
}

void ABoss_Pain::ExecuteChibakuTensei(AActor* Target)
{
    UE_LOG(LogNarutoAI, Log, TEXT("[Boss_Pain] Chibaku Tensei!"));

    bChibakuActive = true;

    // Pull all actors toward Pain's position over 3 seconds
    // Full implementation creates a ChibakuTenseiActor that handles
    // the gravitational pull with a timeline and physics impulses
    // then explodes after 8 seconds

    FTimerHandle Timer;
    GetWorldTimerManager().SetTimer(Timer, FTimerDelegate::CreateLambda([this]()
    {
        bChibakuActive = false;
    }), 8.0f, false);
}

void ABoss_Pain::ExecuteSummonPath()
{
    UE_LOG(LogNarutoAI, Log, TEXT("[Boss_Pain] Summon Path activated!"));

    // Summon Path calls a giant boss creature (e.g., giant dog, chameleon)
    // Full implementation spawns from the Summon Path's character data
}

void ABoss_Pain::ActivatePretaPath()
{
    UE_LOG(LogNarutoAI, Log, TEXT("[Boss_Pain] Preta Path — chakra absorption active!"));

    bPretaPathActive = true;

    // Grant invulnerability to jutsu damage for duration
    if (HealthComponent)
    {
        HealthComponent->GrantInvulnerability(5.0f,
            FGameplayTag::RequestGameplayTag(TEXT("Boss.Pain.PretaAbsorption")));
    }

    FTimerHandle Timer;
    GetWorldTimerManager().SetTimer(Timer, FTimerDelegate::CreateLambda([this]()
    {
        bPretaPathActive = false;
        if (HealthComponent)
        {
            HealthComponent->RemoveInvulnerability(
                FGameplayTag::RequestGameplayTag(TEXT("Boss.Pain.PretaAbsorption")));
        }
    }), 5.0f, false);
}

void ABoss_Pain::ExecuteNarakaRevival()
{
    UE_LOG(LogNarutoAI, Log, TEXT("[Boss_Pain] Naraka Path — attempting revival!"));

    // Revive the first defeated Path at 20% health
    for (ANarutoEnemyBase* Path : ActivePaths)
    {
        if (Path && !Path->IsAlive_Implementation())
        {
            UHealthComponent* PathHealth = Path->GetHealthComponent();
            if (PathHealth)
            {
                PathHealth->Revive(PathHealth->GetMaxHealth() * 0.2f);
                --DefeatedPathCount;
                UE_LOG(LogNarutoAI, Log,
                    TEXT("[Boss_Pain] %s revived!"), *Path->GetName());
                return;
            }
        }
    }
}

// ============================================================
//  Six Paths
// ============================================================

void ABoss_Pain::SpawnSixPaths()
{
    // In the full implementation, each Path is a unique enemy subclass
    // with their own jutsu loadout, AI, and animations:
    //   Deva Path  — Shinra Tensei (this actor IS the Deva Path)
    //   Asura Path — Missile/weapon attacks
    //   Human Path — Soul removal (insta-kill on grab)
    //   Animal Path — Summons creatures
    //   Preta Path — Chakra absorption
    //   Naraka Path — Hell King summoning / revival
    //
    // For now log the intended spawn
    UE_LOG(LogNarutoAI, Log,
        TEXT("[Boss_Pain] Six Paths of Pain deployed. %d spawn points configured."),
        PathSpawnPoints.Num());
}

void ABoss_Pain::OnPathDefeated(ANarutoCharacterBase* DefeatedPath,
                                  const FNarutoDamageEvent& KillingBlow)
{
    ++DefeatedPathCount;

    UE_LOG(LogNarutoAI, Log,
        TEXT("[Boss_Pain] Path defeated (%d total). DefeatedCount: %d"),
        ActivePaths.Num(), DefeatedPathCount);
}
