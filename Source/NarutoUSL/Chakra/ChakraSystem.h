// Copyright (c) 2025 Maze Runner Studios. All Rights Reserved.
// Naruto: Ultimate Shinobi Legacy
// ChakraSystem — Subsystem that ticks all registered ChakraComponents.

#pragma once

#include "CoreMinimal.h"
#include "Core/Subsystems/NarutoSubsystem.h"
#include "ChakraSystem.generated.h"

class UNarutoEventBus;
class UChakraComponent;
class ANarutoCharacterBase;

/**
 * UChakraSystem
 *
 * Owns the tick loop for all active ChakraComponents.
 * ChakraComponents do not self-tick — they register here and
 * the system drives them in priority order each frame.
 *
 * Responsibilities:
 *   - Tick passive regen for all registered components
 *   - Enforce global chakra rules (e.g., no regen during certain world events)
 *   - Provide batch chakra queries for AI decision-making
 */
UCLASS(NotBlueprintable)
class NARUTOUSL_API UChakraSystem : public UNarutoSubsystem
{
    GENERATED_BODY()

public:

    UChakraSystem();

    void Initialize(UNarutoEventBus* InEventBus);
    virtual void Tick(float DeltaTime) override;
    virtual void Shutdown() override;
    virtual TMap<FString, FString> GetDebugInfo() const override;

    void RegisterComponent(UChakraComponent* Component);
    void UnregisterComponent(UChakraComponent* Component);

    /** Suppresses all chakra regen globally (e.g., during certain boss mechanics). */
    void SetGlobalRegenSuppressed(bool bSuppressed);

    bool IsGlobalRegenSuppressed() const { return bGlobalRegenSuppressed; }

private:

    TArray<TWeakObjectPtr<UChakraComponent>> RegisteredComponents;
    TWeakObjectPtr<UNarutoEventBus> EventBus;
    bool bGlobalRegenSuppressed = false;
};
