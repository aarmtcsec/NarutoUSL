// Copyright (c) 2025 Maze Runner Studios. All Rights Reserved.
// Naruto: Ultimate Shinobi Legacy
// NarutoCharacterData — Primary data asset for every playable character.
// One asset per character. Drives stats, jutsu loadout, AI, audio, and visuals.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "Core/Types/NarutoTypes.h"
#include "NarutoCharacterData.generated.h"

class USkeletalMesh;
class UAnimBlueprint;
class UBehaviorTree;
class USoundBase;
class UNiagaraSystem;
class UJutsuData;
class USkillTreeData;

// ============================================================
//  Base Stat Block
// ============================================================

/**
 * FCharacterBaseStats
 * Raw base values before any equipment, skill, or level scaling.
 * All scaling is applied at runtime by the StatResolver.
 */
USTRUCT(BlueprintType)
struct NARUTOUSL_API FCharacterBaseStats
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stats", meta = (ClampMin = "1"))
    float MaxHealth = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stats", meta = (ClampMin = "1"))
    float MaxChakra = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stats", meta = (ClampMin = "1"))
    float MaxStamina = 200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stats", meta = (ClampMin = "0"))
    float PhysicalAttack = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stats", meta = (ClampMin = "0"))
    float ChakraAttack = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stats", meta = (ClampMin = "0"))
    float PhysicalDefense = 50.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stats", meta = (ClampMin = "0"))
    float ChakraDefense = 50.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stats", meta = (ClampMin = "100", ClampMax = "2000"))
    float MovementSpeed = 600.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stats", meta = (ClampMin = "0"))
    float ChakraRegenRate = 5.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stats", meta = (ClampMin = "0"))
    float HealthRegenRate = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stats", meta = (ClampMin = "0", ClampMax = "1"))
    float CriticalChance = 0.05f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stats", meta = (ClampMin = "1"))
    float CriticalMultiplier = 1.75f;
};

// ============================================================
//  Level Scaling
// ============================================================

/**
 * FCharacterLevelScaling
 * Per-level stat growth values. Applied additively each level.
 */
USTRUCT(BlueprintType)
struct NARUTOUSL_API FCharacterLevelScaling
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Scaling")
    float HealthPerLevel = 50.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Scaling")
    float ChakraPerLevel = 20.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Scaling")
    float PhysicalAttackPerLevel = 5.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Scaling")
    float ChakraAttackPerLevel = 5.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Scaling")
    float PhysicalDefensePerLevel = 2.5f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Scaling")
    float ChakraDefensePerLevel = 2.5f;

    /** Optional curve override — if set, overrides flat-per-level for health. */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Scaling")
    TSoftObjectPtr<UCurveFloat> HealthScalingCurve;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Scaling")
    TSoftObjectPtr<UCurveFloat> ChakraScalingCurve;
};

// ============================================================
//  Awakening / Transformation Definition
// ============================================================

USTRUCT(BlueprintType)
struct NARUTOUSL_API FAwakeningData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Awakening")
    FText DisplayName;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Awakening")
    FGameplayTag AwakeningTag;

    /** Minimum chakra percent required to activate. */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Awakening", meta = (ClampMin = "0", ClampMax = "1"))
    float MinChakraPercent = 0.3f;

    /** Chakra drain per second while awakened. */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Awakening", meta = (ClampMin = "0"))
    float ChakraDrainPerSecond = 15.0f;

    /** Stat multipliers applied during awakening. */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Awakening")
    float HealthMultiplier = 1.5f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Awakening")
    float AttackMultiplier = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Awakening")
    float SpeedMultiplier = 1.3f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Awakening")
    TSoftObjectPtr<USkeletalMesh> AwakeningMesh;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Awakening")
    TSoftObjectPtr<UNiagaraSystem> ActivationVFX;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Awakening")
    TSoftObjectPtr<USoundBase> ActivationSFX;

    /** Jutsu unlocked only during this awakening state. */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Awakening")
    TArray<TSoftObjectPtr<UJutsuData>> ExclusiveJutsu;
};

// ============================================================
//  Voice Pack Reference
// ============================================================

USTRUCT(BlueprintType)
struct NARUTOUSL_API FCharacterVoicePack
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Voice")
    TSoftObjectPtr<USoundBase> IdleVoiceLine;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Voice")
    TArray<TSoftObjectPtr<USoundBase>> AttackVoiceLines;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Voice")
    TArray<TSoftObjectPtr<USoundBase>> HitVoiceLines;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Voice")
    TArray<TSoftObjectPtr<USoundBase>> DeathVoiceLines;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Voice")
    TArray<TSoftObjectPtr<USoundBase>> JutsuActivationLines;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Voice")
    TArray<TSoftObjectPtr<USoundBase>> UltimateJutsuLines;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Voice")
    TArray<TSoftObjectPtr<USoundBase>> VictoryLines;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Voice")
    TArray<TSoftObjectPtr<USoundBase>> DefeatLines;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Voice")
    TArray<TSoftObjectPtr<USoundBase>> LowHealthLines;
};

// ============================================================
//  NarutoCharacterData
// ============================================================

/**
 * UNarutoCharacterData
 *
 * Primary data asset for a playable or boss character.
 * Contains everything needed to instantiate, configure, and drive
 * a character at runtime. No character-specific logic lives here —
 * only data. Logic lives in components and the character class.
 *
 * Naming convention: DA_Character_Naruto, DA_Character_Sasuke, etc.
 */
UCLASS(BlueprintType)
class NARUTOUSL_API UNarutoCharacterData : public UPrimaryDataAsset
{
    GENERATED_BODY()

public:

    virtual FPrimaryAssetId GetPrimaryAssetId() const override
    {
        return FPrimaryAssetId(TEXT("CharacterData"), GetFName());
    }

    // ------------------------------------------------------------------
    //  Identity
    // ------------------------------------------------------------------

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Identity")
    FText DisplayName;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Identity")
    FText ShortDescription;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Identity")
    FText LoreDescription;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Identity")
    FGameplayTag CharacterTag;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Identity")
    EVillage PrimaryVillage = EVillage::Konohagakure;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Identity")
    EShinobi_Rank ShinobiRank = EShinobi_Rank::Genin;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Identity")
    TSoftObjectPtr<UTexture2D> PortraitTexture;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Identity")
    TSoftObjectPtr<UTexture2D> FullBodyTexture;

    // ------------------------------------------------------------------
    //  Visuals
    // ------------------------------------------------------------------

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Visuals")
    TSoftObjectPtr<USkeletalMesh> CharacterMesh;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Visuals")
    TSoftClassPtr<UAnimInstance> AnimInstanceClass;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Visuals")
    TArray<TSoftObjectPtr<UMaterialInterface>> CharacterMaterials;

    // ------------------------------------------------------------------
    //  Stats
    // ------------------------------------------------------------------

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stats")
    FCharacterBaseStats BaseStats;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stats")
    FCharacterLevelScaling LevelScaling;

    // ------------------------------------------------------------------
    //  Chakra
    // ------------------------------------------------------------------

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Chakra",
        meta = (Bitmask, BitmaskEnum = "EChakraNature"))
    int32 NatureAffinities = 0;

    /** Primary nature used for jutsu cost reduction. */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Chakra")
    EChakraNature PrimaryNature = EChakraNature::None;

    // ------------------------------------------------------------------
    //  Jutsu
    // ------------------------------------------------------------------

    /** All jutsu this character can learn. Subset may be locked behind progression. */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Jutsu")
    TArray<TSoftObjectPtr<UJutsuData>> AvailableJutsu;

    /** Jutsu equipped by default at game start. */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Jutsu")
    TArray<TSoftObjectPtr<UJutsuData>> DefaultEquippedJutsu;

    /** Signature ultimate jutsu (e.g., Rasengan, Chidori). */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Jutsu")
    TSoftObjectPtr<UJutsuData> UltimateJutsu;

    // ------------------------------------------------------------------
    //  Skill Tree
    // ------------------------------------------------------------------

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Progression")
    TSoftObjectPtr<USkillTreeData> SkillTree;

    // ------------------------------------------------------------------
    //  Awakening / Transformations
    // ------------------------------------------------------------------

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Awakening")
    TArray<FAwakeningData> Awakenings;

    // ------------------------------------------------------------------
    //  AI
    // ------------------------------------------------------------------

    /** Behavior tree used when this character is AI-controlled. */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI")
    TSoftObjectPtr<UBehaviorTree> AIBehaviorTree;

    /** Tags that describe this character's AI combat style. */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI")
    FGameplayTagContainer AICombatStyleTags;

    // ------------------------------------------------------------------
    //  Audio
    // ------------------------------------------------------------------

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Audio")
    FCharacterVoicePack VoicePack;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Audio")
    TSoftObjectPtr<USoundBase> CombatMusicOverride;

    // ------------------------------------------------------------------
    //  Damage Resistances
    // ------------------------------------------------------------------

    /** Per-damage-type resistance multipliers. 1.0 = normal, 0.5 = 50% resist. */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Resistances")
    TMap<EDamageType, float> DamageResistances;

    // ------------------------------------------------------------------
    //  Unlock Conditions
    // ------------------------------------------------------------------

    /** If true, this character is available from the start. */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Unlock")
    bool bUnlockedByDefault = false;

    /** Quest tag that must be completed to unlock this character. */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Unlock",
        meta = (EditCondition = "!bUnlockedByDefault"))
    FGameplayTag UnlockQuestTag;

    /** Reputation requirement for unlock (faction + tier). */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Unlock",
        meta = (EditCondition = "!bUnlockedByDefault"))
    EVillage UnlockReputationFaction = EVillage::None;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Unlock",
        meta = (EditCondition = "!bUnlockedByDefault"))
    EReputationTier UnlockReputationTier = EReputationTier::Neutral;
};
