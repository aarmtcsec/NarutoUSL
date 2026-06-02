// Copyright (c) 2025 Maze Runner Studios. All Rights Reserved.

#include "Narrative/Core/WorldStateManager.h"
#include "Core/Events/NarutoEventBus.h"
#include "NarutoUSL.h"

void UWorldStateManager::Initialize(UNarutoEventBus* InEventBus)
{
    EventBus     = InEventBus;
    bInitialized = true;
    UE_LOG(LogNarutoNarrative, Log, TEXT("[WorldStateManager] Initialized."));
}

void UWorldStateManager::Shutdown()
{
    ActiveFlags.Reset();
}

void UWorldStateManager::SetFlag(FGameplayTag Flag, bool bValue)
{
    if (!Flag.IsValid()) return;

    const bool bWasSet = ActiveFlags.HasTag(Flag);
    if (bWasSet == bValue) return;

    if (bValue)
        ActiveFlags.AddTag(Flag);
    else
        ActiveFlags.RemoveTag(Flag);

    if (EventBus.IsValid())
    {
        FWorldStateChangedEvent Evt;
        Evt.StateTag  = Flag;
        Evt.bNewValue = bValue;
        EventBus->OnWorldStateChanged.Broadcast(Evt);
    }

    UE_LOG(LogNarutoNarrative, Log,
        TEXT("[WorldStateManager] Flag %s = %s"),
        *Flag.ToString(), bValue ? TEXT("TRUE") : TEXT("FALSE"));
}

bool UWorldStateManager::GetFlag(FGameplayTag Flag) const
{
    return ActiveFlags.HasTag(Flag);
}

bool UWorldStateManager::HasAllFlags(const FGameplayTagContainer& Flags) const
{
    return ActiveFlags.HasAll(Flags);
}

bool UWorldStateManager::HasAnyFlag(const FGameplayTagContainer& Flags) const
{
    return ActiveFlags.HasAny(Flags);
}

void UWorldStateManager::SetFlags(const FGameplayTagContainer& Flags, bool bValue)
{
    for (const FGameplayTag& Tag : Flags)
    {
        SetFlag(Tag, bValue);
    }
}

FGameplayTagContainer UWorldStateManager::GetAllActiveFlags() const
{
    return ActiveFlags;
}

void UWorldStateManager::SerializeToSave(FArchive& Ar)
{
    // Serialize active flags as an array of tag strings
    TArray<FString> TagStrings;
    if (Ar.IsSaving())
    {
        for (const FGameplayTag& Tag : ActiveFlags)
        {
            TagStrings.Add(Tag.ToString());
        }
    }
    Ar << TagStrings;
    if (Ar.IsLoading())
    {
        ActiveFlags.Reset();
        for (const FString& TagStr : TagStrings)
        {
            FGameplayTag Tag = FGameplayTag::RequestGameplayTag(FName(*TagStr), false);
            if (Tag.IsValid()) ActiveFlags.AddTag(Tag);
        }
    }
}

void UWorldStateManager::DeserializeFromSave(FArchive& Ar)
{
    SerializeToSave(Ar);
}
