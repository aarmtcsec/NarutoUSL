// Copyright (c) 2025 Maze Runner Studios. All Rights Reserved.
// Naruto: Ultimate Shinobi Legacy
// TransformationComponent — Manages awakening/transformation state transitions.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Core/Types/NarutoTypes.h"
#include "TransformationComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnTransformationChanged,
    int32, OldIndex, int32, NewIndex);

UCLASS(ClassGroup = "NarutoUSL|Character", meta = (BlueprintSpawnableComponent))
class NARUTOUSL_API UTransformationComponent : public UActorComponent
{
    GENERATED_BODY()

public:

    UTransformationComponent();

    void InitializeTransformations(const TArray<FAwakeningData>& InAwakenings);

    UFUNCTION(BlueprintCallable, Category = "Transformation")
    bool ActivateTransformation(int32 Index);

    UFUNCTION(BlueprintCallable, Category = "Transformation")
    void DeactivateTransformation();

    UFUNCTION(BlueprintPure, Category = "Transformation")
    bool IsTransformed() const { return ActiveTransformationIndex >= 0; }

    UFUNCTION(BlueprintPure, Category = "Transformation")
    int32 GetActiveTransformationIndex() const { return ActiveTransformationIndex; }

    UFUNCTION(BlueprintPure, Category = "Transformation")
    const FAwakeningData* GetActiveTransformation() const;

    UFUNCTION(BlueprintPure, Category = "Transformation")
    int32 GetTransformationCount() const { return Awakenings.Num(); }

    /** Called each tick to drain chakra during transformation. */
    void TickTransformation(float DeltaTime);

    UPROPERTY(BlueprintAssignable, Category = "Transformation")
    FOnTransformationChanged OnTransformationChanged;

private:

    TArray<FAwakeningData> Awakenings;
    int32 ActiveTransformationIndex = -1;
};
