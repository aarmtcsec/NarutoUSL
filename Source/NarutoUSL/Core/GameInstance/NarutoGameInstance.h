// Copyright (c) 2025 Maze Runner Studios. All Rights Reserved.
// Naruto: Ultimate Shinobi Legacy
// NarutoGameInstance — Root game instance. Owns all persistent subsystems
// and survives level transitions, streaming, and save/load cycles.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "Core/Types/NarutoTypes.h"
#include "NarutoGameInstance.generated.h"

// Forward declarations — never include subsystem headers here to avoid
// circular dependencies. Use forward decls + accessors only.
class UNarutoEventBus;
class USaveManager;
class UChakraSystem;
class UCombatManager;
class UJutsuManager;
class UFactionManager;
class UQuestManager;
class UNarrativeManager;
class UWorldStateManager;
class UEconomyManager;
class UProgressionManager;
class UAudioManager;
class UUIManager;
class UAnalyticsManager;
class UStreamingManager;

// ============================================================
//  Delegates
// ============================================================

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnGameInitialized);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSaveLoaded, int32, SlotIndex);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSaveFailed, FText, ErrorMessage);

// ============================================================
//  NarutoGameInstance
// ============================================================

/**
 * UNarutoGameInstance
 *
 * Single point of truth for all persistent game state. Subsystems are
 * created here and accessed via typed getters. No subsystem should hold
 * a raw pointer to another — they communicate through the EventBus or
 * via the GameInstance accessor pattern.
 *
 * Initialization order:
 *   1. EventBus (no dependencies)
 *   2. WorldStateManager (depends on EventBus)
 *   3. FactionManager (depends on EventBus, WorldStateManager)
 *   4. QuestManager (depends on EventBus, WorldStateManager, FactionManager)
 *   5. NarrativeManager (depends on QuestManager, WorldStateManager)
 *   6. ChakraSystem (depends on EventBus)
 *   7. CombatManager (depends on ChakraSystem, EventBus)
 *   8. JutsuManager (depends on ChakraSystem, CombatManager)
 *   9. ProgressionManager (depends on EventBus, FactionManager)
 *  10. EconomyManager (depends on FactionManager, EventBus)
 *  11. AudioManager (depends on EventBus)
 *  12. UIManager (depends on EventBus)
 *  13. AnalyticsManager (depends on EventBus)
 *  14. StreamingManager (depends on WorldStateManager)
 *  15. SaveManager (depends on all above — serializes their state)
 */
UCLASS(Config = Game)
class NARUTOUSL_API UNarutoGameInstance : public UGameInstance
{
    GENERATED_BODY()

public:

    UNarutoGameInstance(const FObjectInitializer& ObjectInitializer);

    // ------------------------------------------------------------------
    //  UGameInstance Interface
    // ------------------------------------------------------------------

    virtual void Init() override;
    virtual void Shutdown() override;
    virtual void OnStart() override;
    virtual void OnWorldChanged(UWorld* OldWorld, UWorld* NewWorld) override;

    // ------------------------------------------------------------------
    //  Subsystem Accessors
    // ------------------------------------------------------------------

    UFUNCTION(BlueprintPure, Category = "NarutoUSL|Subsystems")
    UNarutoEventBus* GetEventBus() const { return EventBus; }

    UFUNCTION(BlueprintPure, Category = "NarutoUSL|Subsystems")
    USaveManager* GetSaveManager() const { return SaveManager; }

    UFUNCTION(BlueprintPure, Category = "NarutoUSL|Subsystems")
    UChakraSystem* GetChakraSystem() const { return ChakraSystem; }

    UFUNCTION(BlueprintPure, Category = "NarutoUSL|Subsystems")
    UCombatManager* GetCombatManager() const { return CombatManager; }

    UFUNCTION(BlueprintPure, Category = "NarutoUSL|Subsystems")
    UJutsuManager* GetJutsuManager() const { return JutsuManager; }

    UFUNCTION(BlueprintPure, Category = "NarutoUSL|Subsystems")
    UFactionManager* GetFactionManager() const { return FactionManager; }

    UFUNCTION(BlueprintPure, Category = "NarutoUSL|Subsystems")
    UQuestManager* GetQuestManager() const { return QuestManager; }

    UFUNCTION(BlueprintPure, Category = "NarutoUSL|Subsystems")
    UNarrativeManager* GetNarrativeManager() const { return NarrativeManager; }

    UFUNCTION(BlueprintPure, Category = "NarutoUSL|Subsystems")
    UWorldStateManager* GetWorldStateManager() const { return WorldStateManager; }

    UFUNCTION(BlueprintPure, Category = "NarutoUSL|Subsystems")
    UEconomyManager* GetEconomyManager() const { return EconomyManager; }

    UFUNCTION(BlueprintPure, Category = "NarutoUSL|Subsystems")
    UProgressionManager* GetProgressionManager() const { return ProgressionManager; }

    UFUNCTION(BlueprintPure, Category = "NarutoUSL|Subsystems")
    UAudioManager* GetAudioManager() const { return AudioManager; }

    UFUNCTION(BlueprintPure, Category = "NarutoUSL|Subsystems")
    UUIManager* GetUIManager() const { return UIManager; }

    UFUNCTION(BlueprintPure, Category = "NarutoUSL|Subsystems")
    UAnalyticsManager* GetAnalyticsManager() const { return AnalyticsManager; }

    UFUNCTION(BlueprintPure, Category = "NarutoUSL|Subsystems")
    UStreamingManager* GetStreamingManager() const { return StreamingManager; }

    // ------------------------------------------------------------------
    //  Static Convenience Accessor
    // ------------------------------------------------------------------

    /**
     * Returns the NarutoGameInstance from any world context object.
     * Asserts in debug builds if the cast fails.
     */
    UFUNCTION(BlueprintPure, Category = "NarutoUSL", meta = (WorldContext = "WorldContextObject"))
    static UNarutoGameInstance* Get(const UObject* WorldContextObject);

    // ------------------------------------------------------------------
    //  Save / Load API (delegates to SaveManager internally)
    // ------------------------------------------------------------------

    /**
     * Loads a save slot and restores all subsystem state.
     * Async — result delivered via OnSaveLoaded / OnSaveFailed delegates.
     */
    UFUNCTION(BlueprintCallable, Category = "NarutoUSL|Save")
    void LoadGame(int32 SlotIndex);

    /**
     * Saves all subsystem state to the specified slot.
     * @param SlotIndex  0-9 for manual slots, -1 for autosave.
     */
    UFUNCTION(BlueprintCallable, Category = "NarutoUSL|Save")
    void SaveGame(int32 SlotIndex = -1);

    /** Returns true if a valid save exists in the specified slot. */
    UFUNCTION(BlueprintPure, Category = "NarutoUSL|Save")
    bool HasSaveInSlot(int32 SlotIndex) const;

    // ------------------------------------------------------------------
    //  Game State Queries
    // ------------------------------------------------------------------

    UFUNCTION(BlueprintPure, Category = "NarutoUSL|State")
    bool IsGameInitialized() const { return bGameInitialized; }

    UFUNCTION(BlueprintPure, Category = "NarutoUSL|State")
    bool IsInCombat() const { return bIsInCombat; }

    UFUNCTION(BlueprintPure, Category = "NarutoUSL|State")
    bool IsInCinematic() const { return bIsInCinematic; }

    UFUNCTION(BlueprintCallable, Category = "NarutoUSL|State")
    void SetCinematicState(bool bActive);

    // ------------------------------------------------------------------
    //  Events
    // ------------------------------------------------------------------

    UPROPERTY(BlueprintAssignable, Category = "NarutoUSL|Events")
    FOnGameInitialized OnGameInitialized;

    UPROPERTY(BlueprintAssignable, Category = "NarutoUSL|Events")
    FOnSaveLoaded OnSaveLoaded;

    UPROPERTY(BlueprintAssignable, Category = "NarutoUSL|Events")
    FOnSaveFailed OnSaveFailed;

protected:

    // ------------------------------------------------------------------
    //  Initialization Helpers
    // ------------------------------------------------------------------

    void CreateSubsystems();
    void InitializeSubsystemsInOrder();
    void BindCrossSubsystemEvents();
    void ShutdownSubsystems();

    // ------------------------------------------------------------------
    //  Event Handlers
    // ------------------------------------------------------------------

    UFUNCTION()
    void HandleCombatEncounterChanged(const FCombatEncounterEvent& Event);

private:

    // ------------------------------------------------------------------
    //  Subsystem Instances (owned, GC-rooted via UPROPERTY)
    // ------------------------------------------------------------------

    UPROPERTY()
    TObjectPtr<UNarutoEventBus> EventBus;

    UPROPERTY()
    TObjectPtr<UWorldStateManager> WorldStateManager;

    UPROPERTY()
    TObjectPtr<UFactionManager> FactionManager;

    UPROPERTY()
    TObjectPtr<UQuestManager> QuestManager;

    UPROPERTY()
    TObjectPtr<UNarrativeManager> NarrativeManager;

    UPROPERTY()
    TObjectPtr<UChakraSystem> ChakraSystem;

    UPROPERTY()
    TObjectPtr<UCombatManager> CombatManager;

    UPROPERTY()
    TObjectPtr<UJutsuManager> JutsuManager;

    UPROPERTY()
    TObjectPtr<UProgressionManager> ProgressionManager;

    UPROPERTY()
    TObjectPtr<UEconomyManager> EconomyManager;

    UPROPERTY()
    TObjectPtr<UAudioManager> AudioManager;

    UPROPERTY()
    TObjectPtr<UUIManager> UIManager;

    UPROPERTY()
    TObjectPtr<UAnalyticsManager> AnalyticsManager;

    UPROPERTY()
    TObjectPtr<UStreamingManager> StreamingManager;

    UPROPERTY()
    TObjectPtr<USaveManager> SaveManager;

    // ------------------------------------------------------------------
    //  State Flags
    // ------------------------------------------------------------------

    bool bGameInitialized = false;
    bool bIsInCombat      = false;
    bool bIsInCinematic   = false;
};
