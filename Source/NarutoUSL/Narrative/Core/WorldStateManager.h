// Copyright (c) 2025 Maze Runner Studios. All Rights Reserved.
// Naruto: Ultimate Shinobi Legacy
// WorldStateManager — Authoritative store for all world state flags.
// Drives quest conditions, NPC behavior, and world event eligibility.

#pragma once

#include "CoreMinimal.h"
#include "Core/Subsystems/NarutoSubsystem.h"
#include "GameplayTagContainer.h"
#include "WorldStateManager.generated.h"

class UNarutoEventBus;

/**
 * UWorldStateManager
 *
 * Maintains a set of boolean world state flags identified by GameplayTags.
 * Examples:
 *   WorldState.Konoha.UnderAttack
 *   WorldState.Akatsuki.PainDefeated
 *   WorldState.Chunin.ExamStarted
 *   WorldState.FourthWar.Begun
 *
 * All quest conditions, NPC dialogue conditions, and world event
 * triggers query this manager. Setting a flag broadcasts to the EventBus
 * so all interested systems react immediately.
 */
UCLASS(NotBlueprintable)
class NARUTOUSL_API UWorldStateManager : public UNarutoSubsystem
{
    GENERATED_BODY()

public:

    void Initialize(UNarutoEventBus* InEventBus);
    virtual void Shutdown() override;

    // ------------------------------------------------------------------
    //  Flag API
    // ------------------------------------------------------------------

    UFUNCTION(BlueprintCallable, Category = "WorldState")
    void SetFlag(FGameplayTag Flag, bool bValue);

    UFUNCTION(BlueprintPure, Category = "WorldState")
    bool GetFlag(FGameplayTag Flag) const;

    UFUNCTION(BlueprintPure, Category = "WorldState")
    bool HasAllFlags(const FGameplayTagContainer& Flags) const;

    UFUNCTION(BlueprintPure, Category = "WorldState")
    bool HasAnyFlag(const FGameplayTagContainer& Flags) const;

    UFUNCTION(BlueprintCallable, Category = "WorldState")
    void SetFlags(const FGameplayTagContainer& Flags, bool bValue);

    UFUNCTION(BlueprintPure, Category = "WorldState")
    FGameplayTagContainer GetAllActiveFlags() const;

    // ------------------------------------------------------------------
    //  Serialization
    // ------------------------------------------------------------------

    virtual void SerializeToSave(FArchive& Ar) override;
    virtual void DeserializeFromSave(FArchive& Ar) override;

private:

    FGameplayTagContainer ActiveFlags;
    TWeakObjectPtr<UNarutoEventBus> EventBus;
};
