// Copyright (c) 2025 Maze Runner Studios. All Rights Reserved.
// Naruto: Ultimate Shinobi Legacy
// NarutoGameState — Replicated session-level state visible to all players.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "Core/Types/NarutoTypes.h"
#include "NarutoGameState.generated.h"

// ============================================================
//  Delegates
// ============================================================

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnSessionTimeChanged, float, NewTime, float, MaxTime);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnActiveEncounterChanged, bool, bEncounterActive);

// ============================================================
//  NarutoGameState
// ============================================================

/**
 * ANarutoGameState
 *
 * Holds session-level state that all connected players need to observe:
 * current weather, time of day, active world events, encounter state,
 * and session timer. In single-player this is still used as the
 * authoritative source for world-visible state.
 */
UCLASS()
class NARUTOUSL_API ANarutoGameState : public AGameStateBase
{
    GENERATED_BODY()

public:

    ANarutoGameState(const FObjectInitializer& ObjectInitializer);

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaSeconds) override;

    // ------------------------------------------------------------------
    //  World Time
    // ------------------------------------------------------------------

    /** Returns the current in-game time of day as hours (0.0 - 24.0). */
    UFUNCTION(BlueprintPure, Category = "GameState|World")
    float GetTimeOfDay() const { return TimeOfDay; }

    /** Returns the current season. */
    UFUNCTION(BlueprintPure, Category = "GameState|World")
    ESeason GetCurrentSeason() const { return CurrentSeason; }

    /** Returns the current weather type. */
    UFUNCTION(BlueprintPure, Category = "GameState|World")
    EWeatherType GetCurrentWeather() const { return CurrentWeather; }

    /** Sets the time of day. Called by DayNightCycle system. */
    void SetTimeOfDay(float NewTime);

    /** Sets the current weather. Called by WeatherSystem. */
    void SetCurrentWeather(EWeatherType NewWeather);

    /** Sets the current season. Called by SeasonSystem. */
    void SetCurrentSeason(ESeason NewSeason);

    // ------------------------------------------------------------------
    //  Encounter State
    // ------------------------------------------------------------------

    UFUNCTION(BlueprintPure, Category = "GameState|Encounter")
    bool IsEncounterActive() const { return bEncounterActive; }

    void SetEncounterActive(bool bActive);

    // ------------------------------------------------------------------
    //  Active World Events
    // ------------------------------------------------------------------

    UFUNCTION(BlueprintPure, Category = "GameState|Events")
    const TArray<FGameplayTag>& GetActiveWorldEvents() const { return ActiveWorldEventTags; }

    void AddWorldEvent(FGameplayTag EventTag);
    void RemoveWorldEvent(FGameplayTag EventTag);
    bool HasWorldEvent(FGameplayTag EventTag) const;

    // ------------------------------------------------------------------
    //  Events
    // ------------------------------------------------------------------

    UPROPERTY(BlueprintAssignable, Category = "GameState|Events")
    FOnActiveEncounterChanged OnActiveEncounterChanged;

protected:

    // ------------------------------------------------------------------
    //  Replicated State
    // ------------------------------------------------------------------

    UPROPERTY(BlueprintReadOnly, Category = "GameState|World")
    float TimeOfDay = 6.0f;  // Start at 6 AM

    UPROPERTY(BlueprintReadOnly, Category = "GameState|World")
    ESeason CurrentSeason = ESeason::Spring;

    UPROPERTY(BlueprintReadOnly, Category = "GameState|World")
    EWeatherType CurrentWeather = EWeatherType::Clear;

    UPROPERTY(BlueprintReadOnly, Category = "GameState|Encounter")
    bool bEncounterActive = false;

    UPROPERTY(BlueprintReadOnly, Category = "GameState|Events")
    TArray<FGameplayTag> ActiveWorldEventTags;
};
