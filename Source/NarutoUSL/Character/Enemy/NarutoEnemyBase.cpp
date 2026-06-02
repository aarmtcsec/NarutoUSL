// Copyright (c) 2025 Maze Runner Studios. All Rights Reserved.

#include "Character/Enemy/NarutoEnemyBase.h"
#include "Character/Components/JutsuComponent.h"
#include "AI/Core/AIDirector.h"
#include "Combat/Core/CombatManager.h"
#include "Jutsu/Core/JutsuManager.h"
#include "Jutsu/Data/JutsuData.h"
#include "Core/GameInstance/NarutoGameInstance.h"
#include "BehaviorTree/BehaviorTree.h"
#include "AIController.h"
#include "NarutoUSL.h"

ANarutoEnemyBase::ANarutoEnemyBase(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;
}

void ANarutoEnemyBase::BeginPlay()
{
    Super::BeginPlay();

    RegisterWithAIDirector();

    if (UNarutoGameInstance* GI = UNarutoGameInstance::Get(this))
    {
        if (UCombatManager* CM = GI->GetCombatManager())
        {
            CM->RegisterCombatant(this);
        }
    }

    // Start behavior tree
    if (BehaviorTree)
    {
        if (AAIController* AIC = Cast<AAIController>(GetController()))
        {
            AIC->RunBehaviorTree(BehaviorTree);
        }
    }
}

void ANarutoEnemyBase::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (TransformationComponent)
    {
        TransformationComponent->TickTransformation(DeltaTime);
    }
}

void ANarutoEnemyBase::SetEnemyState(EEnemyState NewState)
{
    CurrentEnemyState = NewState;
}

bool ANarutoEnemyBase::TryExecuteBestJutsu(AActor* Target)
{
    if (!JutsuComponent || !Target) return false;

    UNarutoGameInstance* GI = UNarutoGameInstance::Get(this);
    if (!GI) return false;

    UAIDirector* Director = nullptr; // Retrieved from GI in full implementation
    UJutsuManager* JM = GI->GetJutsuManager();
    if (!JM) return false;

    // Find the highest utility jutsu that isn't already in use
    TArray<UJutsuData*> KnownJutsu = JutsuComponent->GetKnownJutsu();
    UJutsuData* BestJutsu = nullptr;
    float BestScore = -1.0f;

    const float DistToTarget = FVector::Dist(GetActorLocation(), Target->GetActorLocation());

    for (UJutsuData* Jutsu : KnownJutsu)
    {
        if (!Jutsu || !JutsuComponent->CanCast(Jutsu->JutsuTag)) continue;

        // Range check
        if (DistToTarget < Jutsu->AIMinRange || DistToTarget > Jutsu->AIMaxRange) continue;

        // Don't duplicate jutsu already in use by another enemy
        if (Director && Director->IsJutsuInUse(Jutsu->JutsuTag)) continue;

        if (Jutsu->AIUtilityScore > BestScore)
        {
            BestScore = Jutsu->AIUtilityScore;
            BestJutsu = Jutsu;
        }
    }

    if (!BestJutsu) return false;

    if (Director) Director->RegisterJutsuInUse(this, BestJutsu->JutsuTag);

    JM->RequestCast(this, BestJutsu->JutsuTag, Target);
    return true;
}

void ANarutoEnemyBase::ExecuteBasicAttack(AActor* Target)
{
    if (!Target) return;

    UNarutoGameInstance* GI = UNarutoGameInstance::Get(this);
    if (!GI) return;

    if (UCombatManager* CM = GI->GetCombatManager())
    {
        CM->ProcessAttackInput(this, EComboInputType::LightAttack);
    }
}

void ANarutoEnemyBase::OnDefeated_Implementation(ICombatant* Killer)
{
    Super::OnDefeated_Implementation(Killer);

    SetEnemyState(EEnemyState::Dead);
    UnregisterFromAIDirector();

    if (UNarutoGameInstance* GI = UNarutoGameInstance::Get(this))
    {
        if (UCombatManager* CM = GI->GetCombatManager())
        {
            CM->UnregisterCombatant(this);
        }
    }

    SpawnLoot();

    // Disable AI
    if (AAIController* AIC = Cast<AAIController>(GetController()))
    {
        AIC->StopMovement();
        AIC->UnPossess();
    }
}

void ANarutoEnemyBase::OnCombatEntered_Implementation()
{
    Super::OnCombatEntered_Implementation();
    SetEnemyState(EEnemyState::Engaging);
}

void ANarutoEnemyBase::OnCombatExited_Implementation()
{
    Super::OnCombatExited_Implementation();
    SetEnemyState(EEnemyState::Idle);
}

void ANarutoEnemyBase::RegisterWithAIDirector()
{
    // AIDirector accessed via GameInstance in full implementation
}

void ANarutoEnemyBase::UnregisterFromAIDirector()
{
    // AIDirector accessed via GameInstance in full implementation
}

void ANarutoEnemyBase::SpawnLoot()
{
    // Full implementation spawns loot actors at death location
    // and grants XP/Ryo to the killer via EconomyManager
    UE_LOG(LogNarutoUSL, Log,
        TEXT("[NarutoEnemyBase] %s dropped loot (%.0f XP, %lld-%lld Ryo)."),
        *GetName(), XPReward, RyoDropMin, RyoDropMax);
}
