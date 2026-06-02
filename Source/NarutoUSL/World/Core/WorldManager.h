// Copyright (c) 2025 Maze Runner Studios. All Rights Reserved.
// Naruto: Ultimate Shinobi Legacy
// WorldManager — Coordinates weather, day/night, seasons, and world events.

#pragma once

#include "CoreMinimal.h"
#include "Core/Subsystems/NarutoSubsystem.h"
#include "Core/Types/NarutoTypes.h"
#include "WorldManager.generated.h"

class UNarutoEventBus;
class UWorldStateManager;
class ANarutoGameState;

// ============================================================
//  Weather Transition
// ============================================================

USTRUCT()
struct NARUTOUSL_API FWeatherTransition
{
    GENERATED_BODY()

    EWeatherType FromWeather = EWeatherType::Clear;
    EWeatherType ToWeather   = EWeatherType::Clear;
    float        Progress    = 0.0f;   // 0-1
    float        Duration    = 30.0f;
    bool         bActive     = false;
};

// ============================================================
//  WorldManager
// ============================================================

/**
 * UWorldManager
 *
 * Drives all dynamic world systems:
 *   - Day/Night cycle (configurable speed)
 *   - Season progression (Spring → Summer → Autumn → Winter)
 *   - Weather transitions (smooth blending between states)
 *   - World event scheduling (faction wars, invasions, festivals)
 *   - NPC schedule broadcasting (wake/sleep/work/patrol)
 *
 * All world state changes are published through the EventBus
 * so audio, VFX, AI, and UI can react without direct coupling.
 */
UCLASS(NotBlueprintable)
class NARUTOUSL_API UWorldManager : public UNarutoSubsystem
{
    GENERATED_BODY()

public:

    UWorldManager();

    void Initialize(UWorldStateManager* InWorldStateManager);
    virtual void Tick(float DeltaTime) override;
    virtual void Shutdown() override;

    // ------------------------------------------------------------------
    //  Time of Day
    // ------------------------------------------------------------------

    UFUNCTION(BlueprintPure, Category = "World|Time")
    float GetTimeOfDay() const { return TimeOfDay; }

    UFUNCTION(BlueprintPure, Category = "World|Time")
    bool IsNight() const { return TimeOfDay >= 20.0f || TimeOfDay < 6.0f; }

    UFUNCTION(BlueprintPure, Category = "World|Time")
    bool IsDawn() const { return TimeOfDay >= 5.0f && TimeOfDay < 7.0f; }

    UFUNCTION(BlueprintCallable, Category = "World|Time")
    void SetTimeOfDay(float NewTime);

    UFUNCTION(BlueprintCallable, Category = "World|Time")
    void SetTimeScale(float Scale);

    // ------------------------------------------------------------------
    //  Season
    // ------------------------------------------------------------------

    UFUNCTION(BlueprintPure, Category = "World|Season")
    ESeason GetCurrentSeason() const { return CurrentSeason; }

    UFUNCTION(BlueprintCallable, Category = "World|Season")
    void SetSeason(ESeason NewSeason);

    // ------------------------------------------------------------------
    //  Weather
    // ------------------------------------------------------------------

    UFUNCTION(BlueprintPure, Category = "World|Weather")
    EWeatherType GetCurrentWeather() const { return CurrentWeather; }

    UFUNCTION(BlueprintCallable, Category = "World|Weather")
    void RequestWeatherChange(EWeatherType NewWeather, float TransitionDuration = 30.0f);

    UFUNCTION(BlueprintPure, Category = "World|Weather")
    bool IsWeatherTransitioning() const { return ActiveTransition.bActive; }

    UFUNCTION(BlueprintPure, Category = "World|Weather")
    float GetWeatherTransitionProgress() const { return ActiveTransition.Progress; }

    // ------------------------------------------------------------------
    //  World Events
    // ------------------------------------------------------------------

    UFUNCTION(BlueprintCallable, Category = "World|Events")
    void TriggerWorldEvent(FGameplayTag EventTag);

    UFUNCTION(BlueprintCallable, Category = "World|Events")
    void EndWorldEvent(FGameplayTag EventTag);

    // ------------------------------------------------------------------
    //  Callbacks
    // ------------------------------------------------------------------

    void OnWorldChanged(UWorld* OldWorld, UWorld* NewWorld);

private:

    void TickDayNight(float DeltaTime);
    void TickWeatherTransition(float DeltaTime);
    void TickSeasonProgression(float DeltaTime);
    void BroadcastTimeOfDay();

    ANarutoGameState* GetGameState() const;

    TWeakObjectPtr<UWorldStateManager> WorldStateManager;

    float        TimeOfDay       = 6.0f;
    float        TimeScale       = 1.0f;
    ESeason      CurrentSeason   = ESeason::Spring;
    EWeatherType CurrentWeather  = EWeatherType::Clear;

    FWeatherTransition ActiveTransition;

    float SeasonTimer    = 0.0f;
    float LastBroadcastHour = -1.0f;

    // Season duration in real seconds (default ~2 hours per season)
    static constexpr float SeasonDurationSeconds = 7200.0f;
};
