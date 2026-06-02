// Copyright (c) 2025 Maze Runner Studios. All Rights Reserved.

#include "Core/Subsystems/NarutoSubsystem.h"
#include "HAL/PlatformTime.h"

UNarutoSubsystem::UNarutoSubsystem()
{
}

void UNarutoSubsystem::Tick(float DeltaTime)
{
    // Performance tracking — measures actual tick duration for profiling.
    // Subclasses call Super::Tick() at the START of their override so the
    // timer wraps their full execution.
    const double StartTime = FPlatformTime::Seconds();

    // (Subclass logic runs after Super::Tick() returns)

    const double EndTime = FPlatformTime::Seconds();
    LastTickDurationMs = static_cast<float>((EndTime - StartTime) * 1000.0);

    TickDurationAccum += LastTickDurationMs;
    ++TickSampleCount;

    if (TickSampleCount >= TickAverageSampleWindow)
    {
        AverageTickDurationMs = TickDurationAccum / static_cast<float>(TickSampleCount);
        TickDurationAccum = 0.0f;
        TickSampleCount   = 0;
    }
}

TMap<FString, FString> UNarutoSubsystem::GetDebugInfo() const
{
    TMap<FString, FString> Info;
    Info.Add(TEXT("Name"),              SubsystemName);
    Info.Add(TEXT("Initialized"),       bInitialized ? TEXT("true") : TEXT("false"));
    Info.Add(TEXT("TickEnabled"),       bTickEnabled ? TEXT("true") : TEXT("false"));
    Info.Add(TEXT("LastTickMs"),        FString::Printf(TEXT("%.3f"), LastTickDurationMs));
    Info.Add(TEXT("AvgTickMs"),         FString::Printf(TEXT("%.3f"), AverageTickDurationMs));
    return Info;
}
