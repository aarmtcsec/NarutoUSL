// Copyright (c) 2025 Maze Runner Studios. All Rights Reserved.

#include "Character/Components/ProgressionComponent.h"
#include "Core/Settings/NarutoGameSettings.h"
#include "NarutoUSL.h"
#include "Math/UnrealMathUtility.h"

UProgressionComponent::UProgressionComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}

void UProgressionComponent::BeginPlay()
{
    Super::BeginPlay();
}

void UProgressionComponent::InitializeProgression(int32 StartingLevel, float InBaseXPToLevel)
{
    BaseXPToLevel = FMath::Max(100.0f, InBaseXPToLevel);
    SetLevel(FMath::Clamp(StartingLevel, 1,
        GetDefault<UNarutoGameSettings>()->MaxCharacterLevel), false);
}

// ============================================================
//  Level
// ============================================================

float UProgressionComponent::GetXPToNextLevel() const
{
    return CalculateXPRequiredForLevel(CurrentLevel + 1);
}

float UProgressionComponent::GetLevelProgress() const
{
    const float Required = GetXPToNextLevel();
    return Required > 0.0f ? FMath::Clamp(CurrentXP / Required, 0.0f, 1.0f) : 1.0f;
}

void UProgressionComponent::GainXP(float Amount, FGameplayTag SourceTag)
{
    if (Amount <= 0.0f) return;

    const UNarutoGameSettings* Settings = GetDefault<UNarutoGameSettings>();
    if (CurrentLevel >= Settings->MaxCharacterLevel) return;

    CurrentXP += Amount;
    OnXPGained.Broadcast(Amount, CurrentXP);

    // Check for level-up (may level up multiple times from a single XP gain)
    while (CurrentXP >= GetXPToNextLevel() && CurrentLevel < Settings->MaxCharacterLevel)
    {
        CurrentXP -= GetXPToNextLevel();
        ProcessLevelUp();
    }
}

void UProgressionComponent::SetLevel(int32 NewLevel, bool bPreserveXP)
{
    const UNarutoGameSettings* Settings = GetDefault<UNarutoGameSettings>();
    const int32 ClampedLevel = FMath::Clamp(NewLevel, 1, Settings->MaxCharacterLevel);

    const int32 OldLevel = CurrentLevel;
    CurrentLevel = ClampedLevel;

    if (!bPreserveXP)
    {
        CurrentXP = 0.0f;
    }

    if (OldLevel != CurrentLevel)
    {
        OnLevelUp.Broadcast(OldLevel, CurrentLevel);
    }
}

void UProgressionComponent::ProcessLevelUp()
{
    const int32 OldLevel = CurrentLevel;
    ++CurrentLevel;

    // Award skill points on level-up
    AddSkillPoints(1);

    UE_LOG(LogNarutoUSL, Log, TEXT("[ProgressionComponent] %s leveled up: %d -> %d"),
        *GetOwner()->GetName(), OldLevel, CurrentLevel);

    OnLevelUp.Broadcast(OldLevel, CurrentLevel);
}

float UProgressionComponent::CalculateXPRequiredForLevel(int32 Level) const
{
    // XP formula: Base * Level^Exponent
    // At default values (Base=1000, Exp=1.5):
    //   Level 2:  1000
    //   Level 10: ~31,623
    //   Level 50: ~353,553
    //   Level 100: ~1,000,000
    return BaseXPToLevel * FMath::Pow(static_cast<float>(Level), XPScalingExponent);
}

// ============================================================
//  Skill Points
// ============================================================

bool UProgressionComponent::SpendSkillPoint()
{
    if (AvailableSkillPoints <= 0) return false;
    --AvailableSkillPoints;
    return true;
}

void UProgressionComponent::AddSkillPoints(int32 Amount)
{
    AvailableSkillPoints += FMath::Max(0, Amount);
    OnSkillPointsGained.Broadcast(AvailableSkillPoints);
}

// ============================================================
//  Stat Resolution
// ============================================================

float UProgressionComponent::GetResolvedStat(ECharacterStat Stat) const
{
    // 1. Base value
    const float* BasePtr = BaseStats.Find(Stat);
    float Value = BasePtr ? *BasePtr : 0.0f;

    // 2. Level scaling
    const float* ScalePtr = LevelScaling.Find(Stat);
    if (ScalePtr)
    {
        Value += *ScalePtr * static_cast<float>(CurrentLevel - 1);
    }

    // 3. Flat modifiers (sorted by priority, applied in order)
    float FlatBonus = 0.0f;
    float Multiplier = 1.0f;

    // Collect and sort modifiers by priority
    TArray<const FStatModifier*> Relevant;
    for (const FStatModifier& Mod : ActiveModifiers)
    {
        if (Mod.StatType == Stat)
        {
            Relevant.Add(&Mod);
        }
    }

    Relevant.Sort([](const FStatModifier& A, const FStatModifier& B)
    {
        return A.Priority < B.Priority;
    });

    for (const FStatModifier* Mod : Relevant)
    {
        FlatBonus  += Mod->FlatBonus;
        Multiplier *= Mod->Multiplier;
    }

    // 4. Apply flat then multiply
    Value = (Value + FlatBonus) * Multiplier;

    return FMath::Max(0.0f, Value);
}

void UProgressionComponent::AddStatModifier(const FStatModifier& Modifier)
{
    ActiveModifiers.Add(Modifier);
}

void UProgressionComponent::RemoveStatModifiersFromSource(FGameplayTag SourceTag)
{
    ActiveModifiers.RemoveAll([&SourceTag](const FStatModifier& Mod)
    {
        return Mod.SourceTag == SourceTag;
    });
}

void UProgressionComponent::TickStatModifiers(float DeltaTime)
{
    bool bAnyRemoved = false;

    for (int32 i = ActiveModifiers.Num() - 1; i >= 0; --i)
    {
        FStatModifier& Mod = ActiveModifiers[i];
        if (Mod.Duration < 0.0f) continue; // Permanent

        Mod.Duration -= DeltaTime;
        if (Mod.Duration <= 0.0f)
        {
            ActiveModifiers.RemoveAt(i);
            bAnyRemoved = true;
        }
    }
}

void UProgressionComponent::SetBaseStats(const TMap<ECharacterStat, float>& InBaseStats)
{
    BaseStats = InBaseStats;
}

void UProgressionComponent::SetLevelScaling(const TMap<ECharacterStat, float>& InScaling)
{
    LevelScaling = InScaling;
}

// ============================================================
//  Mastery
// ============================================================

float UProgressionComponent::GetJutsuMastery(FGameplayTag JutsuTag) const
{
    const FJutsuMasteryEntry* Entry = MasteryMap.Find(JutsuTag);
    return Entry ? Entry->MasteryXP : 0.0f;
}

int32 UProgressionComponent::GetJutsuMasteryLevel(FGameplayTag JutsuTag) const
{
    const FJutsuMasteryEntry* Entry = MasteryMap.Find(JutsuTag);
    return Entry ? Entry->MasteryLevel : 0;
}

void UProgressionComponent::GainJutsuMastery(FGameplayTag JutsuTag, float Amount)
{
    if (!JutsuTag.IsValid() || Amount <= 0.0f) return;

    const UNarutoGameSettings* Settings = GetDefault<UNarutoGameSettings>();

    FJutsuMasteryEntry& Entry = MasteryMap.FindOrAdd(JutsuTag);
    Entry.JutsuTag = JutsuTag;

    const int32 OldLevel = Entry.MasteryLevel;
    Entry.MasteryXP += Amount;

    // Each mastery level requires 1000 * (Level+1) XP
    const float XPRequired = 1000.0f * static_cast<float>(Entry.MasteryLevel + 1);
    while (Entry.MasteryXP >= XPRequired && Entry.MasteryLevel < Settings->MaxJutsuMasteryLevel)
    {
        Entry.MasteryXP -= XPRequired;
        ++Entry.MasteryLevel;
        RecalculateMasteryBonuses(Entry);
    }

    if (Entry.MasteryLevel != OldLevel)
    {
        OnMasteryGained.Broadcast(JutsuTag, static_cast<float>(Entry.MasteryLevel));
    }
}

float UProgressionComponent::GetJutsuDamageMultiplier(FGameplayTag JutsuTag) const
{
    const FJutsuMasteryEntry* Entry = MasteryMap.Find(JutsuTag);
    return Entry ? Entry->DamageMultiplier : 1.0f;
}

float UProgressionComponent::GetJutsuCostMultiplier(FGameplayTag JutsuTag) const
{
    const FJutsuMasteryEntry* Entry = MasteryMap.Find(JutsuTag);
    return Entry ? Entry->CostMultiplier : 1.0f;
}

void UProgressionComponent::RecalculateMasteryBonuses(FJutsuMasteryEntry& Entry)
{
    // Each mastery level grants +5% damage and -3% chakra cost
    const float LevelF = static_cast<float>(Entry.MasteryLevel);
    Entry.DamageMultiplier = 1.0f + (LevelF * 0.05f);
    Entry.CostMultiplier   = FMath::Max(0.5f, 1.0f - (LevelF * 0.03f));
}

// ============================================================
//  Titles
// ============================================================

void UProgressionComponent::UnlockTitle(FGameplayTag TitleTag)
{
    if (!TitleTag.IsValid() || HasTitle(TitleTag)) return;

    UnlockedTitles.Add(TitleTag);
    OnTitleUnlocked.Broadcast(TitleTag);

    UE_LOG(LogNarutoUSL, Log, TEXT("[ProgressionComponent] %s unlocked title: %s"),
        *GetOwner()->GetName(), *TitleTag.ToString());
}

bool UProgressionComponent::HasTitle(FGameplayTag TitleTag) const
{
    return UnlockedTitles.Contains(TitleTag);
}

void UProgressionComponent::SetActiveTitle(FGameplayTag TitleTag)
{
    if (!HasTitle(TitleTag)) return;
    ActiveTitle = TitleTag;
}
