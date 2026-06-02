// Copyright (c) 2025 Maze Runner Studios. All Rights Reserved.
// Naruto: Ultimate Shinobi Legacy
// ParrySystem — Perfect guard and parry window management.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Core/Types/NarutoTypes.h"
#include "ParrySystem.generated.h"

class ANarutoCharacterBase;

UENUM(BlueprintType)
enum class EParryResult : uint8
{
    None            UMETA(DisplayName = "No Parry"),
    PerfectGuard    UMETA(DisplayName = "Perfect Guard"),
    Parry           UMETA(DisplayName = "Parry"),
    NormalBlock     UMETA(DisplayName = "Normal Block"),
};

USTRUCT()
struct NARUTOUSL_API FParryWindowState
{
    GENERATED_BODY()

    TWeakObjectPtr<ANarutoCharacterBase> Character;
    float WindowTimeRemaining   = 0.0f;
    bool  bParryWindowActive    = false;
    bool  bPerfectGuardActive   = false;
    float ParryWindowDuration   = 0.0f;
};

DECLARE_MULTICAST_DELEGATE_ThreeParams(FOnParryResult,
    ANarutoCharacterBase* /*Defender*/,
    ANarutoCharacterBase* /*Attacker*/,
    EParryResult /*Result*/);

/**
 * UParrySystem
 *
 * Owned by CombatManager. Tracks parry and perfect guard windows
 * for all active combatants. Evaluates incoming hits against
 * active windows to determine the parry result.
 *
 * Window types:
 *   Perfect Guard — input on the exact frame of impact (±PerfectGuardWindow seconds)
 *   Parry         — input within ParryWindow seconds before impact
 *   Normal Block  — holding block outside the parry window
 */
UCLASS(NotBlueprintable)
class NARUTOUSL_API UParrySystem : public UObject
{
    GENERATED_BODY()

public:

    void Initialize();
    void Shutdown();
    void Tick(float DeltaTime);

    // ------------------------------------------------------------------
    //  Window Activation
    // ------------------------------------------------------------------

    /** Called when a character presses the block/parry input. */
    void OnBlockInputPressed(ANarutoCharacterBase* Character);

    /** Called when a character releases the block/parry input. */
    void OnBlockInputReleased(ANarutoCharacterBase* Character);

    // ------------------------------------------------------------------
    //  Hit Evaluation
    // ------------------------------------------------------------------

    /**
     * Evaluates whether an incoming hit is parried, perfect-guarded,
     * blocked, or unblocked for the specified defender.
     */
    EParryResult EvaluateIncomingHit(ANarutoCharacterBase* Defender,
                                     ANarutoCharacterBase* Attacker,
                                     const FNarutoDamageEvent& DamageEvent) const;

    // ------------------------------------------------------------------
    //  Events
    // ------------------------------------------------------------------

    FOnParryResult OnParryResult;

private:

    void ApplyParryReward(ANarutoCharacterBase* Defender,
                          ANarutoCharacterBase* Attacker,
                          EParryResult Result);

    TMap<TWeakObjectPtr<ANarutoCharacterBase>, FParryWindowState> ParryWindows;
};
