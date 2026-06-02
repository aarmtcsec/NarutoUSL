// Copyright (c) 2025 Maze Runner Studios. All Rights Reserved.
// Naruto: Ultimate Shinobi Legacy
// NarutoGameSettings — Developer settings asset for data-driven configuration.
// Accessible via Project Settings → Naruto USL in the editor.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "Core/Types/NarutoTypes.h"
#include "NarutoGameSettings.generated.h"

/**
 * UNarutoGameSettings
 *
 * Centralized configuration for all tunable game parameters.
 * Values are set in the editor and baked into the game config.
 * Runtime systems read from this asset — never hardcode magic numbers.
 *
 * Access pattern:
 *   const UNarutoGameSettings* Settings = GetDefault<UNarutoGameSettings>();
 */
UCLASS(Config = Game, DefaultConfig, meta = (DisplayName = "Naruto USL Settings"))
class NARUTOUSL_API UNarutoGameSettings : public UDeveloperSettings
{
    GENERATED_BODY()

public:

    UNarutoGameSettings();

    virtual FName GetCategoryName() const override { return TEXT("NarutoUSL"); }
    virtual FName GetSectionName()  const override { return TEXT("Game Settings"); }

    // ------------------------------------------------------------------
    //  Chakra
    // ------------------------------------------------------------------

    UPROPERTY(Config, EditAnywhere, Category = "Chakra", meta = (ClampMin = "0.0"))
    float BaseChakraRegenRate = 5.0f;

    UPROPERTY(Config, EditAnywhere, Category = "Chakra", meta = (ClampMin = "0.0"))
    float MeditationChakraRegenMultiplier = 4.0f;

    UPROPERTY(Config, EditAnywhere, Category = "Chakra", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float ChakraShareMaxPercent = 0.5f;

    UPROPERTY(Config, EditAnywhere, Category = "Chakra", meta = (ClampMin = "0.0"))
    float TransformationChakraDrainRate = 10.0f;

    // ------------------------------------------------------------------
    //  Combat
    // ------------------------------------------------------------------

    /** Target simulation rate in frames per second for frame data. */
    UPROPERTY(Config, EditAnywhere, Category = "Combat", meta = (ClampMin = "30", ClampMax = "120"))
    int32 CombatFrameRate = 60;

    UPROPERTY(Config, EditAnywhere, Category = "Combat", meta = (ClampMin = "0.0"))
    float SubstitutionCooldown = 8.0f;

    UPROPERTY(Config, EditAnywhere, Category = "Combat", meta = (ClampMin = "0"))
    int32 MaxSubstitutionsPerCombat = 4;

    UPROPERTY(Config, EditAnywhere, Category = "Combat", meta = (ClampMin = "0.0"))
    float PerfectGuardWindow = 0.15f;

    UPROPERTY(Config, EditAnywhere, Category = "Combat", meta = (ClampMin = "0.0"))
    float PerfectDodgeWindow = 0.12f;

    UPROPERTY(Config, EditAnywhere, Category = "Combat", meta = (ClampMin = "0.0"))
    float ParryWindow = 0.1f;

    UPROPERTY(Config, EditAnywhere, Category = "Combat", meta = (ClampMin = "1.0"))
    float CriticalHitMultiplier = 1.75f;

    // ------------------------------------------------------------------
    //  AI
    // ------------------------------------------------------------------

    UPROPERTY(Config, EditAnywhere, Category = "AI", meta = (ClampMin = "0"))
    int32 MaxActiveEnemies = 64;

    UPROPERTY(Config, EditAnywhere, Category = "AI", meta = (ClampMin = "0.0"))
    float AIPerceptionUpdateInterval = 0.1f;

    UPROPERTY(Config, EditAnywhere, Category = "AI", meta = (ClampMin = "0.0"))
    float AIThreatDecayRate = 2.0f;

    // ------------------------------------------------------------------
    //  World
    // ------------------------------------------------------------------

    /** Real-time seconds per in-game hour. */
    UPROPERTY(Config, EditAnywhere, Category = "World", meta = (ClampMin = "1.0"))
    float SecondsPerGameHour = 90.0f;

    UPROPERTY(Config, EditAnywhere, Category = "World")
    float WeatherTransitionDuration = 30.0f;

    // ------------------------------------------------------------------
    //  Progression
    // ------------------------------------------------------------------

    UPROPERTY(Config, EditAnywhere, Category = "Progression", meta = (ClampMin = "1"))
    int32 MaxCharacterLevel = 100;

    UPROPERTY(Config, EditAnywhere, Category = "Progression", meta = (ClampMin = "1"))
    int32 MaxJutsuMasteryLevel = 10;

    /** XP curve asset — maps level to required XP. */
    UPROPERTY(Config, EditAnywhere, Category = "Progression")
    TSoftObjectPtr<UCurveFloat> LevelXPCurve;

    // ------------------------------------------------------------------
    //  Save
    // ------------------------------------------------------------------

    UPROPERTY(Config, EditAnywhere, Category = "Save", meta = (ClampMin = "1", ClampMax = "10"))
    int32 MaxManualSaveSlots = 9;

    UPROPERTY(Config, EditAnywhere, Category = "Save", meta = (ClampMin = "30.0"))
    float AutosaveIntervalSeconds = 300.0f;

    UPROPERTY(Config, EditAnywhere, Category = "Save", meta = (ClampMin = "1", ClampMax = "5"))
    int32 MaxAutosaveSlots = 3;

    // ------------------------------------------------------------------
    //  Performance
    // ------------------------------------------------------------------

    UPROPERTY(Config, EditAnywhere, Category = "Performance", meta = (ClampMin = "0"))
    int32 MaxSimultaneousVFX = 256;

    UPROPERTY(Config, EditAnywhere, Category = "Performance", meta = (ClampMin = "0"))
    int32 MaxSimultaneousAudioSources = 64;
};
