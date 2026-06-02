// Copyright (c) 2025 Maze Runner Studios. All Rights Reserved.

#include "Chakra/ChakraSystem.h"
#include "Character/Components/ChakraComponent.h"
#include "NarutoUSL.h"

UChakraSystem::UChakraSystem()
{
    SubsystemName = TEXT("ChakraSystem");
    bTickEnabled  = true;
    TickPriority  = ESubsystemTickPriority::PrePhysics;
}

void UChakraSystem::Initialize(UNarutoEventBus* InEventBus)
{
    EventBus     = InEventBus;
    bInitialized = true;
    UE_LOG(LogNarutoChakra, Log, TEXT("[ChakraSystem] Initialized."));
}

void UChakraSystem::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (bGlobalRegenSuppressed) return;

    for (int32 i = RegisteredComponents.Num() - 1; i >= 0; --i)
    {
        UChakraComponent* Comp = RegisteredComponents[i].Get();
        if (!Comp)
        {
            RegisteredComponents.RemoveAt(i);
            continue;
        }
        Comp->TickChakra(DeltaTime);
    }
}

void UChakraSystem::Shutdown()
{
    RegisteredComponents.Empty();
}

void UChakraSystem::RegisterComponent(UChakraComponent* Component)
{
    if (Component && !RegisteredComponents.Contains(Component))
    {
        RegisteredComponents.Add(Component);
    }
}

void UChakraSystem::UnregisterComponent(UChakraComponent* Component)
{
    RegisteredComponents.Remove(Component);
}

void UChakraSystem::SetGlobalRegenSuppressed(bool bSuppressed)
{
    bGlobalRegenSuppressed = bSuppressed;
}

TMap<FString, FString> UChakraSystem::GetDebugInfo() const
{
    TMap<FString, FString> Info = Super::GetDebugInfo();
    Info.Add(TEXT("RegisteredComponents"), FString::FromInt(RegisteredComponents.Num()));
    Info.Add(TEXT("RegenSuppressed"),      bGlobalRegenSuppressed ? TEXT("YES") : TEXT("NO"));
    return Info;
}
