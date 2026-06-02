// Copyright (c) 2025 Maze Runner Studios. All Rights Reserved.
// Naruto: Ultimate Shinobi Legacy
// Interface: IInteractable

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "IInteractable.generated.h"

class ANarutoPlayerCharacter;

UENUM(BlueprintType)
enum class EInteractionType : uint8
{
    Talk        UMETA(DisplayName = "Talk"),
    Examine     UMETA(DisplayName = "Examine"),
    PickUp      UMETA(DisplayName = "Pick Up"),
    Activate    UMETA(DisplayName = "Activate"),
    Trade       UMETA(DisplayName = "Trade"),
    Train       UMETA(DisplayName = "Train"),
    BoardVehicle UMETA(DisplayName = "Board Vehicle"),
    EnterArea   UMETA(DisplayName = "Enter Area"),
};

UINTERFACE(MinimalAPI, BlueprintType)
class UInteractable : public UInterface
{
    GENERATED_BODY()
};

/**
 * IInteractable
 *
 * Implemented by any world object the player can interact with:
 * NPCs, doors, chests, training dummies, mission boards, etc.
 */
class NARUTOUSL_API IInteractable
{
    GENERATED_BODY()

public:

    /** Returns true if the specified player can currently interact with this object. */
    UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Interaction")
    bool CanInteract(ANarutoPlayerCharacter* Interactor) const;

    /** Returns the type of interaction this object offers. */
    UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Interaction")
    EInteractionType GetInteractionType() const;

    /** Returns the localized prompt text shown in the interaction UI. */
    UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Interaction")
    FText GetInteractionPrompt(ANarutoPlayerCharacter* Interactor) const;

    /** Returns the world-space location of the interaction point. */
    UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Interaction")
    FVector GetInteractionLocation() const;

    /** Returns the maximum distance at which interaction is available. */
    UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Interaction")
    float GetInteractionRadius() const;

    /**
     * Executes the interaction.
     * @param Interactor The player initiating the interaction.
     */
    UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Interaction")
    void Interact(ANarutoPlayerCharacter* Interactor);

    /** Called when the player enters interaction range. */
    UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Interaction")
    void OnInteractionRangeEntered(ANarutoPlayerCharacter* Interactor);

    /** Called when the player exits interaction range. */
    UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Interaction")
    void OnInteractionRangeExited(ANarutoPlayerCharacter* Interactor);
};
