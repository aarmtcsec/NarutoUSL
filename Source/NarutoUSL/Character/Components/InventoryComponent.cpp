// Copyright (c) 2025 Maze Runner Studios. All Rights Reserved.

#include "Character/Components/InventoryComponent.h"
#include "NarutoUSL.h"

UInventoryComponent::UInventoryComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}

bool UInventoryComponent::AddItem(FGameplayTag ItemTag, int32 Quantity)
{
    if (!ItemTag.IsValid() || Quantity <= 0) return false;

    FInventorySlot& Slot = Items.FindOrAdd(ItemTag);
    Slot.ItemTag   = ItemTag;
    Slot.Quantity  = FMath::Min(Slot.Quantity + Quantity, Slot.MaxStack);

    OnInventoryChanged.Broadcast(ItemTag, Slot.Quantity);
    return true;
}

bool UInventoryComponent::RemoveItem(FGameplayTag ItemTag, int32 Quantity)
{
    FInventorySlot* Slot = Items.Find(ItemTag);
    if (!Slot || Slot->Quantity < Quantity) return false;

    Slot->Quantity -= Quantity;
    if (Slot->Quantity <= 0) Items.Remove(ItemTag);

    OnInventoryChanged.Broadcast(ItemTag, FMath::Max(0, Slot->Quantity - Quantity + Quantity));
    return true;
}

bool UInventoryComponent::UseItem(FGameplayTag ItemTag)
{
    if (!HasItem(ItemTag)) return false;
    // Item effect applied by ItemManager in full implementation
    return RemoveItem(ItemTag, 1);
}

int32 UInventoryComponent::GetItemQuantity(FGameplayTag ItemTag) const
{
    const FInventorySlot* Slot = Items.Find(ItemTag);
    return Slot ? Slot->Quantity : 0;
}

bool UInventoryComponent::HasItem(FGameplayTag ItemTag, int32 Quantity) const
{
    return GetItemQuantity(ItemTag) >= Quantity;
}

TArray<FInventorySlot> UInventoryComponent::GetAllItems() const
{
    TArray<FInventorySlot> Result;
    for (const auto& Pair : Items)
    {
        if (!Pair.Value.IsEmpty()) Result.Add(Pair.Value);
    }
    return Result;
}
