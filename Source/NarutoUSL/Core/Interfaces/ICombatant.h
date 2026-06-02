// Copyright (c) 2025 Maze Runner Studios. All Rights Reserved.
// Naruto: Ultimate Shinobi Legacy
// Interface: ICombatant
// Implemented by any actor that participates in combat.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Core/Types/NarutoTypes.h"
#include "ICombatant.generated.h"

UINTERFACE(MinimalAPI, BlueprintType, meta = (CannotImplementInterfaceInBlueprint = "false"))
class UCombatant : public UInterface
{
    GENERATED_BODY()
};

/**
 * ICombatant
 *
 * Implemented by all actors that can engage in combat: player characters,
 * enemies, bosses, and summoned creatures. Provides a uniform API for the
 * CombatManager to query and drive combat state without coupling to concrete
 * character classes.
 */
class NARUTOUSL_API ICombatant
{
    GENERATED_BODY()

public:

    // ------------------------------------------------------------------
    //  Identity
    // ------------------------------------------------------------------

    /** Returns the unique runtime ID for this combatant. */
    UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Combat|Identity")
    int32 GetCombatantID() const;

    /** Returns the display name used in combat UI and logs. */
    UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Combat|Identity")
    FText GetCombatantDisplayName() const;

    // ------------------------------------------------------------------
    //  State Queries
    // ------------------------------------------------------------------

    /** Returns true if this combatant is alive and able to act. */
    UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Combat|State")
    bool IsAlive() const;

    /** Returns true if this combatant is currently in an active combat encounter. */
    UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Combat|State")
    bool IsInCombat() const;

    /** Returns the current set of active combat flags. */
    UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Combat|State")
    ECombatFlags GetCombatFlags() const;

    /** Returns true if the specified flag is currently set. */
    UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Combat|State")
    bool HasCombatFlag(ECombatFlags Flag) const;

    /** Returns the current combat stance. */
    UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Combat|State")
    ECombatStance GetCombatStance() const;

    /** Returns the current frame within the active attack animation (0 if idle). */
    UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Combat|State")
    int32 GetCurrentAttackFrame() const;

    // ------------------------------------------------------------------
    //  Stat Queries
    // ------------------------------------------------------------------

    /** Returns the resolved final value of the specified stat. */
    UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Combat|Stats")
    float GetResolvedStat(ECharacterStat Stat) const;

    /** Returns current health as a 0-1 normalized value. */
    UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Combat|Stats")
    float GetHealthPercent() const;

    /** Returns current chakra as a 0-1 normalized value. */
    UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Combat|Stats")
    float GetChakraPercent() const;

    // ------------------------------------------------------------------
    //  Combat Events (called by CombatManager)
    // ------------------------------------------------------------------

    /**
     * Called when this combatant successfully lands a hit on a target.
     * @param Target     The combatant that was hit.
     * @param DamageEvent Full damage event data.
     */
    UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Combat|Events")
    void OnHitLanded(ICombatant* Target, const FNarutoDamageEvent& DamageEvent);

    /**
     * Called when this combatant receives a hit.
     * @param Instigator The combatant that dealt the hit.
     * @param DamageEvent Full damage event data.
     */
    UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Combat|Events")
    void OnHitReceived(ICombatant* Instigator, const FNarutoDamageEvent& DamageEvent);

    /**
     * Called when this combatant successfully blocks an attack.
     * @param Instigator The attacker.
     * @param DamageEvent Damage event (with bIsBlocked = true).
     */
    UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Combat|Events")
    void OnBlockSuccessful(ICombatant* Instigator, const FNarutoDamageEvent& DamageEvent);

    /**
     * Called when this combatant successfully parries an attack.
     * Parry grants a counter window and full invulnerability for ParryInvulFrames.
     */
    UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Combat|Events")
    void OnParrySuccessful(ICombatant* Instigator, const FNarutoDamageEvent& DamageEvent);

    /**
     * Called when this combatant performs a substitution jutsu.
     * @param Instigator The attacker that triggered the substitution.
     */
    UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Combat|Events")
    void OnSubstitutionActivated(ICombatant* Instigator);

    /**
     * Called when this combatant enters combat (first enemy detected).
     */
    UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Combat|Events")
    void OnCombatEntered();

    /**
     * Called when this combatant exits combat (all threats cleared).
     */
    UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Combat|Events")
    void OnCombatExited();

    /**
     * Called when this combatant is defeated.
     * @param Killer The combatant responsible for the killing blow.
     */
    UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Combat|Events")
    void OnDefeated(ICombatant* Killer);

    // ------------------------------------------------------------------
    //  Combat Control (called by CombatManager)
    // ------------------------------------------------------------------

    /**
     * Applies a set of combat flags to this combatant.
     * Used by the CombatManager to set states like IsLaunched, IsKnockdown, etc.
     */
    UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Combat|Control")
    void ApplyCombatFlags(ECombatFlags Flags);

    /**
     * Removes a set of combat flags from this combatant.
     */
    UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Combat|Control")
    void RemoveCombatFlags(ECombatFlags Flags);

    /**
     * Forces this combatant into a specific combat stance.
     */
    UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Combat|Control")
    void SetCombatStance(ECombatStance NewStance);

    /**
     * Applies a knockback impulse to this combatant.
     * @param Direction Normalized world-space direction.
     * @param Force     Impulse magnitude.
     */
    UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Combat|Control")
    void ApplyKnockback(FVector Direction, float Force);

    /**
     * Launches this combatant into the air for aerial combo sequences.
     * @param LaunchVelocity World-space launch velocity.
     */
    UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Combat|Control")
    void ApplyLaunch(FVector LaunchVelocity);

    /**
     * Interrupts the current action and forces the combatant into hitstun.
     * @param HitstunFrames Duration of the stun in frames.
     */
    UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Combat|Control")
    void ApplyHitstun(int32 HitstunFrames);
};
