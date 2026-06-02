// Copyright (c) 2025 Maze Runner Studios. All Rights Reserved.
// Naruto: Ultimate Shinobi Legacy
// EquipmentComponent — Manages equipped weapons, armor, and accessories.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayTagContainer.h"
#include "Core/Types/NarutoTypes.h"
#include "EquipmentComponent.generated.h"

UENUM(BlueprintType)
enum class EEquipmentSlot : uint8
{
    Weapon      UMETA(DisplayName = "Weapon"),
    OffHand     UMETA(DisplayName = "Off-Hand"),
    Head        UMETA(DisplayName = "Head"),
    Body        UMETA(DisplayName = "Body"),
    Accessory1  UMETA(DisplayName = "Accessory 1"),
    Accessory2  UMETA(DisplayName = "Accessory 2"),
    Accessory3  UMETA(DisplayName = "Accessory 3"),
};

USTRUCT(BlueprintType)
struct NARUTOUSL_API FEquipmentSlotData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly) EEquipmentSlot Slot     = EEquipmentSlot::Weapon;
    UPROPERTY(BlueprintReadOnly) FGameplayTag   ItemTag;
    UPROPERTY(BlueprintReadOnly) bool           bOccupied = false;

    /** Stat modifiers granted by this equipment piece. */
    UPROPERTY(BlueprintReadOnly) TArray<FStatModifier> StatModifiers;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnEquipmentChanged,
    EEquipmentSlot, Slot, FGameplayTag, NewItemTag);

UCLASS(ClassGroup = "NarutoUSL|Character", meta = (BlueprintSpawnableComponent))
class NARUTOUSL_API UEquipmentComponent : public UActorComponent
{
    GENERATED_BODY()

public:

    UEquipmentComponent();

    UFUNCTION(BlueprintCallable, Category = "Equipment")
    bool EquipItem(EEquipmentSlot Slot, FGameplayTag ItemTag,
                   const TArray<FStatModifier>& Modifiers);

    UFUNCTION(BlueprintCallable, Category = "Equipment")
    bool UnequipSlot(EEquipmentSlot Slot);

    UFUNCTION(BlueprintPure, Category = "Equipment")
    FGameplayTag GetEquippedItem(EEquipmentSlot Slot) const;

    UFUNCTION(BlueprintPure, Category = "Equipment")
    bool IsSlotOccupied(EEquipmentSlot Slot) const;

    UFUNCTION(BlueprintPure, Category = "Equipment")
    TArray<FStatModifier> GetAllEquipmentModifiers() const;

    UPROPERTY(BlueprintAssignable, Category = "Equipment")
    FOnEquipmentChanged OnEquipmentChanged;

private:

    TMap<EEquipmentSlot, FEquipmentSlotData> Slots;
};
