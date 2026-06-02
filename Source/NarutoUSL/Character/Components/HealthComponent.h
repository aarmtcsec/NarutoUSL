// Copyright (c) 2025 Maze Runner Studios. All Rights Reserved.
// Naruto: Ultimate Shinobi Legacy
// HealthComponent — Manages health, death state, invulnerability, and healing.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Core/Types/NarutoTypes.h"
#include "Core/Interfaces/IDamageable.h"
#include "HealthComponent.generated.h"

// ============================================================
//  Delegates
// ============================================================

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnHealthChanged,    float, NewHealth,    float, MaxHealth);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam (FOnDeath_Health,     const FNarutoDamageEvent&, KillingBlow);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam (FOnRevived,          float, RestoredHealth);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnInvulnerabilityChanged, bool, bIsNowInvulnerable, float, Duration);

// ============================================================
//  HealthComponent
// ============================================================

/**
 * UHealthComponent
 *
 * Attached to every character (player, NPC, enemy, boss, summon).
 * Owns the health value and all logic for applying damage, healing,
 * invulnerability frames, and death state. Does NOT contain combat
 * logic — that lives in CombatComponent. This component only knows
 * about health values and their transitions.
 *
 * Damage flow:
 *   DamageCalculator → ApplyDamage() → OnHealthChanged → (if 0) OnDeath
 */
UCLASS(ClassGroup = "NarutoUSL|Character", meta = (BlueprintSpawnableComponent))
class NARUTOUSL_API UHealthComponent : public UActorComponent
{
    GENERATED_BODY()

public:

    UHealthComponent();

    // ------------------------------------------------------------------
    //  UActorComponent Interface
    // ------------------------------------------------------------------

    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType,
                               FActorComponentTickFunction* ThisTickFunction) override;

    // ------------------------------------------------------------------
    //  Initialization
    // ------------------------------------------------------------------

    /**
     * Sets the maximum health and optionally fills current health to max.
     * Called by the character after stats are resolved.
     */
    void InitializeHealth(float InMaxHealth, bool bFillToMax = true);

    // ------------------------------------------------------------------
    //  Health Queries
    // ------------------------------------------------------------------

    UFUNCTION(BlueprintPure, Category = "Health")
    float GetCurrentHealth() const { return CurrentHealth; }

    UFUNCTION(BlueprintPure, Category = "Health")
    float GetMaxHealth() const { return MaxHealth; }

    UFUNCTION(BlueprintPure, Category = "Health")
    float GetHealthPercent() const;

    UFUNCTION(BlueprintPure, Category = "Health")
    bool IsDead() const { return bIsDead; }

    UFUNCTION(BlueprintPure, Category = "Health")
    bool IsInvulnerable() const { return bIsInvulnerable; }

    UFUNCTION(BlueprintPure, Category = "Health")
    bool IsAtFullHealth() const;

    // ------------------------------------------------------------------
    //  Damage Application
    // ------------------------------------------------------------------

    /**
     * Primary damage entry point. Called by DamageCalculator after
     * all mitigation has been resolved. Returns actual damage applied.
     * Returns 0 if dead or invulnerable.
     */
    UFUNCTION(BlueprintCallable, Category = "Health")
    float ApplyDamage(const FNarutoDamageEvent& DamageEvent);

    /**
     * Applies a healing effect. Returns actual amount healed.
     * Clamped to MaxHealth. Does nothing if dead.
     */
    UFUNCTION(BlueprintCallable, Category = "Health")
    float ApplyHealing(float Amount, FGameplayTag HealerTag);

    /**
     * Applies a damage-over-time tick. Called by StatusEffectManager.
     * Bypasses invulnerability frames (DoT is not a hit).
     */
    UFUNCTION(BlueprintCallable, Category = "Health")
    void ApplyDamageOverTimeTick(float DamagePerTick, EDamageType DamageType, FGameplayTag SourceTag);

    // ------------------------------------------------------------------
    //  Invulnerability
    // ------------------------------------------------------------------

    /**
     * Grants invulnerability for the specified duration.
     * Stacks by taking the longest remaining duration.
     * @param Duration  Seconds. Pass -1 for permanent (until manually cleared).
     * @param SourceTag Tag identifying the source (substitution, awakening, etc.)
     */
    UFUNCTION(BlueprintCallable, Category = "Health|Invulnerability")
    void GrantInvulnerability(float Duration, FGameplayTag SourceTag);

    /** Removes invulnerability granted by the specified source tag. */
    UFUNCTION(BlueprintCallable, Category = "Health|Invulnerability")
    void RemoveInvulnerability(FGameplayTag SourceTag);

    /** Clears all invulnerability regardless of source. */
    UFUNCTION(BlueprintCallable, Category = "Health|Invulnerability")
    void ClearAllInvulnerability();

    // ------------------------------------------------------------------
    //  Death / Revival
    // ------------------------------------------------------------------

    /**
     * Revives this character with the specified health amount.
     * Resets death state and broadcasts OnRevived.
     */
    UFUNCTION(BlueprintCallable, Category = "Health|Death")
    void Revive(float ReviveHealth);

    /**
     * Forces immediate death regardless of current health.
     * Used by scripted events and instant-kill mechanics.
     */
    UFUNCTION(BlueprintCallable, Category = "Health|Death")
    void ForceKill(AActor* Killer);

    // ------------------------------------------------------------------
    //  Max Health Modification
    // ------------------------------------------------------------------

    /**
     * Updates the maximum health value (e.g., after level-up or buff).
     * Optionally scales current health proportionally.
     */
    UFUNCTION(BlueprintCallable, Category = "Health")
    void SetMaxHealth(float NewMaxHealth, bool bScaleCurrentHealth = true);

    // ------------------------------------------------------------------
    //  Regen
    // ------------------------------------------------------------------

    /** Sets the passive health regen rate (HP/sec). 0 = no regen. */
    UFUNCTION(BlueprintCallable, Category = "Health|Regen")
    void SetRegenRate(float HPPerSecond);

    UFUNCTION(BlueprintPure, Category = "Health|Regen")
    float GetRegenRate() const { return RegenRate; }

    // ------------------------------------------------------------------
    //  Events
    // ------------------------------------------------------------------

    UPROPERTY(BlueprintAssignable, Category = "Health|Events")
    FOnHealthChanged OnHealthChanged;

    UPROPERTY(BlueprintAssignable, Category = "Health|Events")
    FOnDeath_Health OnDeath;

    UPROPERTY(BlueprintAssignable, Category = "Health|Events")
    FOnRevived OnRevived;

    UPROPERTY(BlueprintAssignable, Category = "Health|Events")
    FOnInvulnerabilityChanged OnInvulnerabilityChanged;

protected:

    // ------------------------------------------------------------------
    //  Internal
    // ------------------------------------------------------------------

    void TriggerDeath(const FNarutoDamageEvent& KillingBlow);
    void TickRegen(float DeltaTime);
    void TickInvulnerability(float DeltaTime);

    // ------------------------------------------------------------------
    //  State
    // ------------------------------------------------------------------

    UPROPERTY(BlueprintReadOnly, Category = "Health")
    float CurrentHealth = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Health", meta = (ClampMin = "1"))
    float MaxHealth = 1000.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Health")
    bool bIsDead = false;

    UPROPERTY(BlueprintReadOnly, Category = "Health")
    bool bIsInvulnerable = false;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Health", meta = (ClampMin = "0"))
    float RegenRate = 0.0f;

    float RegenAccumulator = 0.0f;

    /** Active invulnerability sources. Key = SourceTag, Value = remaining duration (-1 = permanent). */
    TMap<FGameplayTag, float> InvulnerabilitySources;
};
