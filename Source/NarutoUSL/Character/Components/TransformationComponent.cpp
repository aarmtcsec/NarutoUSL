// Copyright (c) 2025 Maze Runner Studios. All Rights Reserved.

#include "Character/Components/TransformationComponent.h"
#include "Character/Base/NarutoCharacterBase.h"
#include "Character/Components/ChakraComponent.h"
#include "NarutoUSL.h"

UTransformationComponent::UTransformationComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}

void UTransformationComponent::InitializeTransformations(
    const TArray<FAwakeningData>& InAwakenings)
{
    Awakenings = InAwakenings;
}

bool UTransformationComponent::ActivateTransformation(int32 Index)
{
    if (!Awakenings.IsValidIndex(Index)) return false;

    const FAwakeningData& Data = Awakenings[Index];
    ANarutoCharacterBase* Owner = Cast<ANarutoCharacterBase>(GetOwner());
    if (!Owner) return false;

    UChakraComponent* Chakra = Owner->GetChakraComponent();
    if (!Chakra) return false;

    // Check minimum chakra requirement
    if (Chakra->GetChakraPercent() < Data.MinChakraPercent) return false;

    const int32 OldIndex = ActiveTransformationIndex;
    ActiveTransformationIndex = Index;

    // Switch to drain regen mode
    Chakra->SetRegenMode(EChakraRegenMode::Drain);

    OnTransformationChanged.Broadcast(OldIndex, Index);

    UE_LOG(LogNarutoUSL, Log, TEXT("[TransformationComponent] %s activated transformation %d: %s"),
        *Owner->GetName(), Index, *Data.DisplayName.ToString());

    return true;
}

void UTransformationComponent::DeactivateTransformation()
{
    if (ActiveTransformationIndex < 0) return;

    const int32 OldIndex = ActiveTransformationIndex;
    ActiveTransformationIndex = -1;

    ANarutoCharacterBase* Owner = Cast<ANarutoCharacterBase>(GetOwner());
    if (Owner)
    {
        UChakraComponent* Chakra = Owner->GetChakraComponent();
        if (Chakra) Chakra->SetRegenMode(EChakraRegenMode::Passive);
    }

    OnTransformationChanged.Broadcast(OldIndex, -1);
}

const FAwakeningData* UTransformationComponent::GetActiveTransformation() const
{
    return Awakenings.IsValidIndex(ActiveTransformationIndex)
        ? &Awakenings[ActiveTransformationIndex] : nullptr;
}

void UTransformationComponent::TickTransformation(float DeltaTime)
{
    if (!IsTransformed()) return;

    ANarutoCharacterBase* Owner = Cast<ANarutoCharacterBase>(GetOwner());
    if (!Owner) return;

    UChakraComponent* Chakra = Owner->GetChakraComponent();
    if (!Chakra) return;

    // Auto-deactivate when chakra is depleted
    if (Chakra->IsDepleted())
    {
        DeactivateTransformation();
    }
}
