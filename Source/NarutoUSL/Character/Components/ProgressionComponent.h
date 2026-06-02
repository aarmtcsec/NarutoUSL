// Copyright (c) 2025 Maze Runner Studios. All Rights Reserved.
// Naruto: Ultimate Shinobi Legacy
// ProgressionComponent — XP, leveling, mastery, titles, and stat resolution.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Core/Types/NarutoTypes.h"
#include "ProgressionComponent.generated.h"

// ============================================================
//  Delegates
// ============================================================

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnLevelUp,          int32, OldLevel,   int32, NewLevel);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnXPGained,         float, Amount,     float, TotalXP);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnMasteryGained,    FGameplayTag, JutsuTag, float, NewMastery);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam (FOnTitleUnlocked,    FGameplayTag, TitleTag);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam (FOnSkillPointsGained, int32, TotalPoints);

// ============================================================
//  Mastery Entry
// ============================================================

USTRUCT(BlueprintType)
struct NARUTOUSL_API FJutsuMasteryEntry
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly) FGameplayTag JutsuTag;
    UPROPERTY(BlueprintReadOnly) float MasteryXP    = 0.0f;
    UPROPERTY(BlueprintReadOnly) int32 MasteryLevel = 0;

    /** Cached damage multiplier at current mastery level. */
    UPROPERTY(BlueprintReadOnly) float DamageMultiplier = 1.0f;

    /** Cached chakra cost multiplier at current mastery level. */
    UPROPERTY(BlueprintReadOnly) float CostMultiplier = 1.0f;
};

// ============================================================
//  ProgressionComponent
// ============================================================

/**
 * UProgressionComponent
 *
 * Manages all character progression: level, XP, skill points,
 * jutsu mastery, and unlocked titles. Resolves final stat values
 * by combining base stats, level scaling, equipment modifiers,
 * and active skill tree bonuses.
 *
 * Stat resolution order:
 *   1. Base stat (from CharacterData)
 *   2. + Level scaling (level * per-level value)
 *   3. + Flat modifiers (equipment, skills, buffs)
 *   4. × Multiplier modifiers (skills, awakenings, buffs)
 *   5. = Final resolved stat
 */
UCLASS(ClassGroup = "NarutoUSL|Character", meta = (BlueprintSpawnableComponent))
class NARUTOUSL_API UProgressionComponent : public UActorComponent
{
    GENERATED_BODY()

public:

    UProgressionComponent();

    virtual void BeginPlay() override;

    // ------------------------------------------------------------------
    //  Initialization
    // ------------------------------------------------------------------

    void InitializeProgression(int32 StartingLevel, float InBaseXPToLevel);

    // ------------------------------------------------------------------
    //  Level
    // ------------------------------------------------------------------

    UFUNCTION(BlueprintPure, Category = "Progression|Level")
    int32 GetLevel() const { return CurrentLevel; }

    UFUNCTION(BlueprintPure, Category = "Progression|Level")
    float GetCurrentXP() const { return CurrentXP; }

    UFUNCTION(BlueprintPure, Category = "Progression|Level")
    float GetXPToNextLevel() const;

    UFUNCTION(BlueprintPure, Category = "Progression|Level")
    float GetLevelProgress() const;

    UFUNCTION(BlueprintCallable, Category = "Progression|Level")
    void GainXP(float Amount, FGameplayTag SourceTag);

    UFUNCTION(BlueprintCallable, Category = "Progression|Level")
    void SetLevel(int32 NewLevel, bool bPreserveXP = false);

    // ------------------------------------------------------------------
    //  Skill Points
    // ------------------------------------------------------------------

    UFUNCTION(BlueprintPure, Category = "Progression|SkillPoints")
    int32 GetAvailableSkillPoints() const { return AvailableSkillPoints; }

    UFUNCTION(BlueprintCallable, Category = "Progression|SkillPoints")
    bool SpendSkillPoint();

    UFUNCTION(BlueprintCallable, Category = "Progression|SkillPoints")
    void AddSkillPoints(int32 Amount);

    // ------------------------------------------------------------------
    //  Stat Resolution
    // ------------------------------------------------------------------

    /**
     * Returns the fully resolved final value for the specified stat.
     * Combines base, level scaling, and all active modifiers.
     */
    UFUNCTION(BlueprintPure, Category = "Progression|Stats")
    float GetResolvedStat(ECharacterStat Stat) const;

    /**
     * Adds a stat modifier. Modifiers are stacked and resolved each query.
     * Permanent modifiers (Duration = -1) persist until explicitly removed.
     */
    UFUNCTION(BlueprintCallable, Category = "Progression|Stats")
    void AddStatModifier(const FStatModifier& Modifier);

    /** Removes all modifiers with the specified source tag. */
    UFUNCTION(BlueprintCallable, Category = "Progression|Stats")
    void RemoveStatModifiersFromSource(FGameplayTag SourceTag);

    /** Removes all timed modifiers that have expired. Called each tick. */
    void TickStatModifiers(float DeltaTime);

    // ------------------------------------------------------------------
    //  Jutsu Mastery
    // ------------------------------------------------------------------

    UFUNCTION(BlueprintPure, Category = "Progression|Mastery")
    float GetJutsuMastery(FGameplayTag JutsuTag) const;

    UFUNCTION(BlueprintPure, Category = "Progression|Mastery")
    int32 GetJutsuMasteryLevel(FGameplayTag JutsuTag) const;

    UFUNCTION(BlueprintCallable, Category = "Progression|Mastery")
    void GainJutsuMastery(FGameplayTag JutsuTag, float Amount);

    UFUNCTION(BlueprintPure, Category = "Progression|Mastery")
    float GetJutsuDamageMultiplier(FGameplayTag JutsuTag) const;

    UFUNCTION(BlueprintPure, Category = "Progression|Mastery")
    float GetJutsuCostMultiplier(FGameplayTag JutsuTag) const;

    // ------------------------------------------------------------------
    //  Titles
    // ------------------------------------------------------------------

    UFUNCTION(BlueprintCallable, Category = "Progression|Titles")
    void UnlockTitle(FGameplayTag TitleTag);

    UFUNCTION(BlueprintPure, Category = "Progression|Titles")
    bool HasTitle(FGameplayTag TitleTag) const;

    UFUNCTION(BlueprintPure, Category = "Progression|Titles")
    const TArray<FGameplayTag>& GetUnlockedTitles() const { return UnlockedTitles; }

    UFUNCTION(BlueprintCallable, Category = "Progression|Titles")
    void SetActiveTitle(FGameplayTag TitleTag);

    UFUNCTION(BlueprintPure, Category = "Progression|Titles")
    FGameplayTag GetActiveTitle() const { return ActiveTitle; }

    // ------------------------------------------------------------------
    //  Base Stat Configuration (set from CharacterData)
    // ------------------------------------------------------------------

    void SetBaseStats(const TMap<ECharacterStat, float>& InBaseStats);
    void SetLevelScaling(const TMap<ECharacterStat, float>& InScaling);

    // ------------------------------------------------------------------
    //  Events
    // ------------------------------------------------------------------

    UPROPERTY(BlueprintAssignable, Category = "Progression|Events")
    FOnLevelUp OnLevelUp;

    UPROPERTY(BlueprintAssignable, Category = "Progression|Events")
    FOnXPGained OnXPGained;

    UPROPERTY(BlueprintAssignable, Category = "Progression|Events")
    FOnMasteryGained OnMasteryGained;

    UPROPERTY(BlueprintAssignable, Category = "Progression|Events")
    FOnTitleUnlocked OnTitleUnlocked;

    UPROPERTY(BlueprintAssignable, Category = "Progression|Events")
    FOnSkillPointsGained OnSkillPointsGained;

protected:

    void ProcessLevelUp();
    float CalculateXPRequiredForLevel(int32 Level) const;
    void RecalculateMasteryBonuses(FJutsuMasteryEntry& Entry);

    // ------------------------------------------------------------------
    //  State
    // ------------------------------------------------------------------

    UPROPERTY(BlueprintReadOnly, Category = "Progression")
    int32 CurrentLevel = 1;

    UPROPERTY(BlueprintReadOnly, Category = "Progression")
    float CurrentXP = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Progression")
    int32 AvailableSkillPoints = 0;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Progression", meta = (ClampMin = "100"))
    float BaseXPToLevel = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Progression", meta = (ClampMin = "1.0"))
    float XPScalingExponent = 1.5f;

    // Stats
    TMap<ECharacterStat, float> BaseStats;
    TMap<ECharacterStat, float> LevelScaling;
    TArray<FStatModifier>       ActiveModifiers;

    // Mastery
    TMap<FGameplayTag, FJutsuMasteryEntry> MasteryMap;

    // Titles
    UPROPERTY(BlueprintReadOnly, Category = "Progression")
    TArray<FGameplayTag> UnlockedTitles;

    UPROPERTY(BlueprintReadOnly, Category = "Progression")
    FGameplayTag ActiveTitle;
};
