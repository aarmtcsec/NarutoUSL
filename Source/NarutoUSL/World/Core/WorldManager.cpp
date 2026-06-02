// Copyright (c) 2025 Maze Runner Studios. All Rights Reserved.

#include "World/Core/WorldManager.h"
#include "Narrative/Core/WorldStateManager.h"
#include "Core/GameState/NarutoGameState.h"
#include "Core/Settings/NarutoGameSettings.h"
#include "Kismet/GameplayStatics.h"
#include "NarutoUSL.h"

UWorldManager::UWorldManager()
{
    SubsystemName = TEXT("WorldManager");
    bTickEnabled  = true;
    TickPriority  = ESubsystemTickPriority::PreRender;
}

void UWorldManager::Initialize(UWorldStateManager* InWorldStateManager)
{
    WorldStateManager = InWorldStateManager;
    bInitialized      = true;
    UE_LOG(LogNarutoWorld, Log, TEXT("[WorldManager] Initialized."));
}

void UWorldManager::Shutdown() {}

void UWorldManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    TickDayNight(DeltaTime);
    TickWeatherTransition(DeltaTime);
    TickSeasonProgression(DeltaTime);
}

// ============================================================
//  Time of Day
// ============================================================

void UWorldManager::SetTimeOfDay(float NewTime)
{
    TimeOfDay = FMath::Fmod(NewTime, 24.0f);
    if (TimeOfDay < 0.0f) TimeOfDay += 24.0f;

    if (ANarutoGameState* GS = GetGameState())
    {
        GS->SetTimeOfDay(TimeOfDay);
    }
}

void UWorldManager::SetTimeScale(float Scale)
{
    TimeScale = FMath::Max(0.0f, Scale);
}

// ============================================================
//  Season
// ============================================================

void UWorldManager::SetSeason(ESeason NewSeason)
{
    if (CurrentSeason == NewSeason) return;
    CurrentSeason = NewSeason;

    if (ANarutoGameState* GS = GetGameState())
    {
        GS->SetCurrentSeason(NewSeason);
    }

    UE_LOG(LogNarutoWorld, Log, TEXT("[WorldManager] Season changed to: %s"),
        *UEnum::GetValueAsString(NewSeason));
}

// ============================================================
//  Weather
// ============================================================

void UWorldManager::RequestWeatherChange(EWeatherType NewWeather, float TransitionDuration)
{
    if (CurrentWeather == NewWeather) return;

    ActiveTransition.FromWeather = CurrentWeather;
    ActiveTransition.ToWeather   = NewWeather;
    ActiveTransition.Progress    = 0.0f;
    ActiveTransition.Duration    = FMath::Max(0.1f, TransitionDuration);
    ActiveTransition.bActive     = true;

    UE_LOG(LogNarutoWorld, Log,
        TEXT("[WorldManager] Weather transition: %s → %s (%.1fs)"),
        *UEnum::GetValueAsString(CurrentWeather),
        *UEnum::GetValueAsString(NewWeather),
        TransitionDuration);
}

// ============================================================
//  World Events
// ============================================================

void UWorldManager::TriggerWorldEvent(FGameplayTag EventTag)
{
    if (!EventTag.IsValid()) return;

    // Notify game state
    // Full implementation routes through WorldStateManager
    UE_LOG(LogNarutoWorld, Log, TEXT("[WorldManager] World event triggered: %s"),
        *EventTag.ToString());
}

void UWorldManager::EndWorldEvent(FGameplayTag EventTag)
{
    UE_LOG(LogNarutoWorld, Log, TEXT("[WorldManager] World event ended: %s"),
        *EventTag.ToString());
}

void UWorldManager::OnWorldChanged(UWorld* OldWorld, UWorld* NewWorld)
{
    // Reset time-sensitive state on world transition
    ActiveTransition.bActive = false;
}

// ============================================================
//  Tick Stages
// ============================================================

void UWorldManager::TickDayNight(float DeltaTime)
{
    const UNarutoGameSettings* Settings = GetDefault<UNarutoGameSettings>();

    // Advance time: SecondsPerGameHour real seconds = 1 in-game hour
    const float HoursPerSecond = 1.0f / Settings->SecondsPerGameHour;
    TimeOfDay += HoursPerSecond * DeltaTime * TimeScale;

    if (TimeOfDay >= 24.0f) TimeOfDay -= 24.0f;

    // Broadcast to game state every in-game hour
    const float CurrentHour = FMath::FloorToFloat(TimeOfDay);
    if (!FMath::IsNearlyEqual(CurrentHour, LastBroadcastHour))
    {
        LastBroadcastHour = CurrentHour;
        BroadcastTimeOfDay();
    }
}

void UWorldManager::TickWeatherTransition(float DeltaTime)
{
    if (!ActiveTransition.bActive) return;

    ActiveTransition.Progress += DeltaTime / ActiveTransition.Duration;

    if (ActiveTransition.Progress >= 1.0f)
    {
        ActiveTransition.Progress = 1.0f;
        ActiveTransition.bActive  = false;
        CurrentWeather = ActiveTransition.ToWeather;

        if (ANarutoGameState* GS = GetGameState())
        {
            GS->SetCurrentWeather(CurrentWeather);
        }

        UE_LOG(LogNarutoWorld, Log, TEXT("[WorldManager] Weather transition complete: %s"),
            *UEnum::GetValueAsString(CurrentWeather));
    }
}

void UWorldManager::TickSeasonProgression(float DeltaTime)
{
    SeasonTimer += DeltaTime;

    if (SeasonTimer >= SeasonDurationSeconds)
    {
        SeasonTimer = 0.0f;
        const uint8 NextSeason = ((uint8)CurrentSeason + 1) % 4;
        SetSeason((ESeason)NextSeason);
    }
}

void UWorldManager::BroadcastTimeOfDay()
{
    if (ANarutoGameState* GS = GetGameState())
    {
        GS->SetTimeOfDay(TimeOfDay);
    }
}

ANarutoGameState* UWorldManager::GetGameState() const
{
    if (UObject* Outer = GetOuter())
    {
        if (UWorld* World = Outer->GetWorld())
        {
            return World->GetGameState<ANarutoGameState>();
        }
    }
    return nullptr;
}
