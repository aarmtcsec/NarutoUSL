// Copyright (c) 2025 Maze Runner Studios. All Rights Reserved.
// Naruto: Ultimate Shinobi Legacy
// Interface: IFactionMember

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Core/Types/NarutoTypes.h"
#include "IFactionMember.generated.h"

UINTERFACE(MinimalAPI, BlueprintType)
class UFactionMember : public UInterface
{
    GENERATED_BODY()
};

/**
 * IFactionMember
 *
 * Implemented by characters and actors that belong to a faction.
 * Used by the FactionManager and AI systems to determine allegiance,
 * hostility, and reputation-driven behavior.
 */
class NARUTOUSL_API IFactionMember
{
    GENERATED_BODY()

public:

    /** Returns the primary village/faction this actor belongs to. */
    UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Faction")
    EVillage GetPrimaryFaction() const;

    /** Returns all factions this actor is affiliated with. */
    UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Faction")
    TArray<EVillage> GetAllFactions() const;

    /** Returns the shinobi rank of this actor. */
    UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Faction")
    EShinobi_Rank GetShinobiRank() const;

    /**
     * Returns the reputation tier this actor has with the specified faction.
     * Used by AI to determine whether to attack, ignore, or assist.
     */
    UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Faction")
    EReputationTier GetReputationWithFaction(EVillage Faction) const;

    /**
     * Returns true if this actor is hostile toward the specified faction.
     * Drives AI combat targeting decisions.
     */
    UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Faction")
    bool IsHostileToFaction(EVillage Faction) const;

    /**
     * Returns true if this actor is allied with the specified faction.
     */
    UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Faction")
    bool IsAlliedWithFaction(EVillage Faction) const;

    /** Called when this actor's reputation with a faction changes. */
    UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Faction|Events")
    void OnReputationChanged(EVillage Faction, float OldValue, float NewValue, EReputationTier NewTier);

    /** Called when this actor's faction allegiance changes (e.g., defection). */
    UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Faction|Events")
    void OnFactionChanged(EVillage OldFaction, EVillage NewFaction);
};
