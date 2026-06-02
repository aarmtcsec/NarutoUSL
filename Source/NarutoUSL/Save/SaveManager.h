// Copyright (c) 2025 Maze Runner Studios. All Rights Reserved.
// Naruto: Ultimate Shinobi Legacy
// SaveManager — Versioned, async save/load with cloud sync and migration.

#pragma once

#include "CoreMinimal.h"
#include "Core/Subsystems/NarutoSubsystem.h"
#include "Core/Types/NarutoTypes.h"
#include "GameFramework/SaveGame.h"
#include "SaveManager.generated.h"

class UNarutoGameInstance;

// ============================================================
//  Save Slot Metadata
// ============================================================

USTRUCT(BlueprintType)
struct NARUTOUSL_API FSaveSlotMetadata
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly) int32   SlotIndex       = -1;
    UPROPERTY(BlueprintReadOnly) bool    bHasSave        = false;
    UPROPERTY(BlueprintReadOnly) FString CharacterName;
    UPROPERTY(BlueprintReadOnly) int32   CharacterLevel  = 0;
    UPROPERTY(BlueprintReadOnly) float   PlaytimeSeconds = 0.0f;
    UPROPERTY(BlueprintReadOnly) FString LocationName;
    UPROPERTY(BlueprintReadOnly) FDateTime SaveTimestamp;
    UPROPERTY(BlueprintReadOnly) ENarutoSaveVersion Version = ENarutoSaveVersion::Current;
    UPROPERTY(BlueprintReadOnly) TSoftObjectPtr<UTexture2D> Screenshot;
};

// ============================================================
//  Save Data
// ============================================================

UCLASS()
class NARUTOUSL_API UNarutoSaveGame : public USaveGame
{
    GENERATED_BODY()

public:

    UPROPERTY() ENarutoSaveVersion Version = ENarutoSaveVersion::Current;
    UPROPERTY() FSaveSlotMetadata  Metadata;

    // Player state
    UPROPERTY() FGameplayTag       ActiveCharacterTag;
    UPROPERTY() int32              CharacterLevel     = 1;
    UPROPERTY() float              CharacterXP        = 0.0f;
    UPROPERTY() float              CurrentHealth      = 0.0f;
    UPROPERTY() float              CurrentChakra      = 0.0f;
    UPROPERTY() FVector            PlayerLocation     = FVector::ZeroVector;
    UPROPERTY() FRotator           PlayerRotation     = FRotator::ZeroRotator;

    // Progression
    UPROPERTY() TArray<FGameplayTag>                UnlockedJutsu;
    UPROPERTY() TMap<FGameplayTag, float>           JutsuMasteryXP;
    UPROPERTY() TArray<FGameplayTag>                UnlockedTitles;
    UPROPERTY() FGameplayTag                        ActiveTitle;
    UPROPERTY() int32                               AvailableSkillPoints = 0;
    UPROPERTY() TArray<FGameplayTag>                UnlockedSkillNodes;

    // World state
    UPROPERTY() TArray<FGameplayTag>                CompletedQuests;
    UPROPERTY() TMap<FGameplayTag, EQuestState>     QuestStates;
    UPROPERTY() TArray<FGameplayTag>                ActiveWorldStateFlags;
    UPROPERTY() TMap<EVillage, float>               FactionReputations;

    // Economy
    UPROPERTY() int64                               Ryo           = 0;  // Currency
    UPROPERTY() TArray<FGameplayTag>                InventoryItems;
    UPROPERTY() TMap<FGameplayTag, int32>           ItemQuantities;

    // Settings
    UPROPERTY() float                               PlaytimeSeconds = 0.0f;
    UPROPERTY() float                               InGameTimeOfDay = 6.0f;
    UPROPERTY() ESeason                             CurrentSeason   = ESeason::Spring;
};

// ============================================================
//  Callbacks
// ============================================================

DECLARE_DELEGATE_TwoParams(FOnSaveLoadComplete, bool /*bSuccess*/, FText /*ErrorMsg*/);
DECLARE_DELEGATE_TwoParams(FOnSaveWriteComplete, bool /*bSuccess*/, int32 /*SlotIndex*/);

// ============================================================
//  SaveManager
// ============================================================

/**
 * USaveManager
 *
 * Handles all save/load operations:
 *   - Manual saves (slots 0-8)
 *   - Autosaves (slots 9-11, rotating)
 *   - Cloud saves (platform-specific)
 *   - Backup saves (written before overwriting)
 *   - Version migration (upgrades old saves to current schema)
 *
 * All I/O is async. Results delivered via delegates.
 * The GameInstance holds the only reference to this subsystem.
 */
UCLASS(NotBlueprintable)
class NARUTOUSL_API USaveManager : public UNarutoSubsystem
{
    GENERATED_BODY()

public:

    USaveManager();

    void Initialize(UNarutoGameInstance* InGameInstance);
    virtual void Tick(float DeltaTime) override;
    virtual void Shutdown() override;

    // ------------------------------------------------------------------
    //  Save / Load
    // ------------------------------------------------------------------

    void SaveSlot(int32 SlotIndex, FOnSaveWriteComplete OnComplete = FOnSaveWriteComplete());
    void LoadSlot(int32 SlotIndex, FOnSaveLoadComplete  OnComplete = FOnSaveLoadComplete());
    void DeleteSlot(int32 SlotIndex);

    bool SlotHasSave(int32 SlotIndex) const;

    UFUNCTION(BlueprintPure, Category = "Save")
    FSaveSlotMetadata GetSlotMetadata(int32 SlotIndex) const;

    UFUNCTION(BlueprintPure, Category = "Save")
    TArray<FSaveSlotMetadata> GetAllSlotMetadata() const;

    // ------------------------------------------------------------------
    //  Autosave
    // ------------------------------------------------------------------

    void TriggerAutosave();
    void SetAutosaveEnabled(bool bEnabled);

    UFUNCTION(BlueprintPure, Category = "Save")
    int32 GetLastAutosaveSlot() const { return LastAutosaveSlot; }

    // ------------------------------------------------------------------
    //  Cloud
    // ------------------------------------------------------------------

    void SyncToCloud(int32 SlotIndex);
    void SyncFromCloud(int32 SlotIndex, FOnSaveLoadComplete OnComplete);

    // ------------------------------------------------------------------
    //  Version Migration
    // ------------------------------------------------------------------

    /**
     * Migrates a save from an older version to the current schema.
     * Called automatically during LoadSlot if version mismatch detected.
     */
    bool MigrateSave(UNarutoSaveGame* SaveGame);

private:

    UNarutoSaveGame* CollectSaveData();
    void             ApplySaveData(UNarutoSaveGame* SaveGame);
    FString          GetSlotName(int32 SlotIndex) const;
    void             WriteBackupSave(int32 SlotIndex);

    TWeakObjectPtr<UNarutoGameInstance> GameInstance;

    TMap<int32, FSaveSlotMetadata> SlotMetadataCache;

    bool  bAutosaveEnabled   = true;
    float AutosaveTimer      = 0.0f;
    int32 LastAutosaveSlot   = 9;
    int32 AutosaveSlotCursor = 0;

    static constexpr int32 ManualSlotCount  = 9;
    static constexpr int32 AutosaveSlotBase = 9;
    static constexpr int32 AutosaveSlotCount = 3;
};
