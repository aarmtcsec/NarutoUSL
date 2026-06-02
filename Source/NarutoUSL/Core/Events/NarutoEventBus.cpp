// Copyright (c) 2025 Maze Runner Studios. All Rights Reserved.

#include "Core/Events/NarutoEventBus.h"

void UNarutoEventBus::ClearAllBindings()
{
    OnCharacterDefeated.Clear();
    OnCombatEncounterChanged.Clear();
    OnJutsuExecuted.Clear();
    OnQuestStateChanged.Clear();
    OnReputationChanged.Clear();
    OnWorldStateChanged.Clear();
    OnPlayerLevelUp.Clear();
    OnWeatherChanged.Clear();
    OnSeasonChanged.Clear();
}
