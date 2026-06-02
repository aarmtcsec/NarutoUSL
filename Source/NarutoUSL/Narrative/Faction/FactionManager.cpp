// Copyright (c) 2025 Maze Runner Studios. All Rights Reserved.

#include "Narrative/Faction/FactionManager.h"
#include "Core/Events/NarutoEventBus.h"
#include "Narrative/Core/WorldStateManager.h"
#include "NarutoUSL.h"

void UFactionManager::Initialize(UNarutoEventBus* InEventBus,
                                   UWorldStateManager* InWorldStateManager)
{
    EventBus          = InEventBus;
    WorldStateManager = InWorldStateManager;

    // Initialize all factions to neutral
    for (uint8 i = 0; i <= (uint8)EVillage::Otsutsuki; ++i)
    {
        ReputationMap.Add((EVillage)i, 0.0f);
    }

    bInitialized = true;
    UE_LOG(LogNarutoNarrative, Log, TEXT("[FactionManager] Initialized."));
}

void UFactionManager::Shutdown()
{
    ReputationMap.Empty();
}

void UFactionManager::ModifyReputation(EVillage Faction, float Delta)
{
    float& Rep = ReputationMap.FindOrAdd(Faction);
    const float OldRep = Rep;
    Rep = FMath::Clamp(Rep + Delta, -10000.0f, 10000.0f);

    const EReputationTier OldTier = FFactionReputation::CalculateTier(OldRep);
    const EReputationTier NewTier = FFactionReputation::CalculateTier(Rep);

    if (EventBus.IsValid())
    {
        FReputationChangedEvent Evt;
        Evt.Faction   = Faction;
        Evt.OldValue  = OldRep;
        Evt.NewValue  = Rep;
        Evt.OldTier   = OldTier;
        Evt.NewTier   = NewTier;
        EventBus->OnReputationChanged.Broadcast(Evt);
    }

    UE_LOG(LogNarutoNarrative, Log,
        TEXT("[FactionManager] %s reputation: %.0f → %.0f (%s)"),
        *UEnum::GetValueAsString(Faction), OldRep, Rep,
        *UEnum::GetValueAsString(NewTier));
}

float UFactionManager::GetReputation(EVillage Faction) const
{
    const float* Rep = ReputationMap.Find(Faction);
    return Rep ? *Rep : 0.0f;
}

EReputationTier UFactionManager::GetReputationTier(EVillage Faction) const
{
    return FFactionReputation::CalculateTier(GetReputation(Faction));
}

bool UFactionManager::IsHostileTo(EVillage FactionA, EVillage FactionB) const
{
    // Hardcoded hostility rules (data-driven in full implementation)
    if (FactionA == EVillage::Akatsuki && FactionB != EVillage::Akatsuki) return true;
    if (FactionB == EVillage::Akatsuki && FactionA != EVillage::Akatsuki) return true;
    if (FactionA == EVillage::Otsutsuki) return true;
    if (FactionB == EVillage::Otsutsuki) return true;
    return false;
}
