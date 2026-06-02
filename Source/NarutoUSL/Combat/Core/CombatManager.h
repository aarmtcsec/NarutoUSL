// Copyright (c) 2025 Maze Runner Studios. All Rights Reserved.
// Naruto: Ultimate Shinobi Legacy
// CombatManager — Central authority for all combat simulation.
// Owns HitboxManager, ComboSystem, SubstitutionSystem, ParrySystem.
// Drives the per-frame combat tick for all active combatants.

#pragma once

#include "CoreMinimal.h"
#include "Core/Subsystems/NarutoSubsystem.h"
#include "Core/Types/NarutoTypes.h"
#include "Combat/Combos/ComboGraph.h"
#include "CombatManager.generated.h"

class UChakraSystem;
class UNarutoEventBus;
class UHitboxManager;
class UComboSystem;
class USubstitutionSystem;
class UParrySystem;
class UDamageCalculator;
class ANarutoCharacterBase;

// ============================================================
//  Active Combatant Record
// ============================================================

USTRUCT()
struct NARUTOUSL_API FActiveCombatantRecord
{
    GENERATED_BODY()

    TWeakObjectPtr<ANarutoCharacterBase> Character;

    /** Frame counter for the current attack. -1 = not attacking. */
    int32 CurrentAttackFrame = -1;

    /** Whether this combatant is currently in an active attack. */
    bool bIsAttacking = false;

    /** Accumulated time since last frame advance (for sub-60fps correction). */
    float FrameAccumulator = 0.0f;

    bool IsValid() const { return Character.IsValid(); }
};

// ============================================================
//  CombatManager
// ============================================================

/**
 * UCombatManager
 *
 * The single authoritative system for all combat in the game.
 * Responsibilities:
 *
 *   Per-frame:
 *     1. Advance attack frames for all active combatants
 *     2. Evaluate hitboxes via HitboxManager
 *     3. For each hit: check substitution → check parry → resolve damage
 *     4. Tick combo timers
 *     5. Tick substitution cooldowns
 *     6. Tick guard recovery
 *     7. Tick hitstun counters
 *
 *   On demand:
 *     - RegisterCombatant / UnregisterCombatant
 *     - BeginAttack (from player input or AI)
 *     - RequestSubstitution (from player input)
 *     - RequestParry (from player input)
 *
 * Thread safety:
 *   All combat logic runs on the game thread.
 *   Hitbox evaluation is single-threaded but designed for
 *   future parallelization via task graph.
 */
UCLASS(NotBlueprintable)
class NARUTOUSL_API UCombatManager : public UNarutoSubsystem
{
    GENERATED_BODY()

public:

    UCombatManager();

    // ------------------------------------------------------------------
    //  UNarutoSubsystem Interface
    // ------------------------------------------------------------------

    void Initialize(UChakraSystem* InChakraSystem, UNarutoEventBus* InEventBus);
    virtual void Tick(float DeltaTime) override;
    virtual void Shutdown() override;
    virtual TMap<FString, FString> GetDebugInfo() const override;

    // ------------------------------------------------------------------
    //  Combatant Registration
    // ------------------------------------------------------------------

    /**
     * Registers a character as an active combatant.
     * Registers their hurtboxes with HitboxManager.
     * Registers their combo graph with ComboSystem.
     */
    UFUNCTION(BlueprintCallable, Category = "Combat")
    void RegisterCombatant(ANarutoCharacterBase* Character);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void UnregisterCombatant(ANarutoCharacterBase* Character);

    UFUNCTION(BlueprintPure, Category = "Combat")
    bool IsCombatantRegistered(ANarutoCharacterBase* Character) const;

    // ------------------------------------------------------------------
    //  Attack Execution
    // ------------------------------------------------------------------

    /**
     * Begins an attack for the specified character.
     * Registers hitboxes, sets frame data on CombatComponent,
     * and adds the character to the active attack list.
     *
     * @param Character  The attacking character.
     * @param Node       The combo node defining this attack.
     */
    void BeginAttack(ANarutoCharacterBase* Character, const FComboNode& Node);

    /**
     * Immediately cancels the current attack for the specified character.
     * Called on interruption (hitstun, substitution, etc.)
     */
    void CancelAttack(ANarutoCharacterBase* Character);

    // ------------------------------------------------------------------
    //  Input Processing
    // ------------------------------------------------------------------

    /**
     * Processes an attack input from a character.
     * Routes to ComboSystem for graph traversal.
     * If a valid node is returned, calls BeginAttack().
     */
    void ProcessAttackInput(ANarutoCharacterBase* Character, EComboInputType InputType);

    /**
     * Processes a block/parry input press.
     */
    void ProcessBlockInputPressed(ANarutoCharacterBase* Character);

    /**
     * Processes a block/parry input release.
     */
    void ProcessBlockInputReleased(ANarutoCharacterBase* Character);

    /**
     * Processes a substitution input.
     * Only valid when a hit is incoming (checked against pending hits).
     */
    void ProcessSubstitutionInput(ANarutoCharacterBase* Character);

    // ------------------------------------------------------------------
    //  Damage Pipeline
    // ------------------------------------------------------------------

    /**
     * Resolves a damage event through the full pipeline:
     * substitution check → parry check → damage calculation → application.
     * Called internally after hitbox evaluation.
     */
    void ResolveDamageEvent(ANarutoCharacterBase* Attacker,
                            ANarutoCharacterBase* Defender,
                            const FHitboxData& HitboxData,
                            const FComboNode* SourceNode);

    // ------------------------------------------------------------------
    //  Configuration
    // ------------------------------------------------------------------

    void SetDifficultyScale(float Scale);
    void SetCombatPaused(bool bPaused);

    UFUNCTION(BlueprintPure, Category = "Combat")
    float GetDifficultyScale() const { return DifficultyScale; }

    UFUNCTION(BlueprintPure, Category = "Combat")
    bool IsCombatPaused() const { return bCombatPaused; }

    // ------------------------------------------------------------------
    //  Subsystem Accessors
    // ------------------------------------------------------------------

    UHitboxManager*      GetHitboxManager()      const { return HitboxManager; }
    UComboSystem*        GetComboSystem()         const { return ComboSystem; }
    USubstitutionSystem* GetSubstitutionSystem()  const { return SubstitutionSystem; }
    UParrySystem*        GetParrySystem()         const { return ParrySystem; }

private:

    // ------------------------------------------------------------------
    //  Per-Frame Tick Stages
    // ------------------------------------------------------------------

    void TickAttackFrames(float DeltaTime);
    void TickHitboxEvaluation();
    void TickComboTimers(float DeltaTime);
    void TickSubstitutionCooldowns(float DeltaTime);
    void TickGuardRecovery(float DeltaTime);
    void TickHitstun();
    void TickParryWindows(float DeltaTime);

    // ------------------------------------------------------------------
    //  Internal Helpers
    // ------------------------------------------------------------------

    FActiveCombatantRecord* FindRecord(ANarutoCharacterBase* Character);
    void OnComboNodeActivated(ANarutoCharacterBase* Character,
                              const FComboNode& Node, int32 ComboIndex);
    void PublishCombatEncounterEvent(bool bStarted);

    // ------------------------------------------------------------------
    //  State
    // ------------------------------------------------------------------

    UPROPERTY()
    TObjectPtr<UHitboxManager> HitboxManager;

    UPROPERTY()
    TObjectPtr<UComboSystem> ComboSystem;

    UPROPERTY()
    TObjectPtr<USubstitutionSystem> SubstitutionSystem;

    UPROPERTY()
    TObjectPtr<UParrySystem> ParrySystem;

    TWeakObjectPtr<UChakraSystem>   ChakraSystem;
    TWeakObjectPtr<UNarutoEventBus> EventBus;

    TArray<FActiveCombatantRecord> ActiveCombatants;

    float DifficultyScale = 1.0f;
    bool  bCombatPaused   = false;

    int32 TotalHitsThisSession    = 0;
    int32 TotalDamageThisSession  = 0;
    int32 TotalSubstitutions      = 0;
    int32 TotalParries            = 0;
};
