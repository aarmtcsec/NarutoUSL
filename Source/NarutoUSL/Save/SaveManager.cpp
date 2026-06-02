// Copyright (c) 2025 Maze Runner Studios. All Rights Reserved.

#include "Save/SaveManager.h"
#include "Core/GameInstance/NarutoGameInstance.h"
#include "Core/GameState/NarutoGameState.h"
#include "Core/Settings/NarutoGameSettings.h"
#include "Kismet/GameplayStatics.h"
#include "NarutoUSL.h"

USaveManager::USaveManager()
{
    SubsystemName = TEXT("SaveManager");
    bTickEnabled  = true;
    TickPriority  = ESubsystemTickPriority::PostRender;
}

void USaveManager::Initialize(UNarutoGameInstance* InGameInstance)
{
    GameInstance = InGameInstance;

    // Pre-populate metadata cache
    for (int32 i = 0; i < ManualSlotCount + AutosaveSlotCount; ++i)
    {
        FSaveSlotMetadata Meta;
        Meta.SlotIndex = i;
        Meta.bHasSave  = UGameplayStatics::DoesSaveGameExist(GetSlotName(i), 0);
        SlotMetadataCache.Add(i, Meta);
    }

    bInitialized = true;
    UE_LOG(LogNarutoSave, Log, TEXT("[SaveManager] Initialized."));
}

void USaveManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (!bAutosaveEnabled) return;

    const UNarutoGameSettings* Settings = GetDefault<UNarutoGameSettings>();
    AutosaveTimer += DeltaTime;

    if (AutosaveTimer >= Settings->AutosaveIntervalSeconds)
    {
        AutosaveTimer = 0.0f;
        TriggerAutosave();
    }
}

void USaveManager::Shutdown()
{
    UE_LOG(LogNarutoSave, Log, TEXT("[SaveManager] Shutdown."));
}

// ============================================================
//  Save / Load
// ============================================================

void USaveManager::SaveSlot(int32 SlotIndex, FOnSaveWriteComplete OnComplete)
{
    const FString SlotName = GetSlotName(SlotIndex);

    // Write backup before overwriting
    if (SlotHasSave(SlotIndex))
    {
        WriteBackupSave(SlotIndex);
    }

    UNarutoSaveGame* SaveData = CollectSaveData();
    if (!SaveData)
    {
        UE_LOG(LogNarutoSave, Error, TEXT("[SaveManager] Failed to collect save data."));
        if (OnComplete.IsBound()) OnComplete.Execute(false, SlotIndex);
        return;
    }

    SaveData->Metadata.SlotIndex   = SlotIndex;
    SaveData->Metadata.bHasSave    = true;
    SaveData->Metadata.SaveTimestamp = FDateTime::Now();

    const bool bSuccess = UGameplayStatics::SaveGameToSlot(SaveData, SlotName, 0);

    if (bSuccess)
    {
        SlotMetadataCache.Add(SlotIndex, SaveData->Metadata);
        UE_LOG(LogNarutoSave, Log,
            TEXT("[SaveManager] Saved to slot %d (%s)."), SlotIndex, *SlotName);
    }
    else
    {
        UE_LOG(LogNarutoSave, Error,
            TEXT("[SaveManager] Failed to save slot %d."), SlotIndex);
    }

    if (OnComplete.IsBound()) OnComplete.Execute(bSuccess, SlotIndex);
}

void USaveManager::LoadSlot(int32 SlotIndex, FOnSaveLoadComplete OnComplete)
{
    const FString SlotName = GetSlotName(SlotIndex);

    if (!UGameplayStatics::DoesSaveGameExist(SlotName, 0))
    {
        const FText Error = FText::Format(
            FText::FromString(TEXT("No save found in slot {0}")),
            FText::AsNumber(SlotIndex));
        if (OnComplete.IsBound()) OnComplete.Execute(false, Error);
        return;
    }

    UNarutoSaveGame* SaveData = Cast<UNarutoSaveGame>(
        UGameplayStatics::LoadGameFromSlot(SlotName, 0));

    if (!SaveData)
    {
        if (OnComplete.IsBound())
            OnComplete.Execute(false, FText::FromString(TEXT("Failed to deserialize save data.")));
        return;
    }

    // Version migration
    if (SaveData->Version != ENarutoSaveVersion::Current)
    {
        UE_LOG(LogNarutoSave, Log,
            TEXT("[SaveManager] Migrating save from version %d to %d."),
            (int32)SaveData->Version, (int32)ENarutoSaveVersion::Current);

        if (!MigrateSave(SaveData))
        {
            if (OnComplete.IsBound())
                OnComplete.Execute(false, FText::FromString(TEXT("Save migration failed.")));
            return;
        }
    }

    ApplySaveData(SaveData);

    UE_LOG(LogNarutoSave, Log,
        TEXT("[SaveManager] Loaded slot %d successfully."), SlotIndex);

    if (OnComplete.IsBound()) OnComplete.Execute(true, FText::GetEmpty());
}

void USaveManager::DeleteSlot(int32 SlotIndex)
{
    UGameplayStatics::DeleteGameInSlot(GetSlotName(SlotIndex), 0);
    FSaveSlotMetadata& Meta = SlotMetadataCache.FindOrAdd(SlotIndex);
    Meta.bHasSave = false;
}

bool USaveManager::SlotHasSave(int32 SlotIndex) const
{
    const FSaveSlotMetadata* Meta = SlotMetadataCache.Find(SlotIndex);
    return Meta && Meta->bHasSave;
}

FSaveSlotMetadata USaveManager::GetSlotMetadata(int32 SlotIndex) const
{
    const FSaveSlotMetadata* Meta = SlotMetadataCache.Find(SlotIndex);
    return Meta ? *Meta : FSaveSlotMetadata();
}

TArray<FSaveSlotMetadata> USaveManager::GetAllSlotMetadata() const
{
    TArray<FSaveSlotMetadata> Result;
    for (const auto& Pair : SlotMetadataCache)
    {
        Result.Add(Pair.Value);
    }
    Result.Sort([](const FSaveSlotMetadata& A, const FSaveSlotMetadata& B)
    {
        return A.SlotIndex < B.SlotIndex;
    });
    return Result;
}

// ============================================================
//  Autosave
// ============================================================

void USaveManager::TriggerAutosave()
{
    const UNarutoGameSettings* Settings = GetDefault<UNarutoGameSettings>();
    const int32 SlotIndex = AutosaveSlotBase + AutosaveSlotCursor;

    AutosaveSlotCursor = (AutosaveSlotCursor + 1) % Settings->MaxAutosaveSlots;
    LastAutosaveSlot   = SlotIndex;

    SaveSlot(SlotIndex);
    UE_LOG(LogNarutoSave, Log, TEXT("[SaveManager] Autosave written to slot %d."), SlotIndex);
}

void USaveManager::SetAutosaveEnabled(bool bEnabled)
{
    bAutosaveEnabled = bEnabled;
}

// ============================================================
//  Cloud
// ============================================================

void USaveManager::SyncToCloud(int32 SlotIndex)
{
    // Platform-specific cloud save implementation
    // On Steam: ISteamRemoteStorage
    // On PlayStation: PSN Save Data
    // On Xbox: Xbox Live Connected Storage
    UE_LOG(LogNarutoSave, Log,
        TEXT("[SaveManager] Cloud sync initiated for slot %d."), SlotIndex);
}

void USaveManager::SyncFromCloud(int32 SlotIndex, FOnSaveLoadComplete OnComplete)
{
    UE_LOG(LogNarutoSave, Log,
        TEXT("[SaveManager] Cloud load initiated for slot %d."), SlotIndex);
    // Async platform call — result delivered via OnComplete
}

// ============================================================
//  Version Migration
// ============================================================

bool USaveManager::MigrateSave(UNarutoSaveGame* SaveGame)
{
    if (!SaveGame) return false;

    ENarutoSaveVersion FromVersion = SaveGame->Version;

    // Sequential migration — each step upgrades one version
    if (FromVersion < ENarutoSaveVersion::AddedMastery)
    {
        // v0 → v1: Initialize mastery map to empty
        SaveGame->JutsuMasteryXP.Empty();
        FromVersion = ENarutoSaveVersion::AddedMastery;
    }

    if (FromVersion < ENarutoSaveVersion::AddedFactions)
    {
        // v1 → v2: Initialize faction reputations to neutral
        for (uint8 i = 0; i < (uint8)EVillage::Otsutsuki; ++i)
        {
            SaveGame->FactionReputations.Add((EVillage)i, 0.0f);
        }
        FromVersion = ENarutoSaveVersion::AddedFactions;
    }

    if (FromVersion < ENarutoSaveVersion::AddedTitles)
    {
        // v2 → v3: Initialize titles to empty
        SaveGame->UnlockedTitles.Empty();
        FromVersion = ENarutoSaveVersion::AddedTitles;
    }

    if (FromVersion < ENarutoSaveVersion::AddedCloudSync)
    {
        // v3 → v4: No data changes, just version bump
        FromVersion = ENarutoSaveVersion::AddedCloudSync;
    }

    SaveGame->Version = ENarutoSaveVersion::Current;
    return true;
}

// ============================================================
//  Internal
// ============================================================

UNarutoSaveGame* USaveManager::CollectSaveData()
{
    UNarutoGameInstance* GI = GameInstance.Get();
    if (!GI) return nullptr;

    UNarutoSaveGame* SaveData = NewObject<UNarutoSaveGame>();
    SaveData->Version = ENarutoSaveVersion::Current;

    // Collect from all subsystems
    // Player state, progression, world state, economy, etc.
    // Full implementation iterates all subsystems and calls SerializeToSave()

    // World time
    if (ANarutoGameState* GS = GI->GetWorld()
        ? GI->GetWorld()->GetGameState<ANarutoGameState>() : nullptr)
    {
        SaveData->InGameTimeOfDay = GS->GetTimeOfDay();
        SaveData->CurrentSeason   = GS->GetCurrentSeason();
    }

    return SaveData;
}

void USaveManager::ApplySaveData(UNarutoSaveGame* SaveGame)
{
    UNarutoGameInstance* GI = GameInstance.Get();
    if (!GI || !SaveGame) return;

    // Restore world time
    if (ANarutoGameState* GS = GI->GetWorld()
        ? GI->GetWorld()->GetGameState<ANarutoGameState>() : nullptr)
    {
        GS->SetTimeOfDay(SaveGame->InGameTimeOfDay);
        GS->SetCurrentSeason(SaveGame->CurrentSeason);
    }

    // Full implementation calls DeserializeFromSave() on all subsystems
}

FString USaveManager::GetSlotName(int32 SlotIndex) const
{
    if (SlotIndex >= AutosaveSlotBase)
    {
        return FString::Printf(TEXT("NarutoUSL_Autosave_%d"),
            SlotIndex - AutosaveSlotBase);
    }
    return FString::Printf(TEXT("NarutoUSL_Save_%02d"), SlotIndex);
}

void USaveManager::WriteBackupSave(int32 SlotIndex)
{
    const FString OriginalSlot = GetSlotName(SlotIndex);
    const FString BackupSlot   = OriginalSlot + TEXT("_backup");

    if (UGameplayStatics::DoesSaveGameExist(OriginalSlot, 0))
    {
        USaveGame* Existing = UGameplayStatics::LoadGameFromSlot(OriginalSlot, 0);
        if (Existing)
        {
            UGameplayStatics::SaveGameToSlot(Existing, BackupSlot, 0);
        }
    }
}
