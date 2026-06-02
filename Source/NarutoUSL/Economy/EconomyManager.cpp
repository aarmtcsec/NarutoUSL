// Copyright (c) 2025 Maze Runner Studios. All Rights Reserved.

#include "Economy/EconomyManager.h"
#include "Narrative/Faction/FactionManager.h"
#include "Core/Events/NarutoEventBus.h"
#include "NarutoUSL.h"

UEconomyManager::UEconomyManager()
{
    SubsystemName = TEXT("EconomyManager");
}

void UEconomyManager::Initialize(UFactionManager* InFactionManager,
                                   UNarutoEventBus* InEventBus)
{
    FactionManager = InFactionManager;
    EventBus       = InEventBus;
    bInitialized   = true;
    UE_LOG(LogNarutoUSL, Log, TEXT("[EconomyManager] Initialized."));
}

void UEconomyManager::Shutdown()
{
    ItemRegistry.Empty();
}

// ============================================================
//  Currency
// ============================================================

void UEconomyManager::AddRyo(int64 Amount)
{
    if (Amount <= 0) return;
    PlayerRyo += Amount;
    UE_LOG(LogNarutoUSL, Verbose,
        TEXT("[EconomyManager] +%lld Ryo. Total: %lld"), Amount, PlayerRyo);
}

bool UEconomyManager::SpendRyo(int64 Amount)
{
    if (!CanAfford(Amount)) return false;
    PlayerRyo -= Amount;
    return true;
}

bool UEconomyManager::CanAfford(int64 Amount) const
{
    return PlayerRyo >= Amount;
}

// ============================================================
//  Items
// ============================================================

void UEconomyManager::RegisterItem(const FItemData& Item)
{
    if (Item.ItemTag.IsValid())
    {
        ItemRegistry.Add(Item.ItemTag, Item);
    }
}

const FItemData* UEconomyManager::GetItemData(FGameplayTag ItemTag) const
{
    return ItemRegistry.Find(ItemTag);
}

// ============================================================
//  Shop
// ============================================================

int64 UEconomyManager::GetBuyPrice(FGameplayTag ItemTag, EVillage ShopFaction) const
{
    const FItemData* Item = GetItemData(ItemTag);
    if (!Item) return 0;

    const float Discount = GetReputationDiscount(ShopFaction);
    return FMath::Max(1LL, static_cast<int64>(Item->BasePrice * (1.0f - Discount)));
}

int64 UEconomyManager::GetSellPrice(FGameplayTag ItemTag) const
{
    const FItemData* Item = GetItemData(ItemTag);
    if (!Item) return 0;
    return FMath::Max(1LL, static_cast<int64>(Item->BasePrice * SellPriceFraction));
}

bool UEconomyManager::BuyItem(FGameplayTag ItemTag, EVillage ShopFaction, int32 Quantity)
{
    const int64 TotalCost = GetBuyPrice(ItemTag, ShopFaction) * Quantity;
    if (!SpendRyo(TotalCost)) return false;

    UE_LOG(LogNarutoUSL, Log,
        TEXT("[EconomyManager] Bought %dx %s for %lld Ryo."),
        Quantity, *ItemTag.ToString(), TotalCost);
    return true;
}

bool UEconomyManager::SellItem(FGameplayTag ItemTag, int32 Quantity)
{
    const int64 TotalGain = GetSellPrice(ItemTag) * Quantity;
    AddRyo(TotalGain);

    UE_LOG(LogNarutoUSL, Log,
        TEXT("[EconomyManager] Sold %dx %s for %lld Ryo."),
        Quantity, *ItemTag.ToString(), TotalGain);
    return true;
}

// ============================================================
//  Loot
// ============================================================

TArray<TPair<FGameplayTag, int32>> UEconomyManager::RollLootTable(
    const TArray<FLootEntry>& LootTable, float LuckBonus)
{
    TArray<TPair<FGameplayTag, int32>> Drops;

    for (const FLootEntry& Entry : LootTable)
    {
        const float AdjustedChance = FMath::Clamp(Entry.DropChance * LuckBonus, 0.0f, 1.0f);
        if (FMath::FRand() <= AdjustedChance)
        {
            const int32 Qty = FMath::RandRange(Entry.MinQuantity, Entry.MaxQuantity);
            Drops.Add(TPair<FGameplayTag, int32>(Entry.ItemTag, Qty));
        }
    }

    return Drops;
}

void UEconomyManager::GrantCombatRewards(float XPAmount, int64 RyoMin, int64 RyoMax)
{
    const int64 RyoDrop = FMath::RandRange(RyoMin, RyoMax);
    AddRyo(RyoDrop);

    // XP granted via ProgressionManager in full implementation
    UE_LOG(LogNarutoUSL, Log,
        TEXT("[EconomyManager] Combat rewards: %.0f XP, %lld Ryo."), XPAmount, RyoDrop);
}

void UEconomyManager::GrantQuestRewards(float XPAmount, int64 RyoAmount,
                                          const TArray<FGameplayTag>& ItemRewards)
{
    AddRyo(RyoAmount);

    UE_LOG(LogNarutoUSL, Log,
        TEXT("[EconomyManager] Quest rewards: %.0f XP, %lld Ryo, %d items."),
        XPAmount, RyoAmount, ItemRewards.Num());
}

// ============================================================
//  Internal
// ============================================================

float UEconomyManager::GetReputationDiscount(EVillage Faction) const
{
    if (!FactionManager.IsValid()) return 0.0f;

    const EReputationTier Tier = FactionManager->GetReputationTier(Faction);
    switch (Tier)
    {
        case EReputationTier::Friendly:  return 0.05f;
        case EReputationTier::Honored:   return 0.10f;
        case EReputationTier::Revered:   return 0.15f;
        case EReputationTier::Legendary: return MaxReputationDiscount;
        default:                         return 0.0f;
    }
}

void UEconomyManager::SerializeToSave(FArchive& Ar)
{
    Ar << PlayerRyo;
}

void UEconomyManager::DeserializeFromSave(FArchive& Ar)
{
    Ar << PlayerRyo;
}
