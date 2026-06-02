// Copyright (c) 2025 Maze Runner Studios. All Rights Reserved.

#include "Character/Components/ReputationComponent.h"

UReputationComponent::UReputationComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}

void UReputationComponent::SetPrimaryFaction(EVillage Faction)
{
    PrimaryFaction = Faction;
    ReputationValues.FindOrAdd(Faction) = 5000.0f; // Start at Honored with own faction
}

TArray<EVillage> UReputationComponent::GetAllFactions() const
{
    TArray<EVillage> Result;
    ReputationValues.GetKeys(Result);
    return Result;
}

EReputationTier UReputationComponent::GetReputationTier(EVillage Faction) const
{
    return FFactionReputation::CalculateTier(GetReputationValue(Faction));
}

float UReputationComponent::GetReputationValue(EVillage Faction) const
{
    const float* Val = ReputationValues.Find(Faction);
    return Val ? *Val : 0.0f;
}

void UReputationComponent::ModifyReputation(EVillage Faction, float Delta)
{
    float& Val = ReputationValues.FindOrAdd(Faction);
    Val = FMath::Clamp(Val + Delta, -10000.0f, 10000.0f);
}

bool UReputationComponent::IsHostileTo(EVillage Faction) const
{
    return GetReputationTier(Faction) <= EReputationTier::Hostile;
}

bool UReputationComponent::IsAlliedWith(EVillage Faction) const
{
    return Faction == PrimaryFaction ||
           GetReputationTier(Faction) >= EReputationTier::Friendly;
}
