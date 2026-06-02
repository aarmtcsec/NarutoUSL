// Copyright (c) 2025 Maze Runner Studios. All Rights Reserved.
// Naruto: Ultimate Shinobi Legacy
// ReputationComponent — Per-character faction reputation and allegiance.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Core/Types/NarutoTypes.h"
#include "ReputationComponent.generated.h"

UCLASS(ClassGroup = "NarutoUSL|Character", meta = (BlueprintSpawnableComponent))
class NARUTOUSL_API UReputationComponent : public UActorComponent
{
    GENERATED_BODY()

public:

    UReputationComponent();

    void SetPrimaryFaction(EVillage Faction);

    UFUNCTION(BlueprintPure, Category = "Reputation")
    EVillage GetPrimaryFaction() const { return PrimaryFaction; }

    UFUNCTION(BlueprintPure, Category = "Reputation")
    TArray<EVillage> GetAllFactions() const;

    UFUNCTION(BlueprintPure, Category = "Reputation")
    EReputationTier GetReputationTier(EVillage Faction) const;

    UFUNCTION(BlueprintPure, Category = "Reputation")
    float GetReputationValue(EVillage Faction) const;

    UFUNCTION(BlueprintCallable, Category = "Reputation")
    void ModifyReputation(EVillage Faction, float Delta);

    UFUNCTION(BlueprintPure, Category = "Reputation")
    bool IsHostileTo(EVillage Faction) const;

    UFUNCTION(BlueprintPure, Category = "Reputation")
    bool IsAlliedWith(EVillage Faction) const;

private:

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Reputation",
        meta = (AllowPrivateAccess = "true"))
    EVillage PrimaryFaction = EVillage::None;

    TMap<EVillage, float> ReputationValues;
};
