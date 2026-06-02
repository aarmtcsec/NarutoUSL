// Copyright (c) 2025 Maze Runner Studios. All Rights Reserved.
// Naruto: Ultimate Shinobi Legacy
// NarutoPlayerState — Persistent per-player data replicated to all clients.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "Core/Types/NarutoTypes.h"
#include "GameplayTagContainer.h"
#include "NarutoPlayerState.generated.h"

/**
 * ANarutoPlayerState
 *
 * Holds player-identity data that persists across respawns and
 * is replicated to all players in a session.
 * Per-character stat data lives in ProgressionComponent, not here.
 */
UCLASS()
class NARUTOUSL_API ANarutoPlayerState : public APlayerState
{
    GENERATED_BODY()

public:

    ANarutoPlayerState();

    // ------------------------------------------------------------------
    //  Identity
    // ------------------------------------------------------------------

    UFUNCTION(BlueprintPure, Category = "PlayerState")
    FGameplayTag GetActiveCharacterTag() const { return ActiveCharacterTag; }

    UFUNCTION(BlueprintCallable, Category = "PlayerState")
    void SetActiveCharacterTag(FGameplayTag NewTag);

    UFUNCTION(BlueprintPure, Category = "PlayerState")
    EShinobi_Rank GetHighestRank() const { return HighestRank; }

    UFUNCTION(BlueprintCallable, Category = "PlayerState")
    void SetHighestRank(EShinobi_Rank NewRank);

    // ------------------------------------------------------------------
    //  Session Stats
    // ------------------------------------------------------------------

    UFUNCTION(BlueprintPure, Category = "PlayerState")
    int32 GetTotalKills() const { return TotalKills; }

    UFUNCTION(BlueprintPure, Category = "PlayerState")
    int32 GetTotalDeaths() const { return TotalDeaths; }

    UFUNCTION(BlueprintCallable, Category = "PlayerState")
    void RecordKill();

    UFUNCTION(BlueprintCallable, Category = "PlayerState")
    void RecordDeath();

protected:

    UPROPERTY(BlueprintReadOnly, Category = "PlayerState")
    FGameplayTag ActiveCharacterTag;

    UPROPERTY(BlueprintReadOnly, Category = "PlayerState")
    EShinobi_Rank HighestRank = EShinobi_Rank::Genin;

    UPROPERTY(BlueprintReadOnly, Category = "PlayerState")
    int32 TotalKills = 0;

    UPROPERTY(BlueprintReadOnly, Category = "PlayerState")
    int32 TotalDeaths = 0;
};
