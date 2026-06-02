// Copyright (c) 2025 Maze Runner Studios. All Rights Reserved.
// Naruto: Ultimate Shinobi Legacy
// EconomyManager — Currency, trade, loot distribution, and shop pricing.

#pragma once

#include "CoreMinimal.h"
#include "Core/Subsystems/NarutoSubsystem.h"
#include "Core/Types/NarutoTypes.h"
#include "GameplayTagContainer.h"
#include "EconomyManager.generated.h"

class UFactionManager;
class UNarutoEventBus;

// ============================================================
//  Item Definition
// ============================================================

UENUM(BlueprintType)
enum class EItemRarity : uint8
{
    Common      UMETA(DisplayName = "Common"),
    Uncommon    UMETA(DisplayName = "Uncommon"),
    Rare        UMETA(DisplayName = "Rare"),
    Epic        UMETA(DisplayName = "Epic"),
    Legendary   UMETA(DisplayName = "Legendary"),
    Unique      UMETA(DisplayName = "Unique"),
};

USTRUCT(BlueprintType)
struct NARUTOUSL_API FItemData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadOnly) FGameplayTag  ItemTag;
    UPROPERTY(EditAnywhere, BlueprintReadOnly) FText         DisplayName;
    UPROPERTY(EditAnywhere, BlueprintReadOnly) FText         Description;
    UPROPERTY(EditAnywhere, BlueprintReadOnly) EItemRarity   Rarity       = EItemRarity::Common;
    UPROPERTY(EditAnywhere, BlueprintReadOnly) int64         BasePrice    = 100;
    UPROPERTY(EditAnywhere, BlueprintReadOnly) int32         MaxStack     = 99;
    UPROPERTY(EditAnywhere, BlueprintReadOnly) bool          bConsumable  = false;
    UPROPERTY(EditAnywhere, BlueprintReadOnly) TSoftObjectPtr<UTexture2D> Icon;

    /** Stat modifiers granted when equipped (for equipment items). */
    UPROPERTY(EditAnywhere, BlueprintReadOnly) TArray<FStatModifier> StatModifiers;

    /** Effect tags applied when consumed (for consumable items). */
    UPROPERTY(EditAnywhere, BlueprintReadOnly) FGameplayTagContainer ConsumeEffectTags;
};

// ============================================================
//  Loot Roll
// ============================================================

USTRUCT(BlueprintType)
struct NARUTOUSL_API FLootEntry
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadOnly) FGameplayTag ItemTag;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (ClampMin = "0", ClampMax = "1"))
    float DropChance = 0.1f;
    UPROPERTY(EditAnywhere, BlueprintReadOnly) int32 MinQuantity = 1;
    UPROPERTY(EditAnywhere, BlueprintReadOnly) int32 MaxQuantity = 1;
};

// ============================================================
//  EconomyManager
// ============================================================

/**
 * UEconomyManager
 *
 * Manages all economic systems:
 *   - Player currency (Ryo) balance
 *   - Item registry and pricing
 *   - Loot table rolling
 *   - Shop buy/sell with faction reputation discounts
 *   - Supply/demand price fluctuation
 *   - Reward distribution from quests and combat
 */
UCLASS(NotBlueprintable)
class NARUTOUSL_API UEconomyManager : public UNarutoSubsystem
{
    GENERATED_BODY()

public:

    UEconomyManager();

    void Initialize(UFactionManager* InFactionManager, UNarutoEventBus* InEventBus);
    virtual void Shutdown() override;
    virtual void SerializeToSave(FArchive& Ar) override;
    virtual void DeserializeFromSave(FArchive& Ar) override;

    // ------------------------------------------------------------------
    //  Currency
    // ------------------------------------------------------------------

    UFUNCTION(BlueprintPure,     Category = "Economy") int64 GetRyo() const { return PlayerRyo; }
    UFUNCTION(BlueprintCallable, Category = "Economy") void  AddRyo(int64 Amount);
    UFUNCTION(BlueprintCallable, Category = "Economy") bool  SpendRyo(int64 Amount);
    UFUNCTION(BlueprintPure,     Category = "Economy") bool  CanAfford(int64 Amount) const;

    // ------------------------------------------------------------------
    //  Items
    // ------------------------------------------------------------------

    void RegisterItem(const FItemData& Item);
    const FItemData* GetItemData(FGameplayTag ItemTag) const;

    // ------------------------------------------------------------------
    //  Shop
    // ------------------------------------------------------------------

    /**
     * Returns the buy price for an item, adjusted for faction reputation.
     * Higher reputation = lower prices (up to 20% discount at Legendary).
     */
    UFUNCTION(BlueprintPure, Category = "Economy")
    int64 GetBuyPrice(FGameplayTag ItemTag, EVillage ShopFaction) const;

    /**
     * Returns the sell price (always lower than buy price).
     */
    UFUNCTION(BlueprintPure, Category = "Economy")
    int64 GetSellPrice(FGameplayTag ItemTag) const;

    UFUNCTION(BlueprintCallable, Category = "Economy")
    bool BuyItem(FGameplayTag ItemTag, EVillage ShopFaction, int32 Quantity = 1);

    UFUNCTION(BlueprintCallable, Category = "Economy")
    bool SellItem(FGameplayTag ItemTag, int32 Quantity = 1);

    // ------------------------------------------------------------------
    //  Loot
    // ------------------------------------------------------------------

    /**
     * Rolls a loot table and returns the items that dropped.
     * @param LootTable  Array of loot entries with drop chances.
     * @param LuckBonus  Multiplier applied to all drop chances (1.0 = normal).
     */
    UFUNCTION(BlueprintCallable, Category = "Economy")
    TArray<TPair<FGameplayTag, int32>> RollLootTable(
        const TArray<FLootEntry>& LootTable, float LuckBonus = 1.0f);

    /**
     * Grants XP and Ryo to the player from a combat kill.
     */
    UFUNCTION(BlueprintCallable, Category = "Economy")
    void GrantCombatRewards(float XPAmount, int64 RyoMin, int64 RyoMax);

    /**
     * Grants quest rewards to the player.
     */
    UFUNCTION(BlueprintCallable, Category = "Economy")
    void GrantQuestRewards(float XPAmount, int64 RyoAmount,
                           const TArray<FGameplayTag>& ItemRewards);

private:

    float GetReputationDiscount(EVillage Faction) const;

    int64 PlayerRyo = 0;

    TMap<FGameplayTag, FItemData> ItemRegistry;

    TWeakObjectPtr<UFactionManager>  FactionManager;
    TWeakObjectPtr<UNarutoEventBus>  EventBus;

    static constexpr float SellPriceFraction = 0.4f;
    static constexpr float MaxReputationDiscount = 0.2f;
};
