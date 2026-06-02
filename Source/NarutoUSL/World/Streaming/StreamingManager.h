// Copyright (c) 2025 Maze Runner Studios. All Rights Reserved.
// Naruto: Ultimate Shinobi Legacy
// StreamingManager — World Partition streaming coordination and memory budgets.

#pragma once

#include "CoreMinimal.h"
#include "Core/Subsystems/NarutoSubsystem.h"
#include "Core/Types/NarutoTypes.h"
#include "StreamingManager.generated.h"

class UWorldStateManager;

/**
 * UStreamingManager
 *
 * Coordinates UE5 World Partition streaming with game state:
 *   - Sets streaming source positions based on player location
 *   - Adjusts streaming radius based on platform performance budget
 *   - Triggers region-specific events when player enters/exits zones
 *   - Manages asset memory budget (texture streaming, LOD bias)
 *   - Suppresses streaming during cinematics to prevent hitches
 */
UCLASS(NotBlueprintable)
class NARUTOUSL_API UStreamingManager : public UNarutoSubsystem
{
    GENERATED_BODY()

public:

    UStreamingManager();

    void Initialize(UWorldStateManager* InWorldStateManager);
    virtual void Tick(float DeltaTime) override;
    virtual void Shutdown() override;

    void OnWorldChanged(UWorld* OldWorld, UWorld* NewWorld);

    // ------------------------------------------------------------------
    //  Streaming Control
    // ------------------------------------------------------------------

    /** Temporarily pauses streaming (e.g., during cinematics or loading screens). */
    UFUNCTION(BlueprintCallable, Category = "Streaming")
    void SetStreamingPaused(bool bPaused);

    UFUNCTION(BlueprintPure, Category = "Streaming")
    bool IsStreamingPaused() const { return bStreamingPaused; }

    /** Forces a synchronous streaming flush. Use sparingly — causes a hitch. */
    UFUNCTION(BlueprintCallable, Category = "Streaming")
    void FlushStreamingSync();

    // ------------------------------------------------------------------
    //  Region Tracking
    // ------------------------------------------------------------------

    UFUNCTION(BlueprintPure, Category = "Streaming")
    FGameplayTag GetCurrentRegionTag() const { return CurrentRegionTag; }

    void SetCurrentRegion(FGameplayTag RegionTag);

    // ------------------------------------------------------------------
    //  Platform Budgets
    // ------------------------------------------------------------------

    UPROPERTY(EditAnywhere, Category = "Streaming|Budget",
        meta = (ClampMin = "1000", ClampMax = "50000"))
    float StreamingRadiusMeters = 5000.0f;

    UPROPERTY(EditAnywhere, Category = "Streaming|Budget",
        meta = (ClampMin = "256", ClampMax = "8192"))
    int32 MaxStreamedTextureMB = 2048;

private:

    void UpdateStreamingSources();
    void CheckRegionTransitions();

    TWeakObjectPtr<UWorldStateManager> WorldStateManager;
    TWeakObjectPtr<UWorld>             StreamingWorld;

    FGameplayTag CurrentRegionTag;
    FVector      LastPlayerLocation = FVector::ZeroVector;
    bool         bStreamingPaused   = false;
    float        RegionCheckTimer   = 0.0f;

    static constexpr float RegionCheckInterval = 1.0f;
};
