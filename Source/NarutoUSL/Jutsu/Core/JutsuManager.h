// Copyright (c) 2025 Maze Runner Studios. All Rights Reserved.
// Naruto: Ultimate Shinobi Legacy
// JutsuManager — Central authority for all jutsu execution.

#pragma once

#include "CoreMinimal.h"
#include "Core/Subsystems/NarutoSubsystem.h"
#include "Jutsu/Core/JutsuExecutor.h"
#include "Jutsu/Data/JutsuData.h"
#include "JutsuManager.generated.h"

class UChakraSystem;
class UCombatManager;
class UNarutoEventBus;
class UHandSealSystem;
class ANarutoCharacterBase;

// ============================================================
//  Active Cast Record
// ============================================================

USTRUCT()
struct NARUTOUSL_API FActiveCastRecord
{
    GENERATED_BODY()

    TWeakObjectPtr<ANarutoCharacterBase> Caster;
    TWeakObjectPtr<UJutsuData>           JutsuData;
    TObjectPtr<UJutsuExecutorBase>       Executor;
    FJutsuExecutionContext               Context;
    EJutsuExecutionPhase                 Phase       = EJutsuExecutionPhase::Idle;
    float                                PhaseTimer  = 0.0f;
    float                                ChargeTime  = 0.0f;
    bool                                 bSustaining = false;
    uint32                               CastID      = 0;

    bool IsValid() const { return Caster.IsValid() && JutsuData.IsValid(); }
};

// ============================================================
//  JutsuManager
// ============================================================

/**
 * UJutsuManager
 *
 * Owns HandSealSystem. Manages the full lifecycle of every jutsu cast:
 *
 *   RequestCast()
 *     → Validate (knows jutsu, can afford, not on cooldown, not in hitstun)
 *     → Consume chakra
 *     → Begin hand seals (HandSealSystem)
 *     → On seals complete: Activate executor
 *     → Execute()
 *     → Start cooldown
 *     → Tick sustained jutsu
 *     → Complete / Interrupt
 *
 * One FActiveCastRecord per caster. Characters can only cast one
 * jutsu at a time (simultaneous casting is handled by team jutsu).
 */
UCLASS(NotBlueprintable)
class NARUTOUSL_API UJutsuManager : public UNarutoSubsystem
{
    GENERATED_BODY()

public:

    UJutsuManager();

    void Initialize(UChakraSystem* InChakraSystem,
                    UCombatManager* InCombatManager,
                    UNarutoEventBus* InEventBus);

    virtual void Tick(float DeltaTime) override;
    virtual void Shutdown() override;
    virtual TMap<FString, FString> GetDebugInfo() const override;

    // ------------------------------------------------------------------
    //  Cast API
    // ------------------------------------------------------------------

    /**
     * Requests a jutsu cast for the specified character.
     * Validates all conditions, consumes chakra, begins hand seals.
     * Returns the cast ID (0 = failed).
     */
    UFUNCTION(BlueprintCallable, Category = "Jutsu")
    uint32 RequestCast(ANarutoCharacterBase* Caster,
                       FGameplayTag JutsuTag,
                       AActor* PrimaryTarget = nullptr,
                       FVector AimLocation   = FVector::ZeroVector);

    /**
     * Releases a charged jutsu at the current charge level.
     * Only valid for chargeable jutsu in the Charging phase.
     */
    UFUNCTION(BlueprintCallable, Category = "Jutsu")
    void ReleaseCharge(ANarutoCharacterBase* Caster);

    /**
     * Interrupts the active cast for the specified character.
     * Called by CombatManager when the caster takes damage.
     */
    UFUNCTION(BlueprintCallable, Category = "Jutsu")
    void InterruptCast(ANarutoCharacterBase* Caster);

    /**
     * Cancels the active cast without triggering interruption callbacks.
     */
    UFUNCTION(BlueprintCallable, Category = "Jutsu")
    void CancelCast(ANarutoCharacterBase* Caster);

    // ------------------------------------------------------------------
    //  Queries
    // ------------------------------------------------------------------

    UFUNCTION(BlueprintPure, Category = "Jutsu")
    bool IsCasting(ANarutoCharacterBase* Caster) const;

    UFUNCTION(BlueprintPure, Category = "Jutsu")
    EJutsuExecutionPhase GetCastPhase(ANarutoCharacterBase* Caster) const;

    UFUNCTION(BlueprintPure, Category = "Jutsu")
    float GetChargeProgress(ANarutoCharacterBase* Caster) const;

    UHandSealSystem* GetHandSealSystem() const { return HandSealSystem; }

private:

    // ------------------------------------------------------------------
    //  Lifecycle Helpers
    // ------------------------------------------------------------------

    bool ValidateCastRequest(ANarutoCharacterBase* Caster,
                              UJutsuData* Data,
                              FString& OutFailReason) const;

    void BuildExecutionContext(FActiveCastRecord& Record,
                               AActor* PrimaryTarget,
                               FVector AimLocation);

    void ActivateCast(FActiveCastRecord& Record);
    void ExecuteInstantCast(FActiveCastRecord& Record);
    void CompleteCast(FActiveCastRecord& Record);
    void StartCooldown(FActiveCastRecord& Record);

    void TickActiveCasts(float DeltaTime);
    void TickChakraSystem(float DeltaTime);

    void OnHandSealCompleted(ANarutoCharacterBase* Caster, bool bSuccess);

    FActiveCastRecord* FindRecord(ANarutoCharacterBase* Caster);

    // ------------------------------------------------------------------
    //  State
    // ------------------------------------------------------------------

    UPROPERTY()
    TObjectPtr<UHandSealSystem> HandSealSystem;

    TWeakObjectPtr<UChakraSystem>   ChakraSystem;
    TWeakObjectPtr<UCombatManager>  CombatManager;
    TWeakObjectPtr<UNarutoEventBus> EventBus;

    TArray<FActiveCastRecord> ActiveCasts;

    uint32 NextCastID = 1;

    int32 TotalCastsThisSession      = 0;
    int32 TotalInterruptionsThisSession = 0;
};
