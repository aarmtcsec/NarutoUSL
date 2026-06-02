// Copyright (c) 2025 Maze Runner Studios. All Rights Reserved.
// Naruto: Ultimate Shinobi Legacy
// Interface: IDamageable

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Core/Types/NarutoTypes.h"
#include "IDamageable.generated.h"

UINTERFACE(MinimalAPI, BlueprintType)
class UDamageable : public UInterface
{
    GENERATED_BODY()
};

/**
 * IDamageable
 *
 * Implemented by any actor that can receive damage. Separates health/damage
 * reception from combat logic, allowing environmental hazards, traps, and
 * destructible objects to participate in the damage pipeline without
 * implementing the full ICombatant interface.
 */
class NARUTOUSL_API IDamageable
{
    GENERATED_BODY()

public:

    // ------------------------------------------------------------------
    //  Health Queries
    // ------------------------------------------------------------------

    UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Health")
    float GetCurrentHealth() const;

    UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Health")
    float GetMaxHealth() const;

    UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Health")
    float GetHealthPercent() const;

    UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Health")
    bool IsDead() const;

    UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Health")
    bool IsInvulnerable() const;

    // ------------------------------------------------------------------
    //  Damage Reception
    // ------------------------------------------------------------------

    /**
     * Primary damage entry point. Called by the DamageCalculator after
     * all mitigation has been resolved.
     *
     * @param DamageEvent  Fully resolved damage event.
     * @return             Actual damage applied after any remaining mitigation.
     */
    UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Health|Damage")
    float ApplyDamage(const FNarutoDamageEvent& DamageEvent);

    /**
     * Applies a healing effect. Returns actual amount healed.
     * @param Amount       Raw heal amount before any modifiers.
     * @param HealerTag    Gameplay tag identifying the heal source.
     */
    UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Health|Healing")
    float ApplyHealing(float Amount, FGameplayTag HealerTag);

    /**
     * Applies a damage-over-time tick. Called by StatusEffectManager each tick.
     * @param DamagePerTick  Damage for this tick.
     * @param DamageType     Type of the DoT.
     * @param SourceTag      Tag identifying the DoT source.
     */
    UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Health|Damage")
    void ApplyDamageOverTimeTick(float DamagePerTick, EDamageType DamageType, FGameplayTag SourceTag);

    // ------------------------------------------------------------------
    //  Resistance Queries
    // ------------------------------------------------------------------

    /**
     * Returns the damage resistance multiplier for the given damage type.
     * 1.0 = no resistance, 0.5 = 50% reduction, 0.0 = immune, 1.5 = weakness.
     */
    UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Health|Resistance")
    float GetDamageResistance(EDamageType DamageType) const;

    // ------------------------------------------------------------------
    //  Events
    // ------------------------------------------------------------------

    UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Health|Events")
    void OnDamageReceived(const FNarutoDamageEvent& DamageEvent);

    UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Health|Events")
    void OnHealingReceived(float Amount, FGameplayTag HealerTag);

    UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Health|Events")
    void OnDeath(const FNarutoDamageEvent& KillingBlow);
};
