// Copyright (c) 2025 Maze Runner Studios. All Rights Reserved.
// Naruto: Ultimate Shinobi Legacy
// Interface: IJutsuCaster

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Core/Types/NarutoTypes.h"
#include "IJutsuCaster.generated.h"

class UJutsuData;

UINTERFACE(MinimalAPI, BlueprintType)
class UJutsuCaster : public UInterface
{
    GENERATED_BODY()
};

/**
 * IJutsuCaster
 *
 * Implemented by any actor capable of executing jutsu. Provides the
 * JutsuManager with a uniform API to query known jutsu, validate cast
 * conditions, and receive execution callbacks.
 */
class NARUTOUSL_API IJutsuCaster
{
    GENERATED_BODY()

public:

    // ------------------------------------------------------------------
    //  Jutsu Registry
    // ------------------------------------------------------------------

    /** Returns all jutsu data assets known to this caster. */
    UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Jutsu")
    TArray<UJutsuData*> GetKnownJutsu() const;

    /** Returns true if this caster knows the jutsu identified by Tag. */
    UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Jutsu")
    bool KnowsJutsu(FGameplayTag JutsuTag) const;

    /** Returns the mastery level (0-100) for the specified jutsu. */
    UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Jutsu")
    float GetJutsuMastery(FGameplayTag JutsuTag) const;

    // ------------------------------------------------------------------
    //  Cast Validation
    // ------------------------------------------------------------------

    /**
     * Returns true if all conditions are met to cast the specified jutsu:
     * - Jutsu is known
     * - Sufficient chakra
     * - Not on cooldown
     * - Not in a state that prevents casting
     */
    UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Jutsu")
    bool CanCastJutsu(FGameplayTag JutsuTag) const;

    /** Returns the remaining cooldown in seconds for the specified jutsu. */
    UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Jutsu")
    float GetJutsuCooldownRemaining(FGameplayTag JutsuTag) const;

    // ------------------------------------------------------------------
    //  Execution Callbacks (called by JutsuExecutor)
    // ------------------------------------------------------------------

    /**
     * Called immediately before a jutsu begins execution.
     * Allows the caster to apply pre-cast animations, consume chakra, etc.
     */
    UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Jutsu|Events")
    void OnJutsuCastBegin(FGameplayTag JutsuTag, UJutsuData* JutsuData);

    /**
     * Called when a jutsu successfully activates (after hand seals / startup).
     */
    UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Jutsu|Events")
    void OnJutsuActivated(FGameplayTag JutsuTag, UJutsuData* JutsuData);

    /**
     * Called when a jutsu completes its full execution cycle.
     */
    UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Jutsu|Events")
    void OnJutsuCompleted(FGameplayTag JutsuTag, UJutsuData* JutsuData);

    /**
     * Called when a jutsu is interrupted before activation.
     */
    UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Jutsu|Events")
    void OnJutsuInterrupted(FGameplayTag JutsuTag, UJutsuData* JutsuData);

    /**
     * Called when a jutsu enters cooldown.
     * @param CooldownDuration Total cooldown duration in seconds.
     */
    UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Jutsu|Events")
    void OnJutsuCooldownStarted(FGameplayTag JutsuTag, float CooldownDuration);

    /**
     * Called when a jutsu's cooldown expires.
     */
    UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Jutsu|Events")
    void OnJutsuCooldownExpired(FGameplayTag JutsuTag);
};
