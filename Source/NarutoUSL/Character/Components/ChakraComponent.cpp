// Copyright (c) 2025 Maze Runner Studios. All Rights Reserved.

#include "Character/Components/ChakraComponent.h"
#include "Core/Settings/NarutoGameSettings.h"
#include "NarutoUSL.h"

UChakraComponent::UChakraComponent()
{
    // Ticking is managed by ChakraSystem, not the component itself.
    PrimaryComponentTick.bCanEverTick = false;
}

void UChakraComponent::BeginPlay()
{
    Super::BeginPlay();
}

// ============================================================
//  Initialization
// ============================================================

void UChakraComponent::InitializeChakra(float InMaxChakra, EChakraNature InNatureAffinities,
                                         float InRegenRate, bool bFillToMax)
{
    MaxChakra        = FMath::Max(1.0f, InMaxChakra);
    NatureAffinities = InNatureAffinities;
    RegenRate        = FMath::Max(0.0f, InRegenRate);

    if (bFillToMax)
    {
        CurrentChakra = MaxChakra;
    }
    else
    {
        CurrentChakra = FMath::Clamp(CurrentChakra, 0.0f, MaxChakra);
    }

    BroadcastChakraChanged();
}

// ============================================================
//  Queries
// ============================================================

float UChakraComponent::GetChakraPercent() const
{
    return MaxChakra > 0.0f ? FMath::Clamp(CurrentChakra / MaxChakra, 0.0f, 1.0f) : 0.0f;
}

bool UChakraComponent::HasNatureAffinity(EChakraNature Nature) const
{
    return EnumHasAnyFlags(NatureAffinities, Nature);
}

bool UChakraComponent::IsAtMaxChakra() const
{
    return FMath::IsNearlyEqual(CurrentChakra, MaxChakra, 0.01f);
}

bool UChakraComponent::CanAffordCost(const FChakraCost& Cost) const
{
    if (Cost.RequiredNature != EChakraNature::None && !HasNatureAffinity(Cost.RequiredNature))
    {
        return false;
    }

    if (CurrentChakra < Cost.MinimumChakraRequired)
    {
        return false;
    }

    const float EffectiveCost = ResolveEffectiveCost(Cost);
    return CurrentChakra >= EffectiveCost;
}

float UChakraComponent::ResolveEffectiveCost(const FChakraCost& Cost) const
{
    float Total = Cost.FlatCost + (MaxChakra * Cost.PercentCost);

    // Apply nature affinity discount
    if (Cost.RequiredNature != EChakraNature::None && HasNatureAffinity(Cost.RequiredNature))
    {
        Total *= (1.0f - NatureAffinityDiscount);
    }

    return FMath::Max(0.0f, Total);
}

// ============================================================
//  Consumption
// ============================================================

float UChakraComponent::ConsumeChakra(float Amount, bool bAllowPartial)
{
    if (Amount <= 0.0f) return 0.0f;

    if (!bAllowPartial && CurrentChakra < Amount)
    {
        return 0.0f;
    }

    const float Consumed = FMath::Min(Amount, CurrentChakra);
    CurrentChakra -= Consumed;

    BroadcastChakraChanged();

    if (CurrentChakra <= 0.0f)
    {
        CurrentChakra = 0.0f;
        OnChakraDepleted.Broadcast();
        UE_LOG(LogNarutoChakra, Verbose, TEXT("[ChakraComponent] %s chakra depleted."),
            *GetOwner()->GetName());
    }

    return Consumed;
}

bool UChakraComponent::ConsumeCost(const FChakraCost& Cost)
{
    if (!CanAffordCost(Cost)) return false;

    const float EffectiveCost = ResolveEffectiveCost(Cost);
    ConsumeChakra(EffectiveCost);
    return true;
}

// ============================================================
//  Restoration
// ============================================================

float UChakraComponent::RestoreChakra(float Amount)
{
    if (Amount <= 0.0f || IsAtMaxChakra()) return 0.0f;

    const float Previous = CurrentChakra;
    CurrentChakra = FMath::Min(MaxChakra, CurrentChakra + Amount);
    const float Restored = CurrentChakra - Previous;

    if (Restored > 0.0f)
    {
        BroadcastChakraChanged();

        if (IsAtMaxChakra())
        {
            OnChakraFull.Broadcast();
        }
    }

    return Restored;
}

void UChakraComponent::SetChakra(float NewValue)
{
    CurrentChakra = FMath::Clamp(NewValue, 0.0f, MaxChakra);
    BroadcastChakraChanged();
}

void UChakraComponent::FillChakra()
{
    SetChakra(MaxChakra);
    OnChakraFull.Broadcast();
}

// ============================================================
//  Regen Control
// ============================================================

void UChakraComponent::SetRegenMode(EChakraRegenMode NewMode)
{
    RegenMode = NewMode;
}

void UChakraComponent::SetRegenRate(float NewRate)
{
    RegenRate = FMath::Max(0.0f, NewRate);
}

void UChakraComponent::SetRegenMultiplier(float Multiplier, FGameplayTag SourceTag, float Duration)
{
    if (!SourceTag.IsValid()) return;
    RegenMultipliers.Add(SourceTag, FMath::Max(0.0f, Multiplier));
    RegenMultiplierDurations.Add(SourceTag, Duration);
}

void UChakraComponent::RemoveRegenMultiplier(FGameplayTag SourceTag)
{
    RegenMultipliers.Remove(SourceTag);
    RegenMultiplierDurations.Remove(SourceTag);
}

// ============================================================
//  Sharing
// ============================================================

bool UChakraComponent::CanShareChakra() const
{
    return bCanShareChakra && RegenMode != EChakraRegenMode::Suppressed && CurrentChakra > 0.0f;
}

float UChakraComponent::TransferChakraTo(UChakraComponent* Recipient, float Amount)
{
    if (!Recipient || !CanShareChakra() || Amount <= 0.0f) return 0.0f;

    const UNarutoGameSettings* Settings = GetDefault<UNarutoGameSettings>();
    const float MaxTransfer = MaxChakra * Settings->ChakraShareMaxPercent;
    const float ActualAmount = FMath::Min(Amount, FMath::Min(CurrentChakra, MaxTransfer));

    if (ActualAmount <= 0.0f) return 0.0f;

    ConsumeChakra(ActualAmount);
    Recipient->RestoreChakra(ActualAmount);

    OnChakraShared.Broadcast(Recipient->GetOwner(), ActualAmount);

    UE_LOG(LogNarutoChakra, Log, TEXT("[ChakraComponent] %s transferred %.1f chakra to %s."),
        *GetOwner()->GetName(), ActualAmount, *Recipient->GetOwner()->GetName());

    return ActualAmount;
}

// ============================================================
//  Sage Chakra
// ============================================================

void UChakraComponent::InitializeSageChakra(float InMaxSageChakra)
{
    MaxSageChakra = FMath::Max(0.0f, InMaxSageChakra);
    SageChakra    = 0.0f;
}

bool UChakraComponent::HasFullSageChakra() const
{
    return MaxSageChakra > 0.0f && FMath::IsNearlyEqual(SageChakra, MaxSageChakra, 0.01f);
}

void UChakraComponent::AccumulateSageChakra(float Amount)
{
    if (MaxSageChakra <= 0.0f) return;
    SageChakra = FMath::Min(MaxSageChakra, SageChakra + Amount);
}

void UChakraComponent::ConsumeSageChakra(float Amount)
{
    SageChakra = FMath::Max(0.0f, SageChakra - Amount);
}

// ============================================================
//  Tick (driven by ChakraSystem)
// ============================================================

void UChakraComponent::TickChakra(float DeltaTime)
{
    ApplyRegenTick(DeltaTime);

    // Tick regen multiplier durations
    TArray<FGameplayTag> Expired;
    for (auto& Pair : RegenMultiplierDurations)
    {
        if (Pair.Value < 0.0f) continue;
        Pair.Value -= DeltaTime;
        if (Pair.Value <= 0.0f) Expired.Add(Pair.Key);
    }
    for (const FGameplayTag& Tag : Expired)
    {
        RemoveRegenMultiplier(Tag);
    }
}

void UChakraComponent::ApplyRegenTick(float DeltaTime)
{
    if (RegenMode == EChakraRegenMode::Suppressed) return;
    if (IsAtMaxChakra()) return;

    const float EffectiveRate = GetEffectiveRegenRate();
    if (FMath::IsNearlyZero(EffectiveRate)) return;

    if (RegenMode == EChakraRegenMode::Drain)
    {
        // Drain mode: negative regen (transformation cost)
        ConsumeChakra(FMath::Abs(EffectiveRate) * DeltaTime, true);
        return;
    }

    RegenAccumulator += EffectiveRate * DeltaTime;

    if (RegenAccumulator >= 1.0f)
    {
        const float RegenAmount = FMath::FloorToFloat(RegenAccumulator);
        RegenAccumulator -= RegenAmount;
        RestoreChakra(RegenAmount);
    }
}

float UChakraComponent::GetEffectiveRegenRate() const
{
    const UNarutoGameSettings* Settings = GetDefault<UNarutoGameSettings>();

    float Rate = RegenRate;

    if (RegenMode == EChakraRegenMode::Meditation)
    {
        Rate *= Settings->MeditationChakraRegenMultiplier;
    }
    else if (RegenMode == EChakraRegenMode::Drain)
    {
        return -Settings->TransformationChakraDrainRate;
    }

    // Apply all active multipliers
    for (const auto& Pair : RegenMultipliers)
    {
        Rate *= Pair.Value;
    }

    return Rate;
}

void UChakraComponent::BroadcastChakraChanged()
{
    OnChakraChanged.Broadcast(CurrentChakra, MaxChakra);
}
