// Copyright (c) 2025 Maze Runner Studios. All Rights Reserved.
#pragma once
#include "CoreMinimal.h"
#include "Core/Subsystems/NarutoSubsystem.h"
#include "Core/Types/NarutoTypes.h"
#include "FactionManager.generated.h"

class UNarutoEventBus;
class UWorldStateManager;

UCLASS(NotBlueprintable)
class NARUTOUSL_API UFactionManager : public UNarutoSubsystem
{
    GENERATED_BODY()
public:
    void Initialize(UNarutoEventBus* InEventBus, UWorldStateManager* InWorldStateManager);
    virtual void Shutdown() override;

    UFUNCTION(BlueprintCallable, Category = "Faction")
    void ModifyReputation(EVillage Faction, float Delta);

    UFUNCTION(BlueprintPure, Category = "Faction")
    float GetReputation(EVillage Faction) const;

    UFUNCTION(BlueprintPure, Category = "Faction")
    EReputationTier GetReputationTier(EVillage Faction) const;

    UFUNCTION(BlueprintPure, Category = "Faction")
    bool IsHostileTo(EVillage FactionA, EVillage FactionB) const;

private:
    TMap<EVillage, float> ReputationMap;
    TWeakObjectPtr<UNarutoEventBus>    EventBus;
    TWeakObjectPtr<UWorldStateManager> WorldStateManager;
};
