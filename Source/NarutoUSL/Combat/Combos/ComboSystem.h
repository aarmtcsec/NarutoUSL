// Copyright (c) 2025 Maze Runner Studios. All Rights Reserved.
// Naruto: Ultimate Shinobi Legacy
// ComboSystem — Manages combo graph traversal and input buffering.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Combat/Combos/ComboGraph.h"
#include "ComboSystem.generated.h"

class ANarutoCharacterBase;
class UCombatComponent;

DECLARE_MULTICAST_DELEGATE_ThreeParams(FOnComboNodeActivated,
    ANarutoCharacterBase* /*Character*/,
    const FComboNode& /*Node*/,
    int32 /*ComboIndex*/);

DECLARE_MULTICAST_DELEGATE_OneParam(FOnComboFinished,
    ANarutoCharacterBase* /*Character*/);

/**
 * UComboSystem
 *
 * Owned by CombatManager. Manages the combo graph state for all
 * active combatants. Handles:
 *   - Input buffering during cancel windows
 *   - Graph traversal (entry → node → transition → next node)
 *   - Combo validation (airborne requirements, tag requirements)
 *   - Combo reset on timeout or interruption
 *
 * One FComboState per active combatant, stored in a TMap.
 */
UCLASS(NotBlueprintable)
class NARUTOUSL_API UComboSystem : public UObject
{
    GENERATED_BODY()

public:

    void Initialize();
    void Shutdown();

    // ------------------------------------------------------------------
    //  Registration
    // ------------------------------------------------------------------

    void RegisterCombatant(ANarutoCharacterBase* Character, UComboGraphData* GraphData);
    void UnregisterCombatant(ANarutoCharacterBase* Character);

    // ------------------------------------------------------------------
    //  Input Processing
    // ------------------------------------------------------------------

    /**
     * Processes an attack input for the specified character.
     * If in a cancel window: advances the combo graph.
     * If not in a combo: starts a new combo from the entry node.
     * Returns the activated node, or nullptr if no valid transition.
     */
    const FComboNode* ProcessInput(ANarutoCharacterBase* Character, EComboInputType Input);

    /**
     * Buffers an input for the specified character.
     * Buffered inputs are consumed at the start of the next cancel window.
     */
    void BufferInput(ANarutoCharacterBase* Character, EComboInputType Input);

    // ------------------------------------------------------------------
    //  State Control
    // ------------------------------------------------------------------

    /** Called by CombatManager when a character enters a cancel window. */
    void OnCancelWindowOpened(ANarutoCharacterBase* Character);

    /** Called by CombatManager when a character exits a cancel window. */
    void OnCancelWindowClosed(ANarutoCharacterBase* Character);

    /** Called by CombatManager when a character's attack is interrupted. */
    void OnAttackInterrupted(ANarutoCharacterBase* Character);

    /** Called by CombatManager when a combo times out. */
    void OnComboTimeout(ANarutoCharacterBase* Character);

    // ------------------------------------------------------------------
    //  Queries
    // ------------------------------------------------------------------

    UFUNCTION()
    bool IsInCombo(ANarutoCharacterBase* Character) const;

    const FComboNode* GetCurrentNode(ANarutoCharacterBase* Character) const;

    bool HasBufferedInput(ANarutoCharacterBase* Character) const;

    // ------------------------------------------------------------------
    //  Events
    // ------------------------------------------------------------------

    FOnComboNodeActivated OnComboNodeActivated;
    FOnComboFinished      OnComboFinished;

private:

    const FComboNode* ActivateNode(ANarutoCharacterBase* Character,
                                   FComboState& State,
                                   const FComboNode* Node);

    bool ValidateNodeRequirements(ANarutoCharacterBase* Character,
                                  const FComboNode& Node) const;

    TMap<TWeakObjectPtr<ANarutoCharacterBase>, FComboState> ComboStates;
    int32 GlobalComboIndex = 0;
};
