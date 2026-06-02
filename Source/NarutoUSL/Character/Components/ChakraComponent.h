// Copyright (c) 2025 Maze Runner Studios. All Rights Reserved.
// Naruto: Ultimate Shinobi Legacy
// ChakraComponent — Manages chakra pool, regen, nature affinities, and sharing.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Core/Types/NarutoTypes.h"
#include "Core/Interfaces/IChakraUser.h"
#include "ChakraComponent.generated.h"

// ============================================================
//  Delegates
// ============================================================

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnChakraChanged,   float, NewChakra,  float, MaxChakra);
DECLARE_DYNAMIC_MULTICAST_DELEGATE          (FOnChakraDepleted_Comp);
DECLARE_DYNAMIC_MULTICAST_DELEGATE          (FOnChakraFull);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnChakraShared,    AActor*, Recipient, float, Amount);

// ============================================================
//  Regen Mode
// ============================================================

UENUM(BlueprintType)
enum class EChakraRegenMode : uint8
{
    Passive     UMETA(DisplayName = "Passive"),      // Normal out-of-combat regen
    Meditation  UMETA(DisplayName = "Meditation"),   // Boosted regen while meditating
    Suppressed  UMETA(DisplayName = "Suppressed"),   // No regen (sealed, transformed)
    Drain       UMETA(DisplayName = "Drain"),        // Negative regen (transformation cost)
};

// ============================================================
//  ChakraComponent
// ============================================================

/**
 * UChakraComponent
 *
 * Manages the chakra resource for any character. Handles:
 *   - Passive and meditation regen
 *   - Consumption validation and deduction
 *   - Nature affinity queries
 *   - Chakra sharing (Naruto-style team transfer)
 *   - Transformation drain
 *   - Sage chakra accumulation
 *
 * The ChakraSystem subsystem ticks all registered ChakraComponents
 * each frame. Components do not self-tick — they are driven externally
 * to allow the subsystem to batch and prioritize updates.
 */
UCLASS(ClassGroup = "NarutoUSL|Character", meta = (BlueprintSpawnableComponent))
class NARUTOUSL_API UChakraComponent : public UActorComponent
{
    GENERATED_BODY()

public:

    UChakraComponent();

    virtual void BeginPlay() override;

    // ------------------------------------------------------------------
    //  Initialization
    // ------------------------------------------------------------------

    void InitializeChakra(float InMaxChakra, EChakraNature InNatureAffinities,
                          float InRegenRate, bool bFillToMax = true);

    // ------------------------------------------------------------------
    //  Queries
    // ------------------------------------------------------------------

    UFUNCTION(BlueprintPure, Category = "Chakra")
    float GetCurrentChakra() const { return CurrentChakra; }

    UFUNCTION(BlueprintPure, Category = "Chakra")
    float GetMaxChakra() const { return MaxChakra; }

    UFUNCTION(BlueprintPure, Category = "Chakra")
    float GetChakraPercent() const;

    UFUNCTION(BlueprintPure, Category = "Chakra")
    float GetRegenRate() const { return RegenRate; }

    UFUNCTION(BlueprintPure, Category = "Chakra")
    EChakraNature GetNatureAffinities() const { return NatureAffinities; }

    UFUNCTION(BlueprintPure, Category = "Chakra")
    bool HasNatureAffinity(EChakraNature Nature) const;

    UFUNCTION(BlueprintPure, Category = "Chakra")
    EChakraRegenMode GetRegenMode() const { return RegenMode; }

    UFUNCTION(BlueprintPure, Category = "Chakra")
    bool IsDepleted() const { return CurrentChakra <= 0.0f; }

    UFUNCTION(BlueprintPure, Category = "Chakra")
    bool IsAtMaxChakra() const;

    /**
     * Returns true if this component can afford the specified cost.
     * Accounts for nature affinity discounts.
     */
    UFUNCTION(BlueprintPure, Category = "Chakra")
    bool CanAffordCost(const FChakraCost& Cost) const;

    /**
     * Returns the effective chakra cost after nature affinity discounts.
     */
    UFUNCTION(BlueprintPure, Category = "Chakra")
    float ResolveEffectiveCost(const FChakraCost& Cost) const;

    // ------------------------------------------------------------------
    //  Consumption
    // ------------------------------------------------------------------

    /**
     * Consumes chakra. Returns actual amount consumed.
     * Returns 0 if insufficient and bAllowPartial is false.
     */
    UFUNCTION(BlueprintCallable, Category = "Chakra")
    float ConsumeChakra(float Amount, bool bAllowPartial = false);

    /**
     * Consumes chakra using a FChakraCost definition.
     * Applies nature affinity discounts automatically.
     * Returns false if the cost cannot be paid.
     */
    UFUNCTION(BlueprintCallable, Category = "Chakra")
    bool ConsumeCost(const FChakraCost& Cost);

    // ------------------------------------------------------------------
    //  Restoration
    // ------------------------------------------------------------------

    UFUNCTION(BlueprintCallable, Category = "Chakra")
    float RestoreChakra(float Amount);

    UFUNCTION(BlueprintCallable, Category = "Chakra")
    void SetChakra(float NewValue);

    UFUNCTION(BlueprintCallable, Category = "Chakra")
    void FillChakra();

    // ------------------------------------------------------------------
    //  Regen Control
    // ------------------------------------------------------------------

    UFUNCTION(BlueprintCallable, Category = "Chakra|Regen")
    void SetRegenMode(EChakraRegenMode NewMode);

    UFUNCTION(BlueprintCallable, Category = "Chakra|Regen")
    void SetRegenRate(float NewRate);

    /** Sets a temporary regen multiplier (e.g., from a buff). */
    UFUNCTION(BlueprintCallable, Category = "Chakra|Regen")
    void SetRegenMultiplier(float Multiplier, FGameplayTag SourceTag, float Duration = -1.0f);

    UFUNCTION(BlueprintCallable, Category = "Chakra|Regen")
    void RemoveRegenMultiplier(FGameplayTag SourceTag);

    // ------------------------------------------------------------------
    //  Sharing
    // ------------------------------------------------------------------

    UFUNCTION(BlueprintPure, Category = "Chakra|Sharing")
    bool CanShareChakra() const;

    /**
     * Transfers chakra to another component.
     * Capped at ChakraShareMaxPercent of this component's max chakra.
     */
    UFUNCTION(BlueprintCallable, Category = "Chakra|Sharing")
    float TransferChakraTo(UChakraComponent* Recipient, float Amount);

    // ------------------------------------------------------------------
    //  Sage Chakra
    // ------------------------------------------------------------------

    UFUNCTION(BlueprintPure, Category = "Chakra|Sage")
    float GetSageChakra() const { return SageChakra; }

    UFUNCTION(BlueprintPure, Category = "Chakra|Sage")
    float GetMaxSageChakra() const { return MaxSageChakra; }

    UFUNCTION(BlueprintPure, Category = "Chakra|Sage")
    bool HasFullSageChakra() const;

    UFUNCTION(BlueprintCallable, Category = "Chakra|Sage")
    void AccumulateSageChakra(float Amount);

    UFUNCTION(BlueprintCallable, Category = "Chakra|Sage")
    void ConsumeSageChakra(float Amount);

    UFUNCTION(BlueprintCallable, Category = "Chakra|Sage")
    void InitializeSageChakra(float InMaxSageChakra);

    // ------------------------------------------------------------------
    //  Tick (called by ChakraSystem, not self-ticked)
    // ------------------------------------------------------------------

    void TickChakra(float DeltaTime);

    // ------------------------------------------------------------------
    //  Events
    // ------------------------------------------------------------------

    UPROPERTY(BlueprintAssignable, Category = "Chakra|Events")
    FOnChakraChanged OnChakraChanged;

    UPROPERTY(BlueprintAssignable, Category = "Chakra|Events")
    FOnChakraDepleted_Comp OnChakraDepleted;

    UPROPERTY(BlueprintAssignable, Category = "Chakra|Events")
    FOnChakraFull OnChakraFull;

    UPROPERTY(BlueprintAssignable, Category = "Chakra|Events")
    FOnChakraShared OnChakraShared;

protected:

    // ------------------------------------------------------------------
    //  Internal
    // ------------------------------------------------------------------

    void ApplyRegenTick(float DeltaTime);
    float GetEffectiveRegenRate() const;
    void BroadcastChakraChanged();

    // ------------------------------------------------------------------
    //  State
    // ------------------------------------------------------------------

    UPROPERTY(BlueprintReadOnly, Category = "Chakra")
    float CurrentChakra = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Chakra", meta = (ClampMin = "1"))
    float MaxChakra = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Chakra", meta = (ClampMin = "0"))
    float RegenRate = 5.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Chakra")
    EChakraNature NatureAffinities = EChakraNature::None;

    UPROPERTY(BlueprintReadOnly, Category = "Chakra")
    EChakraRegenMode RegenMode = EChakraRegenMode::Passive;

    /** Discount applied to jutsu costs when using a matching nature affinity (0-1). */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Chakra",
        meta = (ClampMin = "0", ClampMax = "0.5"))
    float NatureAffinityDiscount = 0.1f;

    /** Whether this component can share chakra with allies. */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Chakra")
    bool bCanShareChakra = true;

    // Sage chakra
    float SageChakra    = 0.0f;
    float MaxSageChakra = 0.0f;

    // Regen multiplier stack
    TMap<FGameplayTag, float> RegenMultipliers;
    TMap<FGameplayTag, float> RegenMultiplierDurations;

    float RegenAccumulator = 0.0f;
};
