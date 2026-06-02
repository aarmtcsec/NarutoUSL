// Copyright (c) 2025 Maze Runner Studios. All Rights Reserved.
// Naruto: Ultimate Shinobi Legacy
// NarutoEventBus — Decoupled event dispatch system.
//
// Systems publish events without knowing who listens.
// Listeners subscribe without knowing who publishes.
// All event dispatch is type-safe via templated delegates.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Core/Types/NarutoTypes.h"
#include "NarutoEventBus.generated.h"

// ============================================================
//  Event Payload Structs
// ============================================================

USTRUCT(BlueprintType)
struct NARUTOUSL_API FCharacterDefeatedEvent
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly) TWeakObjectPtr<AActor> DefeatedActor;
    UPROPERTY(BlueprintReadOnly) TWeakObjectPtr<AActor> KillerActor;
    UPROPERTY(BlueprintReadOnly) FNarutoDamageEvent KillingBlow;
    UPROPERTY(BlueprintReadOnly) FVector DeathLocation;
    UPROPERTY(BlueprintReadOnly) float Timestamp = 0.0f;
};

USTRUCT(BlueprintType)
struct NARUTOUSL_API FQuestStateChangedEvent
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly) FGameplayTag QuestTag;
    UPROPERTY(BlueprintReadOnly) EQuestState OldState = EQuestState::Locked;
    UPROPERTY(BlueprintReadOnly) EQuestState NewState = EQuestState::Locked;
    UPROPERTY(BlueprintReadOnly) float Timestamp = 0.0f;
};

USTRUCT(BlueprintType)
struct NARUTOUSL_API FReputationChangedEvent
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly) EVillage Faction = EVillage::None;
    UPROPERTY(BlueprintReadOnly) float OldValue = 0.0f;
    UPROPERTY(BlueprintReadOnly) float NewValue = 0.0f;
    UPROPERTY(BlueprintReadOnly) EReputationTier OldTier = EReputationTier::Neutral;
    UPROPERTY(BlueprintReadOnly) EReputationTier NewTier = EReputationTier::Neutral;
};

USTRUCT(BlueprintType)
struct NARUTOUSL_API FWorldStateChangedEvent
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly) FGameplayTag StateTag;
    UPROPERTY(BlueprintReadOnly) bool bNewValue = false;
    UPROPERTY(BlueprintReadOnly) float Timestamp = 0.0f;
};

USTRUCT(BlueprintType)
struct NARUTOUSL_API FCombatEncounterEvent
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly) TWeakObjectPtr<AActor> PlayerActor;
    UPROPERTY(BlueprintReadOnly) TArray<TWeakObjectPtr<AActor>> Enemies;
    UPROPERTY(BlueprintReadOnly) bool bCombatStarted = false;
    UPROPERTY(BlueprintReadOnly) FVector EncounterLocation;
};

USTRUCT(BlueprintType)
struct NARUTOUSL_API FJutsuExecutedEvent
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly) TWeakObjectPtr<AActor> Caster;
    UPROPERTY(BlueprintReadOnly) FGameplayTag JutsuTag;
    UPROPERTY(BlueprintReadOnly) EJutsuType JutsuType = EJutsuType::Ninjutsu;
    UPROPERTY(BlueprintReadOnly) float ChakraConsumed = 0.0f;
    UPROPERTY(BlueprintReadOnly) float Timestamp = 0.0f;
};

// ============================================================
//  Delegate Declarations
// ============================================================

DECLARE_MULTICAST_DELEGATE_OneParam(FOnCharacterDefeated,     const FCharacterDefeatedEvent&);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnQuestStateChanged,     const FQuestStateChangedEvent&);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnReputationChanged,     const FReputationChangedEvent&);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnWorldStateChanged,     const FWorldStateChangedEvent&);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnCombatEncounterChanged, const FCombatEncounterEvent&);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnJutsuExecuted,         const FJutsuExecutedEvent&);
DECLARE_MULTICAST_DELEGATE_TwoParams(FOnPlayerLevelUp,        int32 /*OldLevel*/, int32 /*NewLevel*/);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnWeatherChanged,        EWeatherType /*NewWeather*/);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnSeasonChanged,         ESeason /*NewSeason*/);

// ============================================================
//  Event Bus
// ============================================================

/**
 * UNarutoEventBus
 *
 * Singleton-style event bus accessed via the GameInstance subsystem.
 * All major game systems publish and subscribe through this bus to
 * maintain loose coupling between modules.
 *
 * Usage:
 *   // Subscribe:
 *   EventBus->OnCharacterDefeated.AddUObject(this, &UMySystem::HandleDefeated);
 *
 *   // Publish:
 *   FCharacterDefeatedEvent Evt;
 *   Evt.DefeatedActor = DefeatedActor;
 *   EventBus->OnCharacterDefeated.Broadcast(Evt);
 */
UCLASS(NotBlueprintable)
class NARUTOUSL_API UNarutoEventBus : public UObject
{
    GENERATED_BODY()

public:

    // ------------------------------------------------------------------
    //  Combat Events
    // ------------------------------------------------------------------

    /** Fired when any character (player or NPC) is defeated. */
    FOnCharacterDefeated OnCharacterDefeated;

    /** Fired when a combat encounter starts or ends. */
    FOnCombatEncounterChanged OnCombatEncounterChanged;

    /** Fired when any jutsu is successfully executed. */
    FOnJutsuExecuted OnJutsuExecuted;

    // ------------------------------------------------------------------
    //  Narrative Events
    // ------------------------------------------------------------------

    /** Fired when any quest changes state. */
    FOnQuestStateChanged OnQuestStateChanged;

    /** Fired when the player's reputation with a faction changes. */
    FOnReputationChanged OnReputationChanged;

    /** Fired when a world state flag is set or cleared. */
    FOnWorldStateChanged OnWorldStateChanged;

    // ------------------------------------------------------------------
    //  Progression Events
    // ------------------------------------------------------------------

    /** Fired when the player character levels up. */
    FOnPlayerLevelUp OnPlayerLevelUp;

    // ------------------------------------------------------------------
    //  World Events
    // ------------------------------------------------------------------

    /** Fired when the weather transitions to a new type. */
    FOnWeatherChanged OnWeatherChanged;

    /** Fired when the season changes. */
    FOnSeasonChanged OnSeasonChanged;

    // ------------------------------------------------------------------
    //  Lifecycle
    // ------------------------------------------------------------------

    /** Clears all delegate bindings. Called on world teardown. */
    void ClearAllBindings();
};
