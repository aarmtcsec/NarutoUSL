// Copyright (c) 2025 Maze Runner Studios. All Rights Reserved.
// Naruto: Ultimate Shinobi Legacy
// InventoryComponent — Item storage, stacking, and consumption.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayTagContainer.h"
#include "InventoryComponent.generated.h"

USTRUCT(BlueprintType)
struct NARUTOUSL_API FInventorySlot
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly) FGameplayTag ItemTag;
    UPROPERTY(BlueprintReadOnly) int32        Quantity = 0;
    UPROPERTY(BlueprintReadOnly) int32        MaxStack = 99;

    bool IsEmpty() const { return Quantity <= 0; }
    bool IsFull()  const { return Quantity >= MaxStack; }
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnInventoryChanged,
    FGameplayTag, ItemTag, int32, NewQuantity);

UCLASS(ClassGroup = "NarutoUSL|Character", meta = (BlueprintSpawnableComponent))
class NARUTOUSL_API UInventoryComponent : public UActorComponent
{
    GENERATED_BODY()

public:

    UInventoryComponent();

    UFUNCTION(BlueprintCallable, Category = "Inventory")
    bool AddItem(FGameplayTag ItemTag, int32 Quantity = 1);

    UFUNCTION(BlueprintCallable, Category = "Inventory")
    bool RemoveItem(FGameplayTag ItemTag, int32 Quantity = 1);

    UFUNCTION(BlueprintCallable, Category = "Inventory")
    bool UseItem(FGameplayTag ItemTag);

    UFUNCTION(BlueprintPure, Category = "Inventory")
    int32 GetItemQuantity(FGameplayTag ItemTag) const;

    UFUNCTION(BlueprintPure, Category = "Inventory")
    bool HasItem(FGameplayTag ItemTag, int32 Quantity = 1) const;

    UFUNCTION(BlueprintPure, Category = "Inventory")
    TArray<FInventorySlot> GetAllItems() const;

    UPROPERTY(BlueprintAssignable, Category = "Inventory")
    FOnInventoryChanged OnInventoryChanged;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory",
        meta = (ClampMin = "1"))
    int32 MaxSlots = 50;

private:

    TMap<FGameplayTag, FInventorySlot> Items;
};
