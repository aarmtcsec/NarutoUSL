// Copyright (c) 2025 Maze Runner Studios. All Rights Reserved.

#include "Character/Boss/NarutoBossBase.h"
#include "Character/Components/HealthComponent.h"
#include "NarutoUSL.h"

ANarutoBossBase::ANarutoBossBase(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    PhaseManager = CreateDefaultSubobject<UBossPhaseManager>(TEXT("PhaseManager"));
}

void ANarutoBossBase::BeginPlay()
{
    Super::BeginPlay();

    InitializePhases();

    if (HealthComponent)
    {
        HealthComponent->OnHealthChanged.AddDynamic(
            this, &ANarutoBossBase::HandleHealthChanged);
    }

    if (PhaseManager)
    {
        PhaseManager->OnPhaseChanged.AddUObject(
            this, &ANarutoBossBase::HandlePhaseChanged);
        PhaseManager->OnEnraged.AddUObject(
            this, &ANarutoBossBase::HandleEnraged);
    }

    // Initialize mechanic cooldowns
    MechanicCooldowns.SetNum(5);
    for (float& CD : MechanicCooldowns) CD = 0.0f;
}

void ANarutoBossBase::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (PhaseManager) PhaseManager->Tick(DeltaTime);

    // Tick mechanic cooldowns
    for (float& CD : MechanicCooldowns)
    {
        if (CD > 0.0f) CD -= DeltaTime;
    }
}

int32 ANarutoBossBase::GetCurrentPhase() const
{
    return PhaseManager ? PhaseManager->GetCurrentPhaseIndex() : 0;
}

bool ANarutoBossBase::IsEnraged() const
{
    return PhaseManager && PhaseManager->IsEnraged();
}

void ANarutoBossBase::ExecuteUniqueMechanic(int32 MechanicIndex, AActor* Target)
{
    // Subclasses override this with their specific mechanic logic
    UE_LOG(LogNarutoAI, Log,
        TEXT("[NarutoBossBase] %s executing mechanic %d"),
        *GetName(), MechanicIndex);
}

bool ANarutoBossBase::CanExecuteUniqueMechanic(int32 MechanicIndex) const
{
    if (!MechanicCooldowns.IsValidIndex(MechanicIndex)) return false;
    return MechanicCooldowns[MechanicIndex] <= 0.0f;
}

void ANarutoBossBase::InitializePhases()
{
    if (PhaseManager && PhaseDefinitions.Num() > 0)
    {
        PhaseManager->Initialize(this, PhaseDefinitions);
    }
}

void ANarutoBossBase::OnDefeated_Implementation(ICombatant* Killer)
{
    Super::OnDefeated_Implementation(Killer);

    OnBossDefeated.Broadcast(this);

    UE_LOG(LogNarutoUSL, Log, TEXT("[NarutoBossBase] Boss defeated: %s"), *GetName());
}

void ANarutoBossBase::HandlePhaseChanged(ANarutoBossBase* Boss, int32 OldPhase, int32 NewPhase)
{
    OnPhaseTransition(OldPhase, NewPhase);

    UE_LOG(LogNarutoAI, Log,
        TEXT("[NarutoBossBase] %s phase transition: %d → %d"),
        *GetName(), OldPhase, NewPhase);
}

void ANarutoBossBase::HandleEnraged(ANarutoBossBase* Boss)
{
    OnEnraged();
}

void ANarutoBossBase::HandleHealthChanged(float NewHealth, float MaxHealth)
{
    if (PhaseManager && MaxHealth > 0.0f)
    {
        PhaseManager->OnHealthChanged(NewHealth / MaxHealth);
    }
}
