// Copyright (c) 2025 Maze Runner Studios. All Rights Reserved.

#include "Character/Player/NarutoPlayerState.h"

ANarutoPlayerState::ANarutoPlayerState()
{
}

void ANarutoPlayerState::SetActiveCharacterTag(FGameplayTag NewTag)
{
    ActiveCharacterTag = NewTag;
}

void ANarutoPlayerState::SetHighestRank(EShinobi_Rank NewRank)
{
    if (NewRank > HighestRank)
    {
        HighestRank = NewRank;
    }
}

void ANarutoPlayerState::RecordKill()
{
    ++TotalKills;
}

void ANarutoPlayerState::RecordDeath()
{
    ++TotalDeaths;
}
