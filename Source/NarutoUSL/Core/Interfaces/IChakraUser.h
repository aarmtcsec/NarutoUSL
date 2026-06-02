// Copyright (c) 2025 Maze Runner Studios. All Rights Reserved.
// Naruto: Ultimate Shinobi Legacy
// Interface: IChakraUser
// Implemented by any actor that uses or stores chakra.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Core/Types/NarutoTypes.h"
#include "IChakraUser.generated.h"

UINTERFACE(MinimalAPI, BlueprintType)
class UChakraUser : public UInterface
{
    GENERATED_BODY()
};

/**
 * IChakraUser
 *
 * Uniform API for all chakra-capable actors. The ChakraSystem queries this
 * interface to read and modify chakra without depending on concrete types.
 * Supports nature affinities, sharing, and transformation drain.
 */
class NARUTOUSL_API IChakraUser
{
    GENERATED_BODY()

public:

    // ------------------------------------------------------------------
    //  Chakra Queries
    // ------------------------------------------------------------------

    UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Chakra")
    float GetCurrentChakra() const;

    UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Chakra")
    float GetMaxChakra() const;

    UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Chakra")
    float GetChakraRegenRate() const;

    /** Returns the bitmask of nature affinities this character possesses. */
    UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Chakra")
    EChakraNature GetNatureAffinities() const;

    /** Returns true if this character has the specified nature affinity. */
    UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Chakra")
    bool HasNatureAffinity(EChakraNature Nature) const;

    /** Returns true if this character can afford the specified chakra cost. */
    UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Chakra")
    bool CanAffordChakraCost(const FChakraCost& Cost) const;

    // ------------------------------------------------------------------
    //  Chakra Modification
    // ------------------------------------------------------------------

    /**
     * Consumes chakra. Returns the actual amount consumed (may be less if
     * insufficient chakra is available and bAllowPartial is true).
     * @param Amount       Amount to consume.
     * @param bAllowPartial If false, fails entirely if insufficient chakra.
     * @return Actual amount consumed. 0 if failed.
     */
    UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Chakra")
    float ConsumeChakra(float Amount, bool bAllowPartial = false);

    /**
     * Restores chakra. Returns the actual amount restored (clamped to max).
     */
    UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Chakra")
    float RestoreChakra(float Amount);

    /**
     * Sets chakra to an exact value. Used by transformation systems and
     * scripted events. Clamps to [0, MaxChakra].
     */
    UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Chakra")
    void SetChakra(float NewValue);

    // ------------------------------------------------------------------
    //  Sharing & Transfer
    // ------------------------------------------------------------------

    /**
     * Transfers chakra to another user (e.g., Naruto sharing with allies).
     * @param Recipient  The target user.
     * @param Amount     Amount to transfer.
     * @return Actual amount transferred.
     */
    UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Chakra|Sharing")
    float TransferChakraTo(TScriptInterface<IChakraUser> Recipient, float Amount);

    /**
     * Returns true if this user can currently share chakra with others.
     * (e.g., false during certain transformations or sealing states)
     */
    UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Chakra|Sharing")
    bool CanShareChakra() const;

    // ------------------------------------------------------------------
    //  Events
    // ------------------------------------------------------------------

    /** Called when chakra drops to zero. */
    UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Chakra|Events")
    void OnChakraDepleted();

    /** Called when chakra is fully restored. */
    UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Chakra|Events")
    void OnChakraFullyRestored();

    /**
     * Called each tick by the ChakraSystem to apply passive regeneration.
     * @param DeltaTime Tick delta.
     * @param RegenAmount Amount to restore this tick.
     */
    UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Chakra|Events")
    void OnChakraRegenTick(float DeltaTime, float RegenAmount);
};
