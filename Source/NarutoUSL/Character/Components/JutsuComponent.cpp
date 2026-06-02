// Copyright (c) 2025 Maze Runner Studios. All Rights Reserved.

#include "Character/Components/JutsuComponent.h"
#include "Character/Base/NarutoCharacterBase.h"
#include "Character/Components/ProgressionComponent.h"
#include "NarutoUSL.h"

UJutsuComponent::UJutsuComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
    LoadoutSlots.SetNum(MaxLoadoutSlots);
}

void UJutsuComponent::BeginPlay()
{
    Super::BeginPlay();
}

void UJutsuComponent::InitializeFromCharacterData(
    const TArray<TSoftObjectPtr<UJutsuData>>& DefaultJutsu,
    const TArray<TSoftObjectPtr<UJutsuData>>& EquippedJutsu)
{
    // Unlock all default jutsu
    for (const TSoftObjectPtr<UJutsuData>& JutsuRef : DefaultJutsu)
    {
        UJutsuData* Data = JutsuRef.LoadSynchronous();
        if (Data) UnlockJutsu(Data);
    }

    // Assign equipped jutsu to loadout slots
    for (int32 i = 0; i < EquippedJutsu.Num() && i < MaxLoadoutSlots; ++i)
    {
        UJutsuData* Data = EquippedJutsu[i].LoadSynchronous();
        if (Data && Data->JutsuTag.IsValid())
        {
            AssignJutsuToSlot(i, Data->JutsuTag);
        }
    }
}

// ============================================================
//  Registry
// ============================================================

void UJutsuComponent::UnlockJutsu(UJutsuData* JutsuData)
{
    if (!JutsuData || !JutsuData->JutsuTag.IsValid()) return;
    if (KnowsJutsu(JutsuData->JutsuTag)) return;

    KnownJutsuMap.Add(JutsuData->JutsuTag, JutsuData);
    OnJutsuUnlocked.Broadcast(JutsuData->JutsuTag);

    UE_LOG(LogNarutoJutsu, Log, TEXT("[JutsuComponent] %s unlocked jutsu: %s"),
        *GetOwner()->GetName(), *JutsuData->DisplayName.ToString());
}

bool UJutsuComponent::KnowsJutsu(FGameplayTag JutsuTag) const
{
    return KnownJutsuMap.Contains(JutsuTag);
}

TArray<UJutsuData*> UJutsuComponent::GetKnownJutsu() const
{
    TArray<UJutsuData*> Result;
    for (const auto& Pair : KnownJutsuMap)
    {
        if (Pair.Value) Result.Add(Pair.Value);
    }
    return Result;
}

UJutsuData* UJutsuComponent::GetJutsuDataByTag(FGameplayTag JutsuTag) const
{
    const TObjectPtr<UJutsuData>* Found = KnownJutsuMap.Find(JutsuTag);
    return Found ? Found->Get() : nullptr;
}

// ============================================================
//  Loadout
// ============================================================

bool UJutsuComponent::AssignJutsuToSlot(int32 SlotIndex, FGameplayTag JutsuTag)
{
    if (!LoadoutSlots.IsValidIndex(SlotIndex)) return false;
    if (!KnowsJutsu(JutsuTag)) return false;

    LoadoutSlots[SlotIndex] = JutsuTag;
    OnLoadoutChanged.Broadcast(SlotIndex, JutsuTag);
    return true;
}

void UJutsuComponent::ClearSlot(int32 SlotIndex)
{
    if (LoadoutSlots.IsValidIndex(SlotIndex))
    {
        LoadoutSlots[SlotIndex] = FGameplayTag();
        OnLoadoutChanged.Broadcast(SlotIndex, FGameplayTag());
    }
}

FGameplayTag UJutsuComponent::GetJutsuInSlot(int32 SlotIndex) const
{
    return LoadoutSlots.IsValidIndex(SlotIndex) ? LoadoutSlots[SlotIndex] : FGameplayTag();
}

UJutsuData* UJutsuComponent::GetJutsuDataInSlot(int32 SlotIndex) const
{
    const FGameplayTag Tag = GetJutsuInSlot(SlotIndex);
    return Tag.IsValid() ? GetJutsuDataByTag(Tag) : nullptr;
}

int32 UJutsuComponent::GetSlotForJutsu(FGameplayTag JutsuTag) const
{
    return LoadoutSlots.IndexOfByKey(JutsuTag);
}

// ============================================================
//  Cooldowns
// ============================================================

bool UJutsuComponent::IsOnCooldown(FGameplayTag JutsuTag) const
{
    const FJutsuCooldownEntry* Entry = Cooldowns.Find(JutsuTag);
    return Entry && Entry->IsOnCooldown();
}

float UJutsuComponent::GetCooldownRemaining(FGameplayTag JutsuTag) const
{
    const FJutsuCooldownEntry* Entry = Cooldowns.Find(JutsuTag);
    return Entry ? Entry->CooldownRemaining : 0.0f;
}

float UJutsuComponent::GetCooldownProgress(FGameplayTag JutsuTag) const
{
    const FJutsuCooldownEntry* Entry = Cooldowns.Find(JutsuTag);
    return Entry ? Entry->GetProgress() : 1.0f;
}

void UJutsuComponent::StartCooldown(FGameplayTag JutsuTag, float Duration)
{
    if (!JutsuTag.IsValid() || Duration <= 0.0f) return;

    FJutsuCooldownEntry& Entry = Cooldowns.FindOrAdd(JutsuTag);
    Entry.JutsuTag          = JutsuTag;
    Entry.CooldownRemaining = Duration;
    Entry.TotalCooldown     = Duration;

    OnJutsuCooldownStarted.Broadcast(JutsuTag, Duration);
}

void UJutsuComponent::ResetCooldown(FGameplayTag JutsuTag)
{
    FJutsuCooldownEntry* Entry = Cooldowns.Find(JutsuTag);
    if (Entry)
    {
        Entry->CooldownRemaining = 0.0f;
        OnJutsuReady.Broadcast(JutsuTag);
    }
}

void UJutsuComponent::ResetAllCooldowns()
{
    for (auto& Pair : Cooldowns)
    {
        Pair.Value.CooldownRemaining = 0.0f;
        OnJutsuReady.Broadcast(Pair.Key);
    }
}

void UJutsuComponent::TickCooldowns(float DeltaTime)
{
    for (auto& Pair : Cooldowns)
    {
        if (Pair.Value.CooldownRemaining <= 0.0f) continue;

        Pair.Value.CooldownRemaining -= DeltaTime;
        if (Pair.Value.CooldownRemaining <= 0.0f)
        {
            Pair.Value.CooldownRemaining = 0.0f;
            OnJutsuReady.Broadcast(Pair.Key);
        }
    }
}

// ============================================================
//  Cast Validation
// ============================================================

bool UJutsuComponent::CanCast(FGameplayTag JutsuTag) const
{
    if (!KnowsJutsu(JutsuTag)) return false;
    if (IsOnCooldown(JutsuTag)) return false;

    ANarutoCharacterBase* Owner = Cast<ANarutoCharacterBase>(GetOwner());
    if (!Owner || Owner->IsDead_Implementation()) return false;

    const UJutsuData* Data = GetJutsuDataByTag(JutsuTag);
    if (!Data) return false;

    // Check chakra cost
    return Owner->CanAffordChakraCost_Implementation(Data->ChakraCost);
}
