// Copyright (c) 2025 Maze Runner Studios. All Rights Reserved.

#include "Character/Components/HealthComponent.h"
#include "NarutoUSL.h"

UHealthComponent::UHealthComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.0f;
}

void UHealthComponent::BeginPlay()
{
    Super::BeginPlay();
    // Health is initialized externally by the character after stat resolution.
    // If InitializeHealth was never called, default to MaxHealth.
    if (CurrentHealth <= 0.0f && !bIsDead)
    {
        CurrentHealth = MaxHealth;
    }
}

void UHealthComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                     FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (bIsDead) return;

    TickInvulnerability(DeltaTime);
    TickRegen(DeltaTime);
}

// ============================================================
//  Initialization
// ============================================================

void UHealthComponent::InitializeHealth(float InMaxHealth, bool bFillToMax)
{
    MaxHealth = FMath::Max(1.0f, InMaxHealth);

    if (bFillToMax)
    {
        CurrentHealth = MaxHealth;
    }
    else
    {
        CurrentHealth = FMath::Clamp(CurrentHealth, 0.0f, MaxHealth);
    }

    bIsDead = false;
    OnHealthChanged.Broadcast(CurrentHealth, MaxHealth);
}

// ============================================================
//  Queries
// ============================================================

float UHealthComponent::GetHealthPercent() const
{
    return MaxHealth > 0.0f ? FMath::Clamp(CurrentHealth / MaxHealth, 0.0f, 1.0f) : 0.0f;
}

bool UHealthComponent::IsAtFullHealth() const
{
    return FMath::IsNearlyEqual(CurrentHealth, MaxHealth, 0.01f);
}

// ============================================================
//  Damage
// ============================================================

float UHealthComponent::ApplyDamage(const FNarutoDamageEvent& DamageEvent)
{
    if (bIsDead || bIsInvulnerable) return 0.0f;
    if (DamageEvent.FinalDamage <= 0.0f) return 0.0f;

    const float PreviousHealth = CurrentHealth;
    CurrentHealth = FMath::Max(0.0f, CurrentHealth - DamageEvent.FinalDamage);
    const float ActualDamage = PreviousHealth - CurrentHealth;

    UE_LOG(LogNarutoUSL, Verbose, TEXT("[HealthComponent] %s received %.1f damage (%.1f -> %.1f / %.1f)"),
        *GetOwner()->GetName(), ActualDamage, PreviousHealth, CurrentHealth, MaxHealth);

    OnHealthChanged.Broadcast(CurrentHealth, MaxHealth);

    if (CurrentHealth <= 0.0f)
    {
        TriggerDeath(DamageEvent);
    }

    return ActualDamage;
}

float UHealthComponent::ApplyHealing(float Amount, FGameplayTag HealerTag)
{
    if (bIsDead || Amount <= 0.0f) return 0.0f;

    const float PreviousHealth = CurrentHealth;
    CurrentHealth = FMath::Min(MaxHealth, CurrentHealth + Amount);
    const float ActualHeal = CurrentHealth - PreviousHealth;

    if (ActualHeal > 0.0f)
    {
        OnHealthChanged.Broadcast(CurrentHealth, MaxHealth);
    }

    return ActualHeal;
}

void UHealthComponent::ApplyDamageOverTimeTick(float DamagePerTick, EDamageType DamageType, FGameplayTag SourceTag)
{
    // DoT bypasses invulnerability but still respects death state.
    if (bIsDead || DamagePerTick <= 0.0f) return;

    const float PreviousHealth = CurrentHealth;
    CurrentHealth = FMath::Max(0.0f, CurrentHealth - DamagePerTick);

    OnHealthChanged.Broadcast(CurrentHealth, MaxHealth);

    if (CurrentHealth <= 0.0f)
    {
        FNarutoDamageEvent KillingDoT;
        KillingDoT.FinalDamage = PreviousHealth;
        KillingDoT.DamageType  = DamageType;
        KillingDoT.SourceJutsuTag = SourceTag;
        TriggerDeath(KillingDoT);
    }
}

// ============================================================
//  Invulnerability
// ============================================================

void UHealthComponent::GrantInvulnerability(float Duration, FGameplayTag SourceTag)
{
    if (!SourceTag.IsValid()) return;

    const float* Existing = InvulnerabilitySources.Find(SourceTag);
    if (Existing && *Existing < 0.0f) return; // Already permanent from this source

    // Take the longer duration
    if (Existing)
    {
        InvulnerabilitySources[SourceTag] = FMath::Max(*Existing, Duration);
    }
    else
    {
        InvulnerabilitySources.Add(SourceTag, Duration);
    }

    const bool bWasInvulnerable = bIsInvulnerable;
    bIsInvulnerable = true;

    if (!bWasInvulnerable)
    {
        OnInvulnerabilityChanged.Broadcast(true, Duration);
    }
}

void UHealthComponent::RemoveInvulnerability(FGameplayTag SourceTag)
{
    InvulnerabilitySources.Remove(SourceTag);

    if (InvulnerabilitySources.IsEmpty())
    {
        bIsInvulnerable = false;
        OnInvulnerabilityChanged.Broadcast(false, 0.0f);
    }
}

void UHealthComponent::ClearAllInvulnerability()
{
    InvulnerabilitySources.Empty();
    bIsInvulnerable = false;
    OnInvulnerabilityChanged.Broadcast(false, 0.0f);
}

// ============================================================
//  Death / Revival
// ============================================================

void UHealthComponent::TriggerDeath(const FNarutoDamageEvent& KillingBlow)
{
    if (bIsDead) return;

    bIsDead = true;
    CurrentHealth = 0.0f;
    ClearAllInvulnerability();

    UE_LOG(LogNarutoUSL, Log, TEXT("[HealthComponent] %s has died."), *GetOwner()->GetName());

    OnDeath.Broadcast(KillingBlow);
}

void UHealthComponent::Revive(float ReviveHealth)
{
    if (!bIsDead) return;

    bIsDead = false;
    CurrentHealth = FMath::Clamp(ReviveHealth, 1.0f, MaxHealth);

    OnRevived.Broadcast(CurrentHealth);
    OnHealthChanged.Broadcast(CurrentHealth, MaxHealth);
}

void UHealthComponent::ForceKill(AActor* Killer)
{
    if (bIsDead) return;

    FNarutoDamageEvent KillEvent;
    KillEvent.FinalDamage = CurrentHealth;
    KillEvent.DamageType  = EDamageType::True;
    KillEvent.Instigator  = Killer;
    KillEvent.Target      = GetOwner();

    CurrentHealth = 0.0f;
    TriggerDeath(KillEvent);
}

// ============================================================
//  Max Health
// ============================================================

void UHealthComponent::SetMaxHealth(float NewMaxHealth, bool bScaleCurrentHealth)
{
    const float OldMax = MaxHealth;
    MaxHealth = FMath::Max(1.0f, NewMaxHealth);

    if (bScaleCurrentHealth && OldMax > 0.0f)
    {
        const float HealthRatio = CurrentHealth / OldMax;
        CurrentHealth = FMath::Clamp(HealthRatio * MaxHealth, 0.0f, MaxHealth);
    }
    else
    {
        CurrentHealth = FMath::Clamp(CurrentHealth, 0.0f, MaxHealth);
    }

    OnHealthChanged.Broadcast(CurrentHealth, MaxHealth);
}

// ============================================================
//  Regen
// ============================================================

void UHealthComponent::SetRegenRate(float HPPerSecond)
{
    RegenRate = FMath::Max(0.0f, HPPerSecond);
}

void UHealthComponent::TickRegen(float DeltaTime)
{
    if (RegenRate <= 0.0f || IsAtFullHealth()) return;

    RegenAccumulator += RegenRate * DeltaTime;

    if (RegenAccumulator >= 1.0f)
    {
        const float HealAmount = FMath::FloorToFloat(RegenAccumulator);
        RegenAccumulator -= HealAmount;
        ApplyHealing(HealAmount, FGameplayTag::RequestGameplayTag(TEXT("Heal.Regen.Passive")));
    }
}

void UHealthComponent::TickInvulnerability(float DeltaTime)
{
    if (!bIsInvulnerable) return;

    TArray<FGameplayTag> Expired;

    for (auto& Pair : InvulnerabilitySources)
    {
        if (Pair.Value < 0.0f) continue; // Permanent — skip

        Pair.Value -= DeltaTime;
        if (Pair.Value <= 0.0f)
        {
            Expired.Add(Pair.Key);
        }
    }

    for (const FGameplayTag& Tag : Expired)
    {
        RemoveInvulnerability(Tag);
    }
}
