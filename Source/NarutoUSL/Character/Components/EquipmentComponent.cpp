// Copyright (c) 2025 Maze Runner Studios. All Rights Reserved.

#include "Character/Components/EquipmentComponent.h"
#include "Character/Base/NarutoCharacterBase.h"
#include "Character/Components/ProgressionComponent.h"
#include "NarutoUSL.h"

UEquipmentComponent::UEquipmentComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}

bool UEquipmentComponent::EquipItem(EEquipmentSlot Slot, FGameplayTag ItemTag,
                                      const TArray<FStatModifier>& Modifiers)
{
    if (!ItemTag.IsValid()) return false;

    // Unequip existing item in slot first
    UnequipSlot(Slot);

    FEquipmentSlotData& SlotData = Slots.FindOrAdd(Slot);
    SlotData.Slot          = Slot;
    SlotData.ItemTag       = ItemTag;
    SlotData.bOccupied     = true;
    SlotData.StatModifiers = Modifiers;

    // Apply modifiers to progression component
    ANarutoCharacterBase* Owner = Cast<ANarutoCharacterBase>(GetOwner());
    if (Owner && Owner->GetProgressionComponent())
    {
        for (const FStatModifier& Mod : Modifiers)
        {
            Owner->GetProgressionComponent()->AddStatModifier(Mod);
        }
    }

    OnEquipmentChanged.Broadcast(Slot, ItemTag);
    return true;
}

bool UEquipmentComponent::UnequipSlot(EEquipmentSlot Slot)
{
    FEquipmentSlotData* SlotData = Slots.Find(Slot);
    if (!SlotData || !SlotData->bOccupied) return false;

    // Remove modifiers from progression component
    ANarutoCharacterBase* Owner = Cast<ANarutoCharacterBase>(GetOwner());
    if (Owner && Owner->GetProgressionComponent())
    {
        for (const FStatModifier& Mod : SlotData->StatModifiers)
        {
            Owner->GetProgressionComponent()->RemoveStatModifiersFromSource(Mod.SourceTag);
        }
    }

    const FGameplayTag OldTag = SlotData->ItemTag;
    SlotData->ItemTag       = FGameplayTag();
    SlotData->bOccupied     = false;
    SlotData->StatModifiers.Empty();

    OnEquipmentChanged.Broadcast(Slot, FGameplayTag());
    return true;
}

FGameplayTag UEquipmentComponent::GetEquippedItem(EEquipmentSlot Slot) const
{
    const FEquipmentSlotData* SlotData = Slots.Find(Slot);
    return SlotData && SlotData->bOccupied ? SlotData->ItemTag : FGameplayTag();
}

bool UEquipmentComponent::IsSlotOccupied(EEquipmentSlot Slot) const
{
    const FEquipmentSlotData* SlotData = Slots.Find(Slot);
    return SlotData && SlotData->bOccupied;
}

TArray<FStatModifier> UEquipmentComponent::GetAllEquipmentModifiers() const
{
    TArray<FStatModifier> All;
    for (const auto& Pair : Slots)
    {
        if (Pair.Value.bOccupied)
        {
            All.Append(Pair.Value.StatModifiers);
        }
    }
    return All;
}
