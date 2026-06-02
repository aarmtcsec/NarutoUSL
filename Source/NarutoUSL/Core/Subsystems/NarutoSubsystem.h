// Copyright (c) 2025 Maze Runner Studios. All Rights Reserved.
// Naruto: Ultimate Shinobi Legacy
// NarutoSubsystem — Abstract base class for all game subsystems.
//
// Every major system (CombatManager, ChakraSystem, QuestManager, etc.)
// inherits from this base. Provides a uniform lifecycle, tick registration,
// debug tooling hooks, and serialization contract.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "NarutoSubsystem.generated.h"

class UNarutoEventBus;

// ============================================================
//  Subsystem Priority (controls tick order)
// ============================================================

UENUM()
enum class ESubsystemTickPriority : uint8
{
    PrePhysics      = 0,   // Input, chakra regen
    Physics         = 1,   // Combat resolution, hitbox evaluation
    PostPhysics     = 2,   // AI reactions, status effects
    PreRender       = 3,   // Animation, VFX
    PostRender      = 4,   // UI, analytics
};

// ============================================================
//  NarutoSubsystem
// ============================================================

/**
 * UNarutoSubsystem
 *
 * Abstract base for all game subsystems owned by UNarutoGameInstance.
 *
 * Lifecycle contract:
 *   Initialize() → called once after construction, before first tick.
 *   PostInitialize() → called after ALL subsystems have been initialized.
 *   Tick(DeltaTime) → called each frame if bTickEnabled = true.
 *   Shutdown() → called before destruction, in reverse init order.
 *
 * Serialization contract:
 *   SerializeToSave(Archive) → write state to save archive.
 *   DeserializeFromSave(Archive) → restore state from save archive.
 *
 * Debug contract:
 *   GetDebugInfo() → returns a string map of key/value debug data.
 *   DrawDebugHUD(Canvas) → optional on-screen debug rendering.
 */
UCLASS(Abstract, NotBlueprintable)
class NARUTOUSL_API UNarutoSubsystem : public UObject
{
    GENERATED_BODY()

public:

    UNarutoSubsystem();

    // ------------------------------------------------------------------
    //  Lifecycle
    // ------------------------------------------------------------------

    /**
     * Called once after construction. Subclasses override this to set up
     * internal state and subscribe to EventBus delegates.
     * Do NOT call Super::Initialize() — each subclass manages its own init.
     */
    virtual void Initialize() {}

    /**
     * Called after ALL subsystems have been initialized.
     * Use this for cross-subsystem wiring that requires other systems to
     * already be in a valid state.
     */
    virtual void PostInitialize() {}

    /**
     * Called each frame. Only invoked if bTickEnabled is true.
     * Subclasses should call Super::Tick() to update timing stats.
     */
    virtual void Tick(float DeltaTime);

    /**
     * Called before destruction. Subclasses should unsubscribe from all
     * delegates and release external resources here.
     */
    virtual void Shutdown() {}

    // ------------------------------------------------------------------
    //  Serialization
    // ------------------------------------------------------------------

    /**
     * Writes this subsystem's persistent state to the provided archive.
     * Called by SaveManager during a save operation.
     */
    virtual void SerializeToSave(FArchive& Ar) {}

    /**
     * Restores this subsystem's state from the provided archive.
     * Called by SaveManager after loading a save slot.
     */
    virtual void DeserializeFromSave(FArchive& Ar) {}

    // ------------------------------------------------------------------
    //  Debug
    // ------------------------------------------------------------------

    /**
     * Returns a map of key/value pairs describing current subsystem state.
     * Used by the in-game debug overlay and editor tools.
     */
    virtual TMap<FString, FString> GetDebugInfo() const;

    /**
     * Optional: draw debug information directly to the HUD canvas.
     * Only called in non-shipping builds when debug mode is active.
     */
    virtual void DrawDebugHUD(class UCanvas* Canvas) {}

    // ------------------------------------------------------------------
    //  Accessors
    // ------------------------------------------------------------------

    bool IsInitialized() const { return bInitialized; }
    bool IsTickEnabled() const { return bTickEnabled; }
    const FString& GetSubsystemName() const { return SubsystemName; }
    ESubsystemTickPriority GetTickPriority() const { return TickPriority; }

    float GetLastTickDuration() const { return LastTickDurationMs; }
    float GetAverageTickDuration() const { return AverageTickDurationMs; }

protected:

    // ------------------------------------------------------------------
    //  Configuration (set in subclass constructor or Initialize())
    // ------------------------------------------------------------------

    /** Human-readable name used in logs and debug UI. */
    FString SubsystemName = TEXT("UnnamedSubsystem");

    /** Whether this subsystem receives Tick() calls. */
    bool bTickEnabled = false;

    /** Tick priority relative to other subsystems. */
    ESubsystemTickPriority TickPriority = ESubsystemTickPriority::PostPhysics;

    /** Set to true at the end of Initialize(). */
    bool bInitialized = false;

private:

    // ------------------------------------------------------------------
    //  Performance Tracking
    // ------------------------------------------------------------------

    float LastTickDurationMs    = 0.0f;
    float AverageTickDurationMs = 0.0f;
    float TickDurationAccum     = 0.0f;
    int32 TickSampleCount       = 0;

    static constexpr int32 TickAverageSampleWindow = 60;
};
