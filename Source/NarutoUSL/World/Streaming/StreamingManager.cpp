// Copyright (c) 2025 Maze Runner Studios. All Rights Reserved.

#include "World/Streaming/StreamingManager.h"
#include "Narrative/Core/WorldStateManager.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "NarutoUSL.h"

UStreamingManager::UStreamingManager()
{
    SubsystemName = TEXT("StreamingManager");
    bTickEnabled  = true;
    TickPriority  = ESubsystemTickPriority::PrePhysics;
}

void UStreamingManager::Initialize(UWorldStateManager* InWorldStateManager)
{
    WorldStateManager = InWorldStateManager;
    bInitialized      = true;
    UE_LOG(LogNarutoWorld, Log, TEXT("[StreamingManager] Initialized."));
}

void UStreamingManager::Shutdown()
{
    StreamingWorld = nullptr;
}

void UStreamingManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    if (bStreamingPaused) return;

    UpdateStreamingSources();

    RegionCheckTimer += DeltaTime;
    if (RegionCheckTimer >= RegionCheckInterval)
    {
        RegionCheckTimer = 0.0f;
        CheckRegionTransitions();
    }
}

void UStreamingManager::OnWorldChanged(UWorld* OldWorld, UWorld* NewWorld)
{
    StreamingWorld = NewWorld;
}

void UStreamingManager::SetStreamingPaused(bool bPaused)
{
    bStreamingPaused = bPaused;

    if (StreamingWorld.IsValid())
    {
        // In the full implementation this sets the World Partition streaming state
        UE_LOG(LogNarutoWorld, Log,
            TEXT("[StreamingManager] Streaming %s."),
            bPaused ? TEXT("PAUSED") : TEXT("RESUMED"));
    }
}

void UStreamingManager::FlushStreamingSync()
{
    UWorld* World = StreamingWorld.Get();
    if (!World) return;

    UE_LOG(LogNarutoWorld, Warning,
        TEXT("[StreamingManager] Synchronous streaming flush — may cause hitch."));

    // GEngine->Exec(World, TEXT("wp.Runtime.FlushStreaming"), *GLog); — full impl
}

void UStreamingManager::SetCurrentRegion(FGameplayTag RegionTag)
{
    if (CurrentRegionTag == RegionTag) return;

    const FGameplayTag OldRegion = CurrentRegionTag;
    CurrentRegionTag = RegionTag;

    UE_LOG(LogNarutoWorld, Log,
        TEXT("[StreamingManager] Region transition: %s → %s"),
        *OldRegion.ToString(), *RegionTag.ToString());

    if (WorldStateManager.IsValid())
    {
        if (OldRegion.IsValid())
            WorldStateManager->SetFlag(OldRegion, false);
        if (RegionTag.IsValid())
            WorldStateManager->SetFlag(RegionTag, true);
    }
}

void UStreamingManager::UpdateStreamingSources()
{
    UWorld* World = StreamingWorld.Get();
    if (!World) return;

    APlayerController* PC = World->GetFirstPlayerController();
    if (!PC || !PC->GetPawn()) return;

    LastPlayerLocation = PC->GetPawn()->GetActorLocation();
    // World Partition streaming sources are updated automatically by UE5
    // based on the player's viewport. This manager adjusts the radius and LOD bias.
}

void UStreamingManager::CheckRegionTransitions()
{
    // Region detection uses trigger volumes placed in the world.
    // This manager responds to overlap events from those volumes
    // via the WorldStateManager flag system.
    // Overlap binding is done in BeginPlay of the region trigger actors.
}
