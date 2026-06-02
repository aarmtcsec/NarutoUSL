// Copyright (c) 2025 Maze Runner Studios. All Rights Reserved.
// Naruto: Ultimate Shinobi Legacy
// JutsuData — Primary data asset for every jutsu in the game.
// One asset per jutsu. Drives execution, cost, visuals, audio, and AI.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "Core/Types/NarutoTypes.h"
#include "JutsuData.generated.h"

class UNiagaraSystem;
class USoundBase;
class UAnimMontage;
class UCurveFloat;

// ============================================================
//  Hand Seal Sequence
// ============================================================

UENUM(BlueprintType)
enum class EHandSeal : uint8
{
    Ne      UMETA(DisplayName = "Ne (Rat)"),
    Ushi    UMETA(DisplayName = "Ushi (Ox)"),
    Tora    UMETA(DisplayName = "Tora (Tiger)"),
    U       UMETA(DisplayName = "U (Hare)"),
    Tatsu   UMETA(DisplayName = "Tatsu (Dragon)"),
    Mi      UMETA(DisplayName = "Mi (Snake)"),
    Uma     UMETA(DisplayName = "Uma (Horse)"),
    Hitsuji UMETA(DisplayName = "Hitsuji (Ram)"),
    Saru    UMETA(DisplayName = "Saru (Monkey)"),
    Tori    UMETA(DisplayName = "Tori (Bird)"),
    Inu     UMETA(DisplayName = "Inu (Dog)"),
    I       UMETA(DisplayName = "I (Boar)"),
};

// ============================================================
//  Jutsu Targeting
// ============================================================

UENUM(BlueprintType)
enum class EJutsuTargetingMode : uint8
{
    Self            UMETA(DisplayName = "Self"),
    SingleTarget    UMETA(DisplayName = "Single Target"),
    MultiTarget     UMETA(DisplayName = "Multi Target"),
    AreaOfEffect    UMETA(DisplayName = "Area of Effect"),
    Directional     UMETA(DisplayName = "Directional"),
    Projectile      UMETA(DisplayName = "Projectile"),
    Persistent      UMETA(DisplayName = "Persistent Zone"),
    Summon          UMETA(DisplayName = "Summon"),
};

// ============================================================
//  Jutsu Execution Phase
// ============================================================

UENUM(BlueprintType)
enum class EJutsuExecutionPhase : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    HandSeals   UMETA(DisplayName = "Performing Hand Seals"),
    Charging    UMETA(DisplayName = "Charging"),
    Active      UMETA(DisplayName = "Active"),
    Sustaining  UMETA(DisplayName = "Sustaining"),
    Cooldown    UMETA(DisplayName = "Cooldown"),
    Interrupted UMETA(DisplayName = "Interrupted"),
};

// ============================================================
//  Scaling Definition
// ============================================================

/**
 * FJutsuScaling
 * Defines how a jutsu's damage/healing scales with character stats and mastery.
 */
USTRUCT(BlueprintType)
struct NARUTOUSL_API FJutsuScaling
{
    GENERATED_BODY()

    /** Base damage/healing before any scaling. */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Scaling", meta = (ClampMin = "0"))
    float BaseValue = 100.0f;

    /** Fraction of ChakraAttack added to base value. */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Scaling", meta = (ClampMin = "0"))
    float ChakraAttackScaling = 1.0f;

    /** Fraction of PhysicalAttack added to base value (for Taijutsu). */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Scaling", meta = (ClampMin = "0"))
    float PhysicalAttackScaling = 0.0f;

    /** Mastery level scaling curve. X = mastery level (0-10), Y = damage multiplier. */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Scaling")
    TSoftObjectPtr<UCurveFloat> MasteryScalingCurve;

    /** Whether this jutsu can critically hit. */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Scaling")
    bool bCanCrit = true;

    /** Critical hit multiplier override. 0 = use character's default. */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Scaling", meta = (ClampMin = "0"))
    float CritMultiplierOverride = 0.0f;
};

// ============================================================
//  Status Effect Application
// ============================================================

USTRUCT(BlueprintType)
struct NARUTOUSL_API FJutsuStatusEffect
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Status")
    FGameplayTag StatusTag;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Status", meta = (ClampMin = "0"))
    float Duration = 3.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Status", meta = (ClampMin = "0", ClampMax = "1"))
    float ApplicationChance = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Status", meta = (ClampMin = "0"))
    float DamagePerSecond = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Status")
    EDamageType DotDamageType = EDamageType::Fire;
};

// ============================================================
//  Projectile Definition
// ============================================================

USTRUCT(BlueprintType)
struct NARUTOUSL_API FJutsuProjectileData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Projectile")
    TSoftClassPtr<AActor> ProjectileClass;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Projectile", meta = (ClampMin = "100"))
    float Speed = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Projectile", meta = (ClampMin = "0"))
    float Lifetime = 5.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Projectile")
    bool bHomingEnabled = false;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Projectile",
        meta = (EditCondition = "bHomingEnabled", ClampMin = "0"))
    float HomingAcceleration = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Projectile")
    bool bPiercing = false;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Projectile", meta = (ClampMin = "1"))
    int32 MaxPierceTargets = 1;
};

// ============================================================
//  AoE Definition
// ============================================================

USTRUCT(BlueprintType)
struct NARUTOUSL_API FJutsuAoEData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AoE", meta = (ClampMin = "0"))
    float Radius = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AoE", meta = (ClampMin = "0"))
    float Height = 0.0f;  // 0 = sphere, > 0 = cylinder

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AoE", meta = (ClampMin = "0"))
    float Duration = 0.0f;  // 0 = instant, > 0 = persistent zone

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AoE", meta = (ClampMin = "0"))
    float TickInterval = 0.5f;  // For persistent zones

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AoE", meta = (ClampMin = "0"))
    int32 MaxTargets = 0;  // 0 = unlimited

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AoE")
    bool bFalloffEnabled = true;

    /** Damage at the edge of the AoE as a fraction of center damage. */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AoE",
        meta = (EditCondition = "bFalloffEnabled", ClampMin = "0", ClampMax = "1"))
    float EdgeDamageFraction = 0.5f;
};

// ============================================================
//  JutsuData
// ============================================================

/**
 * UJutsuData
 *
 * Primary data asset for a single jutsu. Contains everything needed
 * to execute, display, and balance the jutsu at runtime.
 * No execution logic lives here — only data.
 *
 * Naming convention: DA_Jutsu_Rasengan, DA_Jutsu_Chidori, etc.
 */
UCLASS(BlueprintType)
class NARUTOUSL_API UJutsuData : public UPrimaryDataAsset
{
    GENERATED_BODY()

public:

    virtual FPrimaryAssetId GetPrimaryAssetId() const override
    {
        return FPrimaryAssetId(TEXT("JutsuData"), GetFName());
    }

    // ------------------------------------------------------------------
    //  Identity
    // ------------------------------------------------------------------

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Identity")
    FText DisplayName;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Identity")
    FText Description;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Identity")
    FGameplayTag JutsuTag;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Identity")
    EJutsuType JutsuType = EJutsuType::Ninjutsu;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Identity")
    EJutsuRank Rank = EJutsuRank::C;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Identity")
    EDamageType DamageType = EDamageType::Chakra;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Identity")
    TSoftObjectPtr<UTexture2D> IconTexture;

    // ------------------------------------------------------------------
    //  Execution
    // ------------------------------------------------------------------

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Execution")
    EJutsuTargetingMode TargetingMode = EJutsuTargetingMode::SingleTarget;

    /** C++ executor class that handles this jutsu's logic. */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Execution")
    TSoftClassPtr<UObject> ExecutorClass;

    /** Hand seal sequence required to perform this jutsu. Empty = no seals. */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Execution")
    TArray<EHandSeal> HandSealSequence;

    /** Time in seconds to perform hand seals. 0 = instant. */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Execution", meta = (ClampMin = "0"))
    float HandSealDuration = 0.5f;

    /** Charge time before activation. 0 = no charge. */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Execution", meta = (ClampMin = "0"))
    float ChargeDuration = 0.0f;

    /** Whether this jutsu can be charged for increased power. */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Execution")
    bool bChargeable = false;

    /** Maximum charge multiplier (at full charge). */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Execution",
        meta = (EditCondition = "bChargeable", ClampMin = "1"))
    float MaxChargeMultiplier = 2.0f;

    /** Duration the jutsu remains active after activation. 0 = instant. */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Execution", meta = (ClampMin = "0"))
    float ActiveDuration = 0.0f;

    /** Whether this jutsu can be sustained (held active). */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Execution")
    bool bSustainable = false;

    /** Chakra cost per second while sustaining. */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Execution",
        meta = (EditCondition = "bSustainable", ClampMin = "0"))
    float SustainCostPerSecond = 5.0f;

    /** Whether this jutsu can be interrupted by hitstun. */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Execution")
    bool bInterruptible = true;

    /** Whether this jutsu grants super armor during execution. */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Execution")
    bool bGrantsArmorDuringCast = false;

    // ------------------------------------------------------------------
    //  Cost
    // ------------------------------------------------------------------

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Cost")
    FChakraCost ChakraCost;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Cost", meta = (ClampMin = "0"))
    float Cooldown = 5.0f;

    /** Cooldown reduction per mastery level (seconds). */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Cost", meta = (ClampMin = "0"))
    float CooldownReductionPerMastery = 0.2f;

    // ------------------------------------------------------------------
    //  Damage / Healing
    // ------------------------------------------------------------------

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Damage")
    FJutsuScaling Scaling;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Damage")
    TArray<FJutsuStatusEffect> StatusEffects;

    // ------------------------------------------------------------------
    //  Targeting
    // ------------------------------------------------------------------

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Targeting", meta = (ClampMin = "0"))
    float Range = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Targeting", meta = (ClampMin = "0"))
    int32 MaxTargets = 1;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Targeting")
    bool bRequiresLineOfSight = true;

    // ------------------------------------------------------------------
    //  Projectile (if applicable)
    // ------------------------------------------------------------------

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Projectile",
        meta = (EditCondition = "TargetingMode == EJutsuTargetingMode::Projectile"))
    FJutsuProjectileData ProjectileData;

    // ------------------------------------------------------------------
    //  AoE (if applicable)
    // ------------------------------------------------------------------

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AoE",
        meta = (EditCondition = "TargetingMode == EJutsuTargetingMode::AreaOfEffect || TargetingMode == EJutsuTargetingMode::Persistent"))
    FJutsuAoEData AoEData;

    // ------------------------------------------------------------------
    //  Visuals
    // ------------------------------------------------------------------

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Visuals")
    TSoftObjectPtr<UAnimMontage> CasterMontage;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Visuals")
    TSoftObjectPtr<UAnimMontage> HandSealMontage;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Visuals")
    TSoftObjectPtr<UNiagaraSystem> CastVFX;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Visuals")
    TSoftObjectPtr<UNiagaraSystem> ImpactVFX;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Visuals")
    TSoftObjectPtr<UNiagaraSystem> SustainVFX;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Visuals")
    FVector CastVFXOffset = FVector::ZeroVector;

    // ------------------------------------------------------------------
    //  Audio
    // ------------------------------------------------------------------

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Audio")
    TSoftObjectPtr<USoundBase> CastSound;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Audio")
    TSoftObjectPtr<USoundBase> ImpactSound;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Audio")
    TSoftObjectPtr<USoundBase> SustainLoop;

    // ------------------------------------------------------------------
    //  Camera
    // ------------------------------------------------------------------

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Camera")
    bool bTriggersCameraShake = false;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Camera",
        meta = (EditCondition = "bTriggersCameraShake"))
    TSoftClassPtr<UCameraShakeBase> CameraShakeClass;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Camera")
    bool bIsUltimateJutsu = false;

    /** Cinematic sequence played for ultimate jutsu. */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Camera",
        meta = (EditCondition = "bIsUltimateJutsu"))
    TSoftObjectPtr<ULevelSequence> UltimateCinematic;

    // ------------------------------------------------------------------
    //  Unlock / Progression
    // ------------------------------------------------------------------

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Unlock")
    bool bUnlockedByDefault = false;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Unlock",
        meta = (EditCondition = "!bUnlockedByDefault"))
    int32 UnlockLevel = 1;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Unlock",
        meta = (EditCondition = "!bUnlockedByDefault"))
    FGameplayTag UnlockSkillNodeTag;

    // ------------------------------------------------------------------
    //  AI Hints
    // ------------------------------------------------------------------

    /** AI utility score for using this jutsu (higher = preferred). */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI", meta = (ClampMin = "0", ClampMax = "100"))
    float AIUtilityScore = 50.0f;

    /** Minimum range at which AI will use this jutsu. */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI", meta = (ClampMin = "0"))
    float AIMinRange = 0.0f;

    /** Maximum range at which AI will use this jutsu. */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI", meta = (ClampMin = "0"))
    float AIMaxRange = 1500.0f;

    /** Tags that describe when AI should prefer this jutsu. */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI")
    FGameplayTagContainer AIPreferenceContextTags;

    // ------------------------------------------------------------------
    //  Helpers
    // ------------------------------------------------------------------

    /** Returns the effective cooldown after mastery reduction. */
    float GetEffectiveCooldown(int32 MasteryLevel) const
    {
        return FMath::Max(0.5f, Cooldown - (CooldownReductionPerMastery * MasteryLevel));
    }

    /** Returns true if this jutsu requires hand seals. */
    bool RequiresHandSeals() const { return HandSealSequence.Num() > 0; }

    /** Returns true if this jutsu is an AoE type. */
    bool IsAoE() const
    {
        return TargetingMode == EJutsuTargetingMode::AreaOfEffect ||
               TargetingMode == EJutsuTargetingMode::Persistent;
    }
};
