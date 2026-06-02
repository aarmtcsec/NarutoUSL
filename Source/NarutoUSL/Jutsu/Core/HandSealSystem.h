// Copyright (c) 2025 Maze Runner Studios. All Rights Reserved.
// Naruto: Ultimate Shinobi Legacy
// HandSealSystem — Validates and tracks hand seal sequences during jutsu casting.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Jutsu/Data/JutsuData.h"
#include "HandSealSystem.generated.h"

class ANarutoCharacterBase;

// ============================================================
//  Hand Seal State
// ============================================================

USTRUCT()
struct NARUTOUSL_API FHandSealState
{
    GENERATED_BODY()

    TWeakObjectPtr<ANarutoCharacterBase> Caster;

    /** The full required sequence for the current jutsu. */
    TArray<EHandSeal> RequiredSequence;

    /** Index of the next seal to perform. */
    int32 CurrentSealIndex = 0;

    /** Time remaining to complete the full sequence. */
    float TimeRemaining = 0.0f;

    /** Total time allowed for the sequence. */
    float TotalDuration = 0.0f;

    /** Whether the sequence is currently active. */
    bool bActive = false;

    /** Whether the sequence was completed successfully. */
    bool bCompleted = false;

    float GetProgress() const
    {
        return TotalDuration > 0.0f
            ? FMath::Clamp(1.0f - (TimeRemaining / TotalDuration), 0.0f, 1.0f)
            : 0.0f;
    }

    int32 GetSealsRemaining() const
    {
        return FMath::Max(0, RequiredSequence.Num() - CurrentSealIndex);
    }

    void Reset()
    {
        RequiredSequence.Empty();
        CurrentSealIndex = 0;
        TimeRemaining    = 0.0f;
        TotalDuration    = 0.0f;
        bActive          = false;
        bCompleted       = false;
    }
};

// ============================================================
//  Delegates
// ============================================================

DECLARE_MULTICAST_DELEGATE_ThreeParams(FOnHandSealPerformed,
    ANarutoCharacterBase* /*Caster*/,
    EHandSeal /*Seal*/,
    int32 /*SealIndex*/);

DECLARE_MULTICAST_DELEGATE_TwoParams(FOnHandSealSequenceCompleted,
    ANarutoCharacterBase* /*Caster*/,
    bool /*bSuccess*/);

DECLARE_MULTICAST_DELEGATE_TwoParams(FOnHandSealSequenceInterrupted,
    ANarutoCharacterBase* /*Caster*/,
    FGameplayTag /*JutsuTag*/);

// ============================================================
//  HandSealSystem
// ============================================================

/**
 * UHandSealSystem
 *
 * Owned by JutsuManager. Manages hand seal sequences for all
 * active casters. Validates seal input order and timing.
 *
 * For player characters:
 *   - Seals are performed automatically during the HandSeal phase
 *   - The system drives the animation via montage sections
 *   - Player can be interrupted during seals (if bInterruptible)
 *
 * For AI characters:
 *   - Seals are performed at the configured HandSealDuration rate
 *   - No input validation — AI always performs seals correctly
 *
 * Mastery effect on seals:
 *   - Higher mastery reduces HandSealDuration
 *   - At mastery 10, some jutsu skip seals entirely
 */
UCLASS(NotBlueprintable)
class NARUTOUSL_API UHandSealSystem : public UObject
{
    GENERATED_BODY()

public:

    void Initialize();
    void Shutdown();

    // ------------------------------------------------------------------
    //  Sequence Management
    // ------------------------------------------------------------------

    /**
     * Begins a hand seal sequence for the specified caster.
     * @param Caster        The character performing the seals.
     * @param JutsuData     The jutsu being cast.
     * @param MasteryLevel  Caster's mastery level for this jutsu (affects duration).
     * @return false if the caster already has an active sequence.
     */
    bool BeginSequence(ANarutoCharacterBase* Caster,
                       const UJutsuData* JutsuData,
                       int32 MasteryLevel);

    /**
     * Interrupts the active sequence for the specified caster.
     * Called when the caster takes damage (if bInterruptible).
     */
    void InterruptSequence(ANarutoCharacterBase* Caster, FGameplayTag JutsuTag);

    /**
     * Cancels the active sequence without broadcasting interruption.
     */
    void CancelSequence(ANarutoCharacterBase* Caster);

    // ------------------------------------------------------------------
    //  Tick
    // ------------------------------------------------------------------

    /**
     * Advances all active sequences. Called by JutsuManager each tick.
     * Broadcasts OnHandSealSequenceCompleted when a sequence finishes.
     */
    void Tick(float DeltaTime);

    // ------------------------------------------------------------------
    //  Queries
    // ------------------------------------------------------------------

    bool HasActiveSequence(ANarutoCharacterBase* Caster) const;
    float GetSequenceProgress(ANarutoCharacterBase* Caster) const;
    int32 GetCurrentSealIndex(ANarutoCharacterBase* Caster) const;

    // ------------------------------------------------------------------
    //  Events
    // ------------------------------------------------------------------

    FOnHandSealPerformed            OnHandSealPerformed;
    FOnHandSealSequenceCompleted    OnHandSealSequenceCompleted;
    FOnHandSealSequenceInterrupted  OnHandSealSequenceInterrupted;

private:

    float CalculateSequenceDuration(const UJutsuData* JutsuData, int32 MasteryLevel) const;

    TMap<TWeakObjectPtr<ANarutoCharacterBase>, FHandSealState> ActiveSequences;

    /** Mastery level at which seal duration is halved. */
    static constexpr int32 MasteryHalfDurationLevel = 5;

    /** Mastery level at which seals are skipped entirely. */
    static constexpr int32 MasterySkipSealsLevel = 10;
};
