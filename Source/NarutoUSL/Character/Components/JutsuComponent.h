// Copyright (c) 2025 Maze Runner Studios. All Rights Reserved.
// Naruto: Ultimate Shinobi Legacy
// JutsuComponent — Per-character jutsu registry, cooldown tracking, and loadout.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayTagContainer.h"
#include "Jutsu/Data/JutsuData.h"
#include "JutsuComponent.generated.h"

// ============================================================
//  Cooldown Entry
// ============================================================

USTRUCT()
struct NARUTOUSL_API FJutsuCooldownEntry
{
    GENERATED_BODY()

    FGameplayTag JutsuTag;
    float        CooldownRemaining = 0.0f;
    float        TotalCooldown     = 0.0f;

    bool IsOnCooldown() const { return CooldownRemaining > 0.0f; }
    float GetProgress() const
    {
        return TotalCooldown > 0.0f
            ? FMath::Clamp(1.0f - (CooldownRemaining / TotalCooldown), 0.0f, 1.0f)
            : 1.0f;
    }
};

// ============================================================
//  Delegates
// ============================================================

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnJutsuUnlocked,    FGameplayTag, JutsuTag);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnJutsuCooldown,   FGameplayTag, JutsuTag, float, Duration);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnJutsuReady,       FGameplayTag, JutsuTag);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnLoadoutChanged,  int32, SlotIndex, FGameplayTag, NewJutsuTag);

// ============================================================
//  JutsuComponent
// ============================================================

/**
 * UJutsuComponent
 *
 * Manages the jutsu registry, cooldowns, and active loadout for a character.
 * The JutsuManager queries this component to validate and execute jutsu.
 *
 * Loadout:
 *   Characters have 8 loadout slots (4 standard + 4 alternate).
 *   Each slot maps to a jutsu tag. The player assigns jutsu to slots
 *   via the skill/loadout UI.
 *
 * Cooldowns:
 *   Tracked per jutsu tag. Ticked by JutsuManager each frame.
 *   Mastery level reduces effective cooldown.
 */
UCLASS(ClassGroup = "NarutoUSL|Character", meta = (BlueprintSpawnableComponent))
class NARUTOUSL_API UJutsuComponent : public UActorComponent
{
    GENERATED_BODY()

public:

    UJutsuComponent();

    virtual void BeginPlay() override;

    static constexpr int32 MaxLoadoutSlots = 8;

    // ------------------------------------------------------------------
    //  Initialization
    // ------------------------------------------------------------------

    void InitializeFromCharacterData(const TArray<TSoftObjectPtr<UJutsuData>>& DefaultJutsu,
                                      const TArray<TSoftObjectPtr<UJutsuData>>& EquippedJutsu);

    // ------------------------------------------------------------------
    //  Registry
    // ------------------------------------------------------------------

    UFUNCTION(BlueprintCallable, Category = "Jutsu")
    void UnlockJutsu(UJutsuData* JutsuData);

    UFUNCTION(BlueprintPure, Category = "Jutsu")
    bool KnowsJutsu(FGameplayTag JutsuTag) const;

    UFUNCTION(BlueprintPure, Category = "Jutsu")
    TArray<UJutsuData*> GetKnownJutsu() const;

    UFUNCTION(BlueprintPure, Category = "Jutsu")
    UJutsuData* GetJutsuDataByTag(FGameplayTag JutsuTag) const;

    // ------------------------------------------------------------------
    //  Loadout
    // ------------------------------------------------------------------

    UFUNCTION(BlueprintCallable, Category = "Jutsu|Loadout")
    bool AssignJutsuToSlot(int32 SlotIndex, FGameplayTag JutsuTag);

    UFUNCTION(BlueprintCallable, Category = "Jutsu|Loadout")
    void ClearSlot(int32 SlotIndex);

    UFUNCTION(BlueprintPure, Category = "Jutsu|Loadout")
    FGameplayTag GetJutsuInSlot(int32 SlotIndex) const;

    UFUNCTION(BlueprintPure, Category = "Jutsu|Loadout")
    UJutsuData* GetJutsuDataInSlot(int32 SlotIndex) const;

    UFUNCTION(BlueprintPure, Category = "Jutsu|Loadout")
    int32 GetSlotForJutsu(FGameplayTag JutsuTag) const;

    // ------------------------------------------------------------------
    //  Cooldowns
    // ------------------------------------------------------------------

    UFUNCTION(BlueprintPure, Category = "Jutsu|Cooldown")
    bool IsOnCooldown(FGameplayTag JutsuTag) const;

    UFUNCTION(BlueprintPure, Category = "Jutsu|Cooldown")
    float GetCooldownRemaining(FGameplayTag JutsuTag) const;

    UFUNCTION(BlueprintPure, Category = "Jutsu|Cooldown")
    float GetCooldownProgress(FGameplayTag JutsuTag) const;

    void StartCooldown(FGameplayTag JutsuTag, float Duration);
    void ResetCooldown(FGameplayTag JutsuTag);
    void ResetAllCooldowns();

    /** Ticks all active cooldowns. Called by JutsuManager. */
    void TickCooldowns(float DeltaTime);

    // ------------------------------------------------------------------
    //  Cast Validation
    // ------------------------------------------------------------------

    UFUNCTION(BlueprintPure, Category = "Jutsu")
    bool CanCast(FGameplayTag JutsuTag) const;

    // ------------------------------------------------------------------
    //  Events
    // ------------------------------------------------------------------

    UPROPERTY(BlueprintAssignable, Category = "Jutsu|Events")
    FOnJutsuUnlocked OnJutsuUnlocked;

    UPROPERTY(BlueprintAssignable, Category = "Jutsu|Events")
    FOnJutsuCooldown OnJutsuCooldownStarted;

    UPROPERTY(BlueprintAssignable, Category = "Jutsu|Events")
    FOnJutsuReady OnJutsuReady;

    UPROPERTY(BlueprintAssignable, Category = "Jutsu|Events")
    FOnLoadoutChanged OnLoadoutChanged;

private:

    /** All known jutsu. Key = JutsuTag. */
    TMap<FGameplayTag, TObjectPtr<UJutsuData>> KnownJutsuMap;

    /** Active cooldowns. Key = JutsuTag. */
    TMap<FGameplayTag, FJutsuCooldownEntry> Cooldowns;

    /** Loadout slots. Index = slot, Value = JutsuTag. */
    TArray<FGameplayTag> LoadoutSlots;
};
