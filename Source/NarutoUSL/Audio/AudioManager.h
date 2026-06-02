// Copyright (c) 2025 Maze Runner Studios. All Rights Reserved.
// Naruto: Ultimate Shinobi Legacy
// AudioManager — Dynamic music, 3D audio, voice, and ambience orchestration.

#pragma once

#include "CoreMinimal.h"
#include "Core/Subsystems/NarutoSubsystem.h"
#include "Core/Types/NarutoTypes.h"
#include "AudioManager.generated.h"

class UNarutoEventBus;
class UAudioComponent;

UENUM(BlueprintType)
enum class EMusicLayer : uint8
{
    Base        UMETA(DisplayName = "Base Layer"),
    Combat      UMETA(DisplayName = "Combat Layer"),
    Boss        UMETA(DisplayName = "Boss Layer"),
    Cinematic   UMETA(DisplayName = "Cinematic Layer"),
    Ambient     UMETA(DisplayName = "Ambient Layer"),
};

USTRUCT()
struct NARUTOUSL_API FMusicState
{
    GENERATED_BODY()

    TWeakObjectPtr<UAudioComponent> Component;
    TSoftObjectPtr<USoundBase>      CurrentTrack;
    float                           Volume      = 1.0f;
    float                           TargetVolume = 1.0f;
    bool                            bFading     = false;
    float                           FadeSpeed   = 2.0f;
};

/**
 * UAudioManager
 *
 * Manages all audio systems:
 *   - Dynamic layered music (base + combat + boss layers blend in/out)
 *   - Weather-reactive ambience
 *   - Combat state music transitions
 *   - Voice line queuing and priority
 *   - 3D positional audio routing
 *   - MetaSound parameter updates
 */
UCLASS(NotBlueprintable)
class NARUTOUSL_API UAudioManager : public UNarutoSubsystem
{
    GENERATED_BODY()

public:

    UAudioManager();

    void Initialize(UNarutoEventBus* InEventBus);
    virtual void Tick(float DeltaTime) override;
    virtual void Shutdown() override;

    // ------------------------------------------------------------------
    //  Music
    // ------------------------------------------------------------------

    UFUNCTION(BlueprintCallable, Category = "Audio|Music")
    void PlayMusic(USoundBase* Track, EMusicLayer Layer, float FadeInTime = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Audio|Music")
    void StopMusic(EMusicLayer Layer, float FadeOutTime = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Audio|Music")
    void SetMusicVolume(EMusicLayer Layer, float Volume, float FadeTime = 0.5f);

    // ------------------------------------------------------------------
    //  Combat / World Events
    // ------------------------------------------------------------------

    void OnCombatStateChanged(bool bInCombat);
    void OnWeatherChanged(EWeatherType NewWeather);
    void OnWorldChanged(UWorld* OldWorld, UWorld* NewWorld);

    // ------------------------------------------------------------------
    //  Voice
    // ------------------------------------------------------------------

    UFUNCTION(BlueprintCallable, Category = "Audio|Voice")
    void PlayVoiceLine(USoundBase* VoiceLine, AActor* Speaker, int32 Priority = 0);

    UFUNCTION(BlueprintCallable, Category = "Audio|Voice")
    void StopVoiceLine(AActor* Speaker);

    // ------------------------------------------------------------------
    //  SFX
    // ------------------------------------------------------------------

    UFUNCTION(BlueprintCallable, Category = "Audio|SFX")
    void PlaySFXAtLocation(USoundBase* Sound, FVector Location, float VolumeMultiplier = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Audio|SFX")
    void PlaySFXAttached(USoundBase* Sound, USceneComponent* AttachTo,
                         FName SocketName = NAME_None);

    // ------------------------------------------------------------------
    //  Master Volume
    // ------------------------------------------------------------------

    UFUNCTION(BlueprintCallable, Category = "Audio|Settings")
    void SetMasterVolume(float Volume);

    UFUNCTION(BlueprintCallable, Category = "Audio|Settings")
    void SetMusicVolumeSetting(float Volume);

    UFUNCTION(BlueprintCallable, Category = "Audio|Settings")
    void SetSFXVolumeSetting(float Volume);

    UFUNCTION(BlueprintCallable, Category = "Audio|Settings")
    void SetVoiceVolumeSetting(float Volume);

private:

    void TickMusicFades(float DeltaTime);

    TMap<EMusicLayer, FMusicState> MusicLayers;
    TWeakObjectPtr<UNarutoEventBus> EventBus;

    float MasterVolume = 1.0f;
    float MusicVolume  = 1.0f;
    float SFXVolume    = 1.0f;
    float VoiceVolume  = 1.0f;

    // Voice priority queue
    struct FVoiceRequest
    {
        TWeakObjectPtr<AActor>         Speaker;
        TWeakObjectPtr<UAudioComponent> Component;
        int32                          Priority = 0;
    };
    TArray<FVoiceRequest> ActiveVoiceLines;
};
