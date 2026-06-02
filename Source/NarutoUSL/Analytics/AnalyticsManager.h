// Copyright (c) 2025 Maze Runner Studios. All Rights Reserved.
// Naruto: Ultimate Shinobi Legacy
// AnalyticsManager — Telemetry, crash reporting, and session metrics.

#pragma once

#include "CoreMinimal.h"
#include "Core/Subsystems/NarutoSubsystem.h"
#include "AnalyticsManager.generated.h"

class UNarutoEventBus;

/**
 * UAnalyticsManager
 *
 * Collects and reports gameplay telemetry:
 *   - Session events (start, end, crash)
 *   - Combat metrics (hit rate, jutsu usage, substitution frequency)
 *   - Progression metrics (time per area, quest completion rates)
 *   - Performance metrics (FPS, memory, hitches)
 *
 * All data is anonymized and batched before transmission.
 * No PII is ever collected.
 * Transmission only occurs with explicit player consent.
 */
UCLASS(NotBlueprintable)
class NARUTOUSL_API UAnalyticsManager : public UNarutoSubsystem
{
    GENERATED_BODY()

public:

    UAnalyticsManager();

    void Initialize(UNarutoEventBus* InEventBus);
    virtual void Tick(float DeltaTime) override;
    virtual void Shutdown() override;

    // ------------------------------------------------------------------
    //  Event Recording
    // ------------------------------------------------------------------

    UFUNCTION(BlueprintCallable, Category = "Analytics")
    void RecordEvent(const FString& EventName,
                     const TMap<FString, FString>& Parameters);

    UFUNCTION(BlueprintCallable, Category = "Analytics")
    void RecordCombatEvent(const FString& EventType, float Value);

    UFUNCTION(BlueprintCallable, Category = "Analytics")
    void RecordProgressionEvent(const FString& EventType, int32 Value);

    // ------------------------------------------------------------------
    //  Session
    // ------------------------------------------------------------------

    void StartSession();
    void EndSession();

    // ------------------------------------------------------------------
    //  Consent
    // ------------------------------------------------------------------

    UFUNCTION(BlueprintCallable, Category = "Analytics")
    void SetTelemetryConsent(bool bConsented);

    UFUNCTION(BlueprintPure, Category = "Analytics")
    bool HasTelemetryConsent() const { return bTelemetryConsented; }

private:

    void FlushEventQueue();
    void RecordPerformanceMetrics(float DeltaTime);

    struct FAnalyticsEvent
    {
        FString                  EventName;
        TMap<FString, FString>   Parameters;
        double                   Timestamp = 0.0;
    };

    TArray<FAnalyticsEvent>         EventQueue;
    TWeakObjectPtr<UNarutoEventBus> EventBus;

    bool   bTelemetryConsented = false;
    float  FlushInterval       = 60.0f;
    float  FlushTimer          = 0.0f;
    float  PerformanceTimer    = 0.0f;
    double SessionStartTime    = 0.0;

    // Performance rolling averages
    float AvgFPS       = 0.0f;
    float MinFPS       = 9999.0f;
    int32 HitchCount   = 0;
};
