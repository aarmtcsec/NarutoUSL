// Copyright (c) 2025 Maze Runner Studios. All Rights Reserved.

#include "Core/GameState/NarutoGameState.h"
#include "Core/GameInstance/NarutoGameInstance.h"
#include "Core/Events/NarutoEventBus.h"

ANarutoGameState::ANarutoGameState(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    PrimaryActorTick.bCanEverTick = true;
}

void ANarutoGameState::BeginPlay()
{
    Super::BeginPlay();
}

void ANarutoGameState::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);
}

void ANarutoGameState::SetTimeOfDay(float NewTime)
{
    TimeOfDay = FMath::Fmod(NewTime, 24.0f);
    if (TimeOfDay < 0.0f) TimeOfDay += 24.0f;
}

void ANarutoGameState::SetCurrentWeather(EWeatherType NewWeather)
{
    if (CurrentWeather == NewWeather) return;

    CurrentWeather = NewWeather;

    if (UNarutoGameInstance* GI = UNarutoGameInstance::Get(this))
    {
        if (UNarutoEventBus* Bus = GI->GetEventBus())
        {
            Bus->OnWeatherChanged.Broadcast(NewWeather);
        }
    }
}

void ANarutoGameState::SetCurrentSeason(ESeason NewSeason)
{
    if (CurrentSeason == NewSeason) return;

    CurrentSeason = NewSeason;

    if (UNarutoGameInstance* GI = UNarutoGameInstance::Get(this))
    {
        if (UNarutoEventBus* Bus = GI->GetEventBus())
        {
            Bus->OnSeasonChanged.Broadcast(NewSeason);
        }
    }
}

void ANarutoGameState::SetEncounterActive(bool bActive)
{
    if (bEncounterActive == bActive) return;

    bEncounterActive = bActive;
    OnActiveEncounterChanged.Broadcast(bActive);
}

void ANarutoGameState::AddWorldEvent(FGameplayTag EventTag)
{
    if (!EventTag.IsValid() || ActiveWorldEventTags.Contains(EventTag)) return;

    ActiveWorldEventTags.Add(EventTag);

    if (UNarutoGameInstance* GI = UNarutoGameInstance::Get(this))
    {
        if (UNarutoEventBus* Bus = GI->GetEventBus())
        {
            FWorldStateChangedEvent Evt;
            Evt.StateTag   = EventTag;
            Evt.bNewValue  = true;
            Evt.Timestamp  = GetWorld()->GetTimeSeconds();
            Bus->OnWorldStateChanged.Broadcast(Evt);
        }
    }
}

void ANarutoGameState::RemoveWorldEvent(FGameplayTag EventTag)
{
    if (!ActiveWorldEventTags.Remove(EventTag)) return;

    if (UNarutoGameInstance* GI = UNarutoGameInstance::Get(this))
    {
        if (UNarutoEventBus* Bus = GI->GetEventBus())
        {
            FWorldStateChangedEvent Evt;
            Evt.StateTag   = EventTag;
            Evt.bNewValue  = false;
            Evt.Timestamp  = GetWorld()->GetTimeSeconds();
            Bus->OnWorldStateChanged.Broadcast(Evt);
        }
    }
}

bool ANarutoGameState::HasWorldEvent(FGameplayTag EventTag) const
{
    return ActiveWorldEventTags.Contains(EventTag);
}
