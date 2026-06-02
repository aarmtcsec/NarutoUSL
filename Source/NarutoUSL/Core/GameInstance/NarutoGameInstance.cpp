// Copyright (c) 2025 Maze Runner Studios. All Rights Reserved.

#include "Core/GameInstance/NarutoGameInstance.h"

// Subsystem headers — included only in .cpp to keep compile times fast
#include "Core/Events/NarutoEventBus.h"
#include "Narrative/Core/WorldStateManager.h"
#include "Narrative/Faction/FactionManager.h"
#include "Narrative/Quest/QuestManager.h"
#include "Narrative/Core/NarrativeManager.h"
#include "Chakra/ChakraSystem.h"
#include "Combat/Core/CombatManager.h"
#include "Jutsu/Core/JutsuManager.h"
#include "Progression/Core/ProgressionManager.h"
#include "Economy/EconomyManager.h"
#include "Audio/AudioManager.h"
#include "UI/Core/UIManager.h"
#include "Save/SaveManager.h"
#include "World/Streaming/StreamingManager.h"

#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"

// ============================================================
//  Constructor
// ============================================================

UNarutoGameInstance::UNarutoGameInstance(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
}

// ============================================================
//  UGameInstance Interface
// ============================================================

void UNarutoGameInstance::Init()
{
    Super::Init();

    UE_LOG(LogTemp, Log, TEXT("[NarutoGameInstance] Initializing Naruto: Ultimate Shinobi Legacy v%d.%d.%d"),
        NARUTOUSL_VERSION_MAJOR, NARUTOUSL_VERSION_MINOR, NARUTOUSL_VERSION_PATCH);

    CreateSubsystems();
    InitializeSubsystemsInOrder();
    BindCrossSubsystemEvents();

    bGameInitialized = true;
    OnGameInitialized.Broadcast();

    UE_LOG(LogTemp, Log, TEXT("[NarutoGameInstance] All subsystems initialized successfully."));
}

void UNarutoGameInstance::Shutdown()
{
    UE_LOG(LogTemp, Log, TEXT("[NarutoGameInstance] Shutting down."));

    ShutdownSubsystems();

    if (EventBus)
    {
        EventBus->ClearAllBindings();
    }

    Super::Shutdown();
}

void UNarutoGameInstance::OnStart()
{
    Super::OnStart();
}

void UNarutoGameInstance::OnWorldChanged(UWorld* OldWorld, UWorld* NewWorld)
{
    Super::OnWorldChanged(OldWorld, NewWorld);

    if (StreamingManager && NewWorld)
    {
        StreamingManager->OnWorldChanged(OldWorld, NewWorld);
    }

    if (AudioManager && NewWorld)
    {
        AudioManager->OnWorldChanged(OldWorld, NewWorld);
    }
}

// ============================================================
//  Static Accessor
// ============================================================

UNarutoGameInstance* UNarutoGameInstance::Get(const UObject* WorldContextObject)
{
    UGameInstance* GI = UGameplayStatics::GetGameInstance(WorldContextObject);
    UNarutoGameInstance* NarutoGI = Cast<UNarutoGameInstance>(GI);
    checkf(NarutoGI, TEXT("[NarutoGameInstance::Get] Failed to cast GameInstance. "
                          "Ensure DefaultGameInstanceClass is set to UNarutoGameInstance."));
    return NarutoGI;
}

// ============================================================
//  Subsystem Lifecycle
// ============================================================

void UNarutoGameInstance::CreateSubsystems()
{
    // Create all subsystems as outer-owned UObjects so GC tracks them.
    // Order here is declaration order — initialization order is separate.

    EventBus           = NewObject<UNarutoEventBus>(this,           TEXT("EventBus"));
    WorldStateManager  = NewObject<UWorldStateManager>(this,        TEXT("WorldStateManager"));
    FactionManager     = NewObject<UFactionManager>(this,           TEXT("FactionManager"));
    QuestManager       = NewObject<UQuestManager>(this,             TEXT("QuestManager"));
    NarrativeManager   = NewObject<UNarrativeManager>(this,         TEXT("NarrativeManager"));
    ChakraSystem       = NewObject<UChakraSystem>(this,             TEXT("ChakraSystem"));
    CombatManager      = NewObject<UCombatManager>(this,            TEXT("CombatManager"));
    JutsuManager       = NewObject<UJutsuManager>(this,             TEXT("JutsuManager"));
    ProgressionManager = NewObject<UProgressionManager>(this,       TEXT("ProgressionManager"));
    EconomyManager     = NewObject<UEconomyManager>(this,           TEXT("EconomyManager"));
    AudioManager       = NewObject<UAudioManager>(this,             TEXT("AudioManager"));
    UIManager          = NewObject<UUIManager>(this,                TEXT("UIManager"));
    AnalyticsManager   = NewObject<UAnalyticsManager>(this,         TEXT("AnalyticsManager"));
    StreamingManager   = NewObject<UStreamingManager>(this,         TEXT("StreamingManager"));
    SaveManager        = NewObject<USaveManager>(this,              TEXT("SaveManager"));
}

void UNarutoGameInstance::InitializeSubsystemsInOrder()
{
    // Each Initialize() call receives only the dependencies it needs.
    // No subsystem receives the full GameInstance pointer — this enforces
    // the dependency graph and prevents hidden coupling.

    EventBus->Initialize();

    WorldStateManager->Initialize(EventBus);
    FactionManager->Initialize(EventBus, WorldStateManager);
    QuestManager->Initialize(EventBus, WorldStateManager, FactionManager);
    NarrativeManager->Initialize(QuestManager, WorldStateManager);

    ChakraSystem->Initialize(EventBus);
    CombatManager->Initialize(ChakraSystem, EventBus);
    JutsuManager->Initialize(ChakraSystem, CombatManager, EventBus);

    ProgressionManager->Initialize(EventBus, FactionManager);
    EconomyManager->Initialize(FactionManager, EventBus);

    AudioManager->Initialize(EventBus);
    UIManager->Initialize(EventBus);
    AnalyticsManager->Initialize(EventBus);
    StreamingManager->Initialize(WorldStateManager);

    // SaveManager last — it needs all subsystems ready to restore state into them.
    SaveManager->Initialize(this);
}

void UNarutoGameInstance::BindCrossSubsystemEvents()
{
    // Wire up cross-subsystem reactions that don't fit cleanly into
    // a single subsystem's Initialize() scope.

    if (EventBus)
    {
        EventBus->OnCombatEncounterChanged.AddUObject(
            this, &UNarutoGameInstance::HandleCombatEncounterChanged);
    }
}

void UNarutoGameInstance::ShutdownSubsystems()
{
    // Shutdown in reverse initialization order to respect dependencies.
    if (SaveManager)        SaveManager->Shutdown();
    if (StreamingManager)   StreamingManager->Shutdown();
    if (AnalyticsManager)   AnalyticsManager->Shutdown();
    if (UIManager)          UIManager->Shutdown();
    if (AudioManager)       AudioManager->Shutdown();
    if (EconomyManager)     EconomyManager->Shutdown();
    if (ProgressionManager) ProgressionManager->Shutdown();
    if (JutsuManager)       JutsuManager->Shutdown();
    if (CombatManager)      CombatManager->Shutdown();
    if (ChakraSystem)       ChakraSystem->Shutdown();
    if (NarrativeManager)   NarrativeManager->Shutdown();
    if (QuestManager)       QuestManager->Shutdown();
    if (FactionManager)     FactionManager->Shutdown();
    if (WorldStateManager)  WorldStateManager->Shutdown();
    // EventBus last — other subsystems may fire events during shutdown.
    if (EventBus)           EventBus->ClearAllBindings();
}

// ============================================================
//  Save / Load
// ============================================================

void UNarutoGameInstance::LoadGame(int32 SlotIndex)
{
    if (!SaveManager)
    {
        OnSaveFailed.Broadcast(FText::FromString(TEXT("SaveManager not initialized.")));
        return;
    }

    SaveManager->LoadSlot(SlotIndex,
        FOnSaveLoadComplete::CreateLambda([this, SlotIndex](bool bSuccess, FText ErrorMsg)
        {
            if (bSuccess)
            {
                OnSaveLoaded.Broadcast(SlotIndex);
            }
            else
            {
                OnSaveFailed.Broadcast(ErrorMsg);
            }
        }));
}

void UNarutoGameInstance::SaveGame(int32 SlotIndex)
{
    if (!SaveManager)
    {
        UE_LOG(LogTemp, Error, TEXT("[NarutoGameInstance] SaveGame called but SaveManager is null."));
        return;
    }

    SaveManager->SaveSlot(SlotIndex);
}

bool UNarutoGameInstance::HasSaveInSlot(int32 SlotIndex) const
{
    return SaveManager ? SaveManager->SlotHasSave(SlotIndex) : false;
}

// ============================================================
//  State Control
// ============================================================

void UNarutoGameInstance::SetCinematicState(bool bActive)
{
    if (bIsInCinematic == bActive) return;

    bIsInCinematic = bActive;

    if (CombatManager)
    {
        CombatManager->SetCombatPaused(bActive);
    }

    if (UIManager)
    {
        UIManager->SetCinematicMode(bActive);
    }
}

// ============================================================
//  Event Handlers
// ============================================================

void UNarutoGameInstance::HandleCombatEncounterChanged(const FCombatEncounterEvent& Event)
{
    bIsInCombat = Event.bCombatStarted;

    if (AudioManager)
    {
        AudioManager->OnCombatStateChanged(bIsInCombat);
    }
}
