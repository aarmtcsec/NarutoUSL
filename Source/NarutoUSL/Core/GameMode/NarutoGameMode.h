// Copyright (c) 2025 Maze Runner Studios. All Rights Reserved.
// Naruto: Ultimate Shinobi Legacy
// NarutoGameMode — Server-authoritative game rules and session management.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "Core/Types/NarutoTypes.h"
#include "NarutoGameMode.generated.h"

class ANarutoPlayerCharacter;
class ANarutoPlayerController;

// ============================================================
//  Session Type
// ============================================================

UENUM(BlueprintType)
enum class EGameSessionType : uint8
{
    OpenWorld       UMETA(DisplayName = "Open World"),
    BossEncounter   UMETA(DisplayName = "Boss Encounter"),
    ArenaFight      UMETA(DisplayName = "Arena Fight"),
    Cinematic       UMETA(DisplayName = "Cinematic"),
    MainMenu        UMETA(DisplayName = "Main Menu"),
};

// ============================================================
//  NarutoGameMode
// ============================================================

/**
 * ANarutoGameMode
 *
 * Governs session-level rules: respawn logic, encounter boundaries,
 * difficulty scaling, and session type transitions. One GameMode
 * instance exists per level/session. The open world uses a single
 * persistent GameMode; boss arenas and the main menu use derived classes.
 */
UCLASS(Config = Game)
class NARUTOUSL_API ANarutoGameMode : public AGameModeBase
{
    GENERATED_BODY()

public:

    ANarutoGameMode(const FObjectInitializer& ObjectInitializer);

    // ------------------------------------------------------------------
    //  AGameModeBase Interface
    // ------------------------------------------------------------------

    virtual void InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage) override;
    virtual void StartPlay() override;
    virtual void Tick(float DeltaSeconds) override;
    virtual void HandleStartingNewPlayer_Implementation(APlayerController* NewPlayer) override;
    virtual AActor* FindPlayerStart_Implementation(AController* Player, const FString& IncomingName) override;
    virtual void RestartPlayer(AController* NewPlayer) override;

    // ------------------------------------------------------------------
    //  Session Management
    // ------------------------------------------------------------------

    UFUNCTION(BlueprintCallable, Category = "GameMode|Session")
    void SetSessionType(EGameSessionType NewType);

    UFUNCTION(BlueprintPure, Category = "GameMode|Session")
    EGameSessionType GetSessionType() const { return CurrentSessionType; }

    // ------------------------------------------------------------------
    //  Respawn
    // ------------------------------------------------------------------

    /**
     * Called when a player character is defeated.
     * Triggers the respawn flow: death camera, UI, countdown, respawn.
     */
    UFUNCTION(BlueprintCallable, Category = "GameMode|Respawn")
    void HandlePlayerDefeated(ANarutoPlayerController* PlayerController);

    /** Returns the respawn delay in seconds for the current session type. */
    UFUNCTION(BlueprintPure, Category = "GameMode|Respawn")
    float GetRespawnDelay() const;

    // ------------------------------------------------------------------
    //  Difficulty
    // ------------------------------------------------------------------

    UFUNCTION(BlueprintCallable, Category = "GameMode|Difficulty")
    void SetGlobalDifficultyScale(float Scale);

    UFUNCTION(BlueprintPure, Category = "GameMode|Difficulty")
    float GetGlobalDifficultyScale() const { return GlobalDifficultyScale; }

protected:

    // ------------------------------------------------------------------
    //  Configuration
    // ------------------------------------------------------------------

    UPROPERTY(EditDefaultsOnly, Category = "GameMode|Session")
    EGameSessionType CurrentSessionType = EGameSessionType::OpenWorld;

    UPROPERTY(EditDefaultsOnly, Category = "GameMode|Respawn")
    float OpenWorldRespawnDelay = 5.0f;

    UPROPERTY(EditDefaultsOnly, Category = "GameMode|Respawn")
    float BossEncounterRespawnDelay = 8.0f;

    UPROPERTY(EditDefaultsOnly, Category = "GameMode|Respawn")
    float ArenaRespawnDelay = 3.0f;

    UPROPERTY(EditDefaultsOnly, Category = "GameMode|Difficulty")
    float GlobalDifficultyScale = 1.0f;

    // ------------------------------------------------------------------
    //  Internal
    // ------------------------------------------------------------------

    void ExecuteRespawn(ANarutoPlayerController* PlayerController);

    FTimerHandle RespawnTimerHandle;
};
