// Copyright (c) 2025 Maze Runner Studios. All Rights Reserved.

#include "Analytics/AnalyticsManager.h"
#include "Core/Events/NarutoEventBus.h"
#include "HAL/PlatformTime.h"
#include "NarutoUSL.h"

UAnalyticsManager::UAnalyticsManager()
{
    SubsystemName = TEXT("AnalyticsManager");
    bTickEnabled  = true;
    TickPriority  = ESubsystemTickPriority::PostRender;
}

void UAnalyticsManager::Initialize(UNarutoEventBus* InEventBus)
{
    EventBus         = InEventBus;
    SessionStartTime = FPlatformTime::Seconds();
    bInitialized     = true;
    StartSession();
    UE_LOG(LogNarutoUSL, Log, TEXT("[AnalyticsManager] Initialized."));
}

void UAnalyticsManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    RecordPerformanceMetrics(DeltaTime);

    FlushTimer += DeltaTime;
    if (FlushTimer >= FlushInterval)
    {
        FlushTimer = 0.0f;
        FlushEventQueue();
    }
}

void UAnalyticsManager::Shutdown()
{
    EndSession();
    FlushEventQueue();
    EventQueue.Empty();
}

// ============================================================
//  Event Recording
// ============================================================

void UAnalyticsManager::RecordEvent(const FString& EventName,
                                      const TMap<FString, FString>& Parameters)
{
    if (!bTelemetryConsented) return;

    FAnalyticsEvent Event;
    Event.EventName  = EventName;
    Event.Parameters = Parameters;
    Event.Timestamp  = FPlatformTime::Seconds() - SessionStartTime;
    EventQueue.Add(MoveTemp(Event));
}

void UAnalyticsManager::RecordCombatEvent(const FString& EventType, float Value)
{
    TMap<FString, FString> Params;
    Params.Add(TEXT("type"),  EventType);
    Params.Add(TEXT("value"), FString::SanitizeFloat(Value));
    RecordEvent(TEXT("combat"), Params);
}

void UAnalyticsManager::RecordProgressionEvent(const FString& EventType, int32 Value)
{
    TMap<FString, FString> Params;
    Params.Add(TEXT("type"),  EventType);
    Params.Add(TEXT("value"), FString::FromInt(Value));
    RecordEvent(TEXT("progression"), Params);
}

// ============================================================
//  Session
// ============================================================

void UAnalyticsManager::StartSession()
{
    TMap<FString, FString> Params;
    Params.Add(TEXT("platform"), FPlatformProperties::PlatformName());
    RecordEvent(TEXT("session_start"), Params);
}

void UAnalyticsManager::EndSession()
{
    const double Duration = FPlatformTime::Seconds() - SessionStartTime;
    TMap<FString, FString> Params;
    Params.Add(TEXT("duration_seconds"), FString::SanitizeFloat(Duration));
    Params.Add(TEXT("avg_fps"),          FString::SanitizeFloat(AvgFPS));
    Params.Add(TEXT("hitch_count"),      FString::FromInt(HitchCount));
    RecordEvent(TEXT("session_end"), Params);
}

void UAnalyticsManager::SetTelemetryConsent(bool bConsented)
{
    bTelemetryConsented = bConsented;
    UE_LOG(LogNarutoUSL, Log,
        TEXT("[AnalyticsManager] Telemetry consent: %s"),
        bConsented ? TEXT("GRANTED") : TEXT("DENIED"));
}

// ============================================================
//  Internal
// ============================================================

void UAnalyticsManager::FlushEventQueue()
{
    if (EventQueue.IsEmpty() || !bTelemetryConsented) return;

    // In shipping: batch-send to analytics backend (e.g., AWS Pinpoint, GameAnalytics)
    // In development: log locally
    UE_LOG(LogNarutoUSL, Verbose,
        TEXT("[AnalyticsManager] Flushing %d events."), EventQueue.Num());

    EventQueue.Empty();
}

void UAnalyticsManager::RecordPerformanceMetrics(float DeltaTime)
{
    PerformanceTimer += DeltaTime;
    if (PerformanceTimer < 1.0f) return;
    PerformanceTimer = 0.0f;

    const float CurrentFPS = 1.0f / FMath::Max(DeltaTime, SMALL_NUMBER);
    AvgFPS  = FMath::Lerp(AvgFPS, CurrentFPS, 0.1f);
    MinFPS  = FMath::Min(MinFPS, CurrentFPS);

    // Record hitches (frames longer than 100ms)
    if (DeltaTime > 0.1f)
    {
        ++HitchCount;
    }
}
