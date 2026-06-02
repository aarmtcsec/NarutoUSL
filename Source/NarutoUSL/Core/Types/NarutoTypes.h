// Copyright (c) 2025 Maze Runner Studios. All Rights Reserved.
// Naruto: Ultimate Shinobi Legacy
// Core type definitions shared across all systems.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Engine/DataTable.h"
#include "NarutoTypes.generated.h"

// ============================================================
//  Forward Declarations
// ============================================================

class UNarutoCharacterData;
class UJutsuData;
class UFactionData;

// ============================================================
//  Version
// ============================================================

/** Save data schema version. Increment on breaking changes. */
UENUM(BlueprintType)
enum class ENarutoSaveVersion : uint8
{
    Initial         = 0,
    AddedMastery    = 1,
    AddedFactions   = 2,
    AddedTitles     = 3,
    AddedCloudSync  = 4,
    Current         = AddedCloudSync
};

// ============================================================
//  Chakra Nature Types
// ============================================================

UENUM(BlueprintType, meta = (Bitflags, UseEnumValuesAsMaskValuesInEditor = "true"))
enum class EChakraNature : uint8
{
    None        = 0         UMETA(DisplayName = "None"),
    Fire        = 1 << 0    UMETA(DisplayName = "Fire Release"),
    Water       = 1 << 1    UMETA(DisplayName = "Water Release"),
    Earth       = 1 << 2    UMETA(DisplayName = "Earth Release"),
    Wind        = 1 << 3    UMETA(DisplayName = "Wind Release"),
    Lightning   = 1 << 4    UMETA(DisplayName = "Lightning Release"),
    // Advanced natures (require two base natures)
    Wood        = 1 << 5    UMETA(DisplayName = "Wood Release"),
    Ice         = 1 << 6    UMETA(DisplayName = "Ice Release"),
    Lava        = 1 << 7    UMETA(DisplayName = "Lava Release"),
};
ENUM_CLASS_FLAGS(EChakraNature);

// ============================================================
//  Jutsu Classification
// ============================================================

UENUM(BlueprintType)
enum class EJutsuType : uint8
{
    Ninjutsu        UMETA(DisplayName = "Ninjutsu"),
    Taijutsu        UMETA(DisplayName = "Taijutsu"),
    Genjutsu        UMETA(DisplayName = "Genjutsu"),
    Medical         UMETA(DisplayName = "Medical Ninjutsu"),
    Summoning       UMETA(DisplayName = "Summoning Jutsu"),
    SageArts        UMETA(DisplayName = "Sage Arts"),
    ForbiddenJutsu  UMETA(DisplayName = "Forbidden Jutsu"),
    Kekkei_Genkai   UMETA(DisplayName = "Kekkei Genkai"),
    Kekkei_Mora     UMETA(DisplayName = "Kekkei Mora"),
    Dojutsu         UMETA(DisplayName = "Dojutsu"),
};

UENUM(BlueprintType)
enum class EJutsuRank : uint8
{
    E   UMETA(DisplayName = "E-Rank"),
    D   UMETA(DisplayName = "D-Rank"),
    C   UMETA(DisplayName = "C-Rank"),
    B   UMETA(DisplayName = "B-Rank"),
    A   UMETA(DisplayName = "A-Rank"),
    S   UMETA(DisplayName = "S-Rank"),
    Kinjutsu UMETA(DisplayName = "Kinjutsu"),
};

// ============================================================
//  Combat State
// ============================================================

UENUM(BlueprintType)
enum class ECombatStance : uint8
{
    Neutral         UMETA(DisplayName = "Neutral"),
    Aggressive      UMETA(DisplayName = "Aggressive"),
    Defensive       UMETA(DisplayName = "Defensive"),
    Evasive         UMETA(DisplayName = "Evasive"),
    Charging        UMETA(DisplayName = "Charging"),
    Awakened        UMETA(DisplayName = "Awakened"),
    SageMode        UMETA(DisplayName = "Sage Mode"),
    BijuuMode       UMETA(DisplayName = "Bijuu Mode"),
};

UENUM(BlueprintType)
enum class ECombatPhase : uint8
{
    Startup         UMETA(DisplayName = "Startup"),
    Active          UMETA(DisplayName = "Active"),
    Recovery        UMETA(DisplayName = "Recovery"),
    Hitstun         UMETA(DisplayName = "Hitstun"),
    Blockstun       UMETA(DisplayName = "Blockstun"),
    Invulnerable    UMETA(DisplayName = "Invulnerable"),
    Armored         UMETA(DisplayName = "Super Armor"),
};

UENUM(BlueprintType, meta = (Bitflags, UseEnumValuesAsMaskValuesInEditor = "true"))
enum class ECombatFlags : uint32
{
    None                = 0,
    CanCombo            = 1 << 0,
    CanCancel           = 1 << 1,
    CanSubstitute       = 1 << 2,
    CanParry            = 1 << 3,
    CanBlock            = 1 << 4,
    IsAirborne          = 1 << 5,
    IsLaunched          = 1 << 6,
    IsKnockdown         = 1 << 7,
    HasSuperArmor       = 1 << 8,
    IsInvulnerable      = 1 << 9,
    IsGuardBroken       = 1 << 10,
    IsCounterState      = 1 << 11,
    IsUltimateActive    = 1 << 12,
    IsAwakened          = 1 << 13,
    IsTransformed       = 1 << 14,
    IsStaggered         = 1 << 15,
};
ENUM_CLASS_FLAGS(ECombatFlags);

// ============================================================
//  Damage Types
// ============================================================

UENUM(BlueprintType)
enum class EDamageType : uint8
{
    Physical        UMETA(DisplayName = "Physical"),
    Chakra          UMETA(DisplayName = "Chakra"),
    Fire            UMETA(DisplayName = "Fire"),
    Water           UMETA(DisplayName = "Water"),
    Earth           UMETA(DisplayName = "Earth"),
    Wind            UMETA(DisplayName = "Wind"),
    Lightning       UMETA(DisplayName = "Lightning"),
    Wood            UMETA(DisplayName = "Wood"),
    Ice             UMETA(DisplayName = "Ice"),
    Lava            UMETA(DisplayName = "Lava"),
    Poison          UMETA(DisplayName = "Poison"),
    Genjutsu        UMETA(DisplayName = "Genjutsu"),
    True            UMETA(DisplayName = "True Damage"),  // Bypasses all mitigation
};

// ============================================================
//  Character Rank / Shinobi Grade
// ============================================================

UENUM(BlueprintType)
enum class EShinobi_Rank : uint8
{
    Academy_Student UMETA(DisplayName = "Academy Student"),
    Genin           UMETA(DisplayName = "Genin"),
    Chunin          UMETA(DisplayName = "Chunin"),
    Special_Jonin   UMETA(DisplayName = "Special Jonin"),
    Jonin           UMETA(DisplayName = "Jonin"),
    ANBU            UMETA(DisplayName = "ANBU"),
    Kage            UMETA(DisplayName = "Kage"),
    Legendary       UMETA(DisplayName = "Legendary Shinobi"),
    Otsutsuki       UMETA(DisplayName = "Otsutsuki"),
};

// ============================================================
//  Village / Faction IDs
// ============================================================

UENUM(BlueprintType)
enum class EVillage : uint8
{
    None            UMETA(DisplayName = "None / Rogue"),
    Konohagakure    UMETA(DisplayName = "Hidden Leaf"),
    Sunagakure      UMETA(DisplayName = "Hidden Sand"),
    Kirigakure      UMETA(DisplayName = "Hidden Mist"),
    Kumogakure      UMETA(DisplayName = "Hidden Cloud"),
    Iwagakure       UMETA(DisplayName = "Hidden Stone"),
    Otogakure       UMETA(DisplayName = "Hidden Sound"),
    Amegakure       UMETA(DisplayName = "Hidden Rain"),
    Akatsuki        UMETA(DisplayName = "Akatsuki"),
    Otsutsuki       UMETA(DisplayName = "Otsutsuki Clan"),
};

// ============================================================
//  Reputation Tier
// ============================================================

UENUM(BlueprintType)
enum class EReputationTier : uint8
{
    Nemesis     UMETA(DisplayName = "Nemesis"),
    Hostile     UMETA(DisplayName = "Hostile"),
    Unfriendly  UMETA(DisplayName = "Unfriendly"),
    Neutral     UMETA(DisplayName = "Neutral"),
    Friendly    UMETA(DisplayName = "Friendly"),
    Honored     UMETA(DisplayName = "Honored"),
    Revered     UMETA(DisplayName = "Revered"),
    Legendary   UMETA(DisplayName = "Legendary"),
};

// ============================================================
//  Quest Types
// ============================================================

UENUM(BlueprintType)
enum class EQuestType : uint8
{
    MainStory       UMETA(DisplayName = "Main Story"),
    SideStory       UMETA(DisplayName = "Side Story"),
    CharacterStory  UMETA(DisplayName = "Character Story"),
    VillageStory    UMETA(DisplayName = "Village Story"),
    FactionStory    UMETA(DisplayName = "Faction Story"),
    LegendaryHunt   UMETA(DisplayName = "Legendary Hunt"),
    WorldEvent      UMETA(DisplayName = "World Event"),
    DailyMission    UMETA(DisplayName = "Daily Mission"),
    BountyMission   UMETA(DisplayName = "Bounty Mission"),
};

UENUM(BlueprintType)
enum class EQuestState : uint8
{
    Locked          UMETA(DisplayName = "Locked"),
    Available       UMETA(DisplayName = "Available"),
    Active          UMETA(DisplayName = "Active"),
    Completed       UMETA(DisplayName = "Completed"),
    Failed          UMETA(DisplayName = "Failed"),
    Abandoned       UMETA(DisplayName = "Abandoned"),
};

// ============================================================
//  World / Environment
// ============================================================

UENUM(BlueprintType)
enum class EWeatherType : uint8
{
    Clear           UMETA(DisplayName = "Clear"),
    Cloudy          UMETA(DisplayName = "Cloudy"),
    Overcast        UMETA(DisplayName = "Overcast"),
    LightRain       UMETA(DisplayName = "Light Rain"),
    HeavyRain       UMETA(DisplayName = "Heavy Rain"),
    Thunderstorm    UMETA(DisplayName = "Thunderstorm"),
    Fog             UMETA(DisplayName = "Fog"),
    Snow            UMETA(DisplayName = "Snow"),
    Blizzard        UMETA(DisplayName = "Blizzard"),
    Sandstorm       UMETA(DisplayName = "Sandstorm"),
    ChakraStorm     UMETA(DisplayName = "Chakra Storm"),  // Supernatural weather
};

UENUM(BlueprintType)
enum class ESeason : uint8
{
    Spring  UMETA(DisplayName = "Spring"),
    Summer  UMETA(DisplayName = "Summer"),
    Autumn  UMETA(DisplayName = "Autumn"),
    Winter  UMETA(DisplayName = "Winter"),
};

// ============================================================
//  Stat Identifiers (used by data-driven stat system)
// ============================================================

UENUM(BlueprintType)
enum class ECharacterStat : uint8
{
    MaxHealth           UMETA(DisplayName = "Max Health"),
    MaxChakra           UMETA(DisplayName = "Max Chakra"),
    MaxStamina          UMETA(DisplayName = "Max Stamina"),
    PhysicalAttack      UMETA(DisplayName = "Physical Attack"),
    ChakraAttack        UMETA(DisplayName = "Chakra Attack"),
    PhysicalDefense     UMETA(DisplayName = "Physical Defense"),
    ChakraDefense       UMETA(DisplayName = "Chakra Defense"),
    Speed               UMETA(DisplayName = "Speed"),
    ChakraRegen         UMETA(DisplayName = "Chakra Regen"),
    HealthRegen         UMETA(DisplayName = "Health Regen"),
    CriticalChance      UMETA(DisplayName = "Critical Chance"),
    CriticalMultiplier  UMETA(DisplayName = "Critical Multiplier"),
    ComboWindow         UMETA(DisplayName = "Combo Window"),
    JutsuCooldownReduction UMETA(DisplayName = "Jutsu Cooldown Reduction"),
    SageChakraGain      UMETA(DisplayName = "Sage Chakra Gain"),
};

// ============================================================
//  Structs
// ============================================================

/** Represents a single stat modifier from equipment, buffs, or skills. */
USTRUCT(BlueprintType)
struct NARUTOUSL_API FStatModifier
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stat")
    ECharacterStat StatType = ECharacterStat::MaxHealth;

    /** Flat additive bonus applied before multipliers. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stat")
    float FlatBonus = 0.0f;

    /** Multiplicative bonus (1.0 = no change, 1.1 = +10%). */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stat")
    float Multiplier = 1.0f;

    /** Priority order for stacking resolution. Higher = applied later. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stat")
    int32 Priority = 0;

    /** Source tag for identification and removal. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stat")
    FGameplayTag SourceTag;

    /** Duration in seconds. -1 = permanent. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stat")
    float Duration = -1.0f;

    FStatModifier() = default;

    FStatModifier(ECharacterStat InStat, float InFlat, float InMult, FGameplayTag InSource, float InDuration = -1.0f)
        : StatType(InStat), FlatBonus(InFlat), Multiplier(InMult), SourceTag(InSource), Duration(InDuration)
    {}
};

/** Resolved stat value after all modifiers are applied. */
USTRUCT(BlueprintType)
struct NARUTOUSL_API FResolvedStat
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Stat")
    float BaseValue = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Stat")
    float FinalValue = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Stat")
    float TotalFlatBonus = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Stat")
    float TotalMultiplier = 1.0f;
};

/** Frame data for a single attack or action. */
USTRUCT(BlueprintType)
struct NARUTOUSL_API FAttackFrameData
{
    GENERATED_BODY()

    /** Frames before the hitbox becomes active. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Frame Data")
    int32 StartupFrames = 5;

    /** Frames the hitbox is active. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Frame Data")
    int32 ActiveFrames = 3;

    /** Frames after the active window before the character can act again. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Frame Data")
    int32 RecoveryFrames = 12;

    /** Frames the opponent is in hitstun on hit. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Frame Data")
    int32 HitstunFrames = 15;

    /** Frames the opponent is in blockstun on block. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Frame Data")
    int32 BlockstunFrames = 8;

    /** Frame advantage on hit (positive = attacker advantage). */
    UPROPERTY(BlueprintReadOnly, Category = "Frame Data")
    int32 FrameAdvantageOnHit = 0;

    /** Frame advantage on block. */
    UPROPERTY(BlueprintReadOnly, Category = "Frame Data")
    int32 FrameAdvantageOnBlock = 0;

    /** Invulnerability window start frame (0 = none). */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Frame Data")
    int32 InvulnerabilityStart = 0;

    /** Invulnerability window end frame. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Frame Data")
    int32 InvulnerabilityEnd = 0;

    /** Armor frames (absorb hits without interruption). */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Frame Data")
    int32 ArmorStart = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Frame Data")
    int32 ArmorEnd = 0;

    /** Cancel window: frame at which the player can cancel into another action. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Frame Data")
    int32 CancelWindowStart = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Frame Data")
    int32 CancelWindowEnd = 0;

    /** Tags that describe what this attack can cancel into. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Frame Data")
    FGameplayTagContainer CancelTargetTags;

    /** Priority value for clash resolution. Higher wins. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Frame Data")
    int32 Priority = 0;

    /** Combo tag used to chain into the next attack in a sequence. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Frame Data")
    FGameplayTag ComboTag;

    int32 TotalFrames() const { return StartupFrames + ActiveFrames + RecoveryFrames; }

    bool IsInStartup(int32 Frame) const { return Frame < StartupFrames; }
    bool IsActive(int32 Frame) const { return Frame >= StartupFrames && Frame < StartupFrames + ActiveFrames; }
    bool IsInRecovery(int32 Frame) const { return Frame >= StartupFrames + ActiveFrames; }
    bool IsInvulnerable(int32 Frame) const { return InvulnerabilityStart > 0 && Frame >= InvulnerabilityStart && Frame <= InvulnerabilityEnd; }
    bool HasArmor(int32 Frame) const { return ArmorStart > 0 && Frame >= ArmorStart && Frame <= ArmorEnd; }
    bool IsInCancelWindow(int32 Frame) const { return CancelWindowStart > 0 && Frame >= CancelWindowStart && Frame <= CancelWindowEnd; }
};

/** Hitbox definition for a single attack. */
USTRUCT(BlueprintType)
struct NARUTOUSL_API FHitboxData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hitbox")
    FName HitboxID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hitbox")
    FVector LocalOffset = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hitbox")
    FVector HalfExtent = FVector(50.0f, 50.0f, 50.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hitbox")
    float SphereRadius = 0.0f;  // If > 0, use sphere instead of box

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hitbox")
    FName AttachBone = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hitbox")
    EDamageType DamageType = EDamageType::Physical;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hitbox")
    float DamageMultiplier = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hitbox")
    FVector KnockbackDirection = FVector(1.0f, 0.0f, 0.3f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hitbox")
    float KnockbackForce = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hitbox")
    bool bLaunches = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hitbox")
    bool bGroundBounce = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hitbox")
    bool bWallBounce = false;

    /** Tags applied to the target on hit (e.g., Burning, Paralyzed). */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hitbox")
    FGameplayTagContainer AppliedStatusTags;
};

/** Damage event passed through the damage pipeline. */
USTRUCT(BlueprintType)
struct NARUTOUSL_API FNarutoDamageEvent
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Damage")
    float BaseDamage = 0.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Damage")
    float FinalDamage = 0.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Damage")
    EDamageType DamageType = EDamageType::Physical;

    UPROPERTY(BlueprintReadWrite, Category = "Damage")
    bool bIsCritical = false;

    UPROPERTY(BlueprintReadWrite, Category = "Damage")
    bool bIsBlocked = false;

    UPROPERTY(BlueprintReadWrite, Category = "Damage")
    bool bIsParried = false;

    UPROPERTY(BlueprintReadWrite, Category = "Damage")
    FVector HitLocation = FVector::ZeroVector;

    UPROPERTY(BlueprintReadWrite, Category = "Damage")
    FVector HitNormal = FVector::ZeroVector;

    UPROPERTY(BlueprintReadWrite, Category = "Damage")
    TWeakObjectPtr<AActor> Instigator = nullptr;

    UPROPERTY(BlueprintReadWrite, Category = "Damage")
    TWeakObjectPtr<AActor> Target = nullptr;

    UPROPERTY(BlueprintReadWrite, Category = "Damage")
    FGameplayTag SourceJutsuTag;

    UPROPERTY(BlueprintReadWrite, Category = "Damage")
    FGameplayTagContainer AppliedStatusTags;

    UPROPERTY(BlueprintReadWrite, Category = "Damage")
    FHitboxData HitboxData;
};

/** Chakra cost definition for a jutsu or ability. */
USTRUCT(BlueprintType)
struct NARUTOUSL_API FChakraCost
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chakra")
    float FlatCost = 0.0f;

    /** Percentage of max chakra consumed. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chakra")
    float PercentCost = 0.0f;

    /** Required nature affinity to cast. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chakra")
    EChakraNature RequiredNature = EChakraNature::None;

    /** Minimum chakra level required (0-100 scale). */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chakra")
    float MinimumChakraRequired = 0.0f;

    /** If true, cost scales with mastery level. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chakra")
    bool bScalesWithMastery = false;

    /** Mastery cost reduction curve (mastery level -> cost multiplier). */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chakra")
    UCurveFloat* MasteryCostCurve = nullptr;
};

/** Reputation entry for a single faction. */
USTRUCT(BlueprintType)
struct NARUTOUSL_API FFactionReputation
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Faction")
    EVillage Faction = EVillage::None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Faction")
    float ReputationPoints = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Faction")
    EReputationTier Tier = EReputationTier::Neutral;

    static EReputationTier CalculateTier(float Points)
    {
        if (Points <= -5000.0f) return EReputationTier::Nemesis;
        if (Points <= -2500.0f) return EReputationTier::Hostile;
        if (Points <= -500.0f)  return EReputationTier::Unfriendly;
        if (Points <   500.0f)  return EReputationTier::Neutral;
        if (Points <  2500.0f)  return EReputationTier::Friendly;
        if (Points <  5000.0f)  return EReputationTier::Honored;
        if (Points <  9000.0f)  return EReputationTier::Revered;
        return EReputationTier::Legendary;
    }
};
