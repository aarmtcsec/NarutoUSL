// Copyright (c) 2025 Maze Runner Studios. All Rights Reserved.

#include "Audio/AudioManager.h"
#include "Core/Events/NarutoEventBus.h"
#include "Kismet/GameplayStatics.h"
#include "Components/AudioComponent.h"
#include "NarutoUSL.h"

UAudioManager::UAudioManager()
{
    SubsystemName = TEXT("AudioManager");
    bTickEnabled  = true;
    TickPriority  = ESubsystemTickPriority::PostRender;
}

void UAudioManager::Initialize(UNarutoEventBus* InEventBus)
{
    EventBus = InEventBus;

    if (EventBus.IsValid())
    {
        EventBus->OnWeatherChanged.AddUObject(this, &UAudioManager::OnWeatherChanged);
    }

    bInitialized = true;
    UE_LOG(LogNarutoAudio, Log, TEXT("[AudioManager] Initialized."));
}

void UAudioManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    TickMusicFades(DeltaTime);
}

void UAudioManager::Shutdown()
{
    for (auto& Pair : MusicLayers)
    {
        if (Pair.Value.Component.IsValid())
        {
            Pair.Value.Component->Stop();
        }
    }
    MusicLayers.Empty();
    ActiveVoiceLines.Empty();
}

// ============================================================
//  Music
// ============================================================

void UAudioManager::PlayMusic(USoundBase* Track, EMusicLayer Layer, float FadeInTime)
{
    if (!Track) return;

    FMusicState& State = MusicLayers.FindOrAdd(Layer);

    // Stop existing track on this layer
    if (State.Component.IsValid())
    {
        State.Component->FadeOut(FadeInTime * 0.5f, 0.0f);
    }

    // Create new audio component
    UWorld* World = GetOuter() ? GetOuter()->GetWorld() : nullptr;
    if (!World) return;

    UAudioComponent* NewComp = UGameplayStatics::SpawnSound2D(World, Track);
    if (!NewComp) return;

    NewComp->SetVolumeMultiplier(0.0f);
    NewComp->FadeIn(FadeInTime, MusicVolume * MasterVolume);

    State.Component    = NewComp;
    State.CurrentTrack = Track;
    State.Volume       = MusicVolume * MasterVolume;
    State.TargetVolume = MusicVolume * MasterVolume;
    State.bFading      = false;

    UE_LOG(LogNarutoAudio, Log, TEXT("[AudioManager] Playing music on layer %s: %s"),
        *UEnum::GetValueAsString(Layer), *Track->GetName());
}

void UAudioManager::StopMusic(EMusicLayer Layer, float FadeOutTime)
{
    FMusicState* State = MusicLayers.Find(Layer);
    if (!State || !State->Component.IsValid()) return;

    State->Component->FadeOut(FadeOutTime, 0.0f);
    State->TargetVolume = 0.0f;
    State->bFading      = true;
    State->FadeSpeed    = 1.0f / FMath::Max(0.01f, FadeOutTime);
}

void UAudioManager::SetMusicVolume(EMusicLayer Layer, float Volume, float FadeTime)
{
    FMusicState* State = MusicLayers.Find(Layer);
    if (!State) return;

    State->TargetVolume = FMath::Clamp(Volume, 0.0f, 1.0f) * MusicVolume * MasterVolume;
    State->bFading      = true;
    State->FadeSpeed    = FadeTime > 0.0f ? 1.0f / FadeTime : 100.0f;
}

// ============================================================
//  Combat / World Events
// ============================================================

void UAudioManager::OnCombatStateChanged(bool bInCombat)
{
    if (bInCombat)
    {
        SetMusicVolume(EMusicLayer::Base,   0.3f, 1.0f);
        SetMusicVolume(EMusicLayer::Combat, 1.0f, 1.0f);
    }
    else
    {
        SetMusicVolume(EMusicLayer::Combat, 0.0f, 2.0f);
        SetMusicVolume(EMusicLayer::Base,   1.0f, 2.0f);
    }
}

void UAudioManager::OnWeatherChanged(EWeatherType NewWeather)
{
    // Adjust ambient layer based on weather
    // Full implementation loads weather-specific ambience tracks
    UE_LOG(LogNarutoAudio, Verbose,
        TEXT("[AudioManager] Weather changed to %s — updating ambience."),
        *UEnum::GetValueAsString(NewWeather));
}

void UAudioManager::OnWorldChanged(UWorld* OldWorld, UWorld* NewWorld)
{
    // Stop all music on world transition
    for (auto& Pair : MusicLayers)
    {
        if (Pair.Value.Component.IsValid())
        {
            Pair.Value.Component->Stop();
        }
    }
    MusicLayers.Empty();
}

// ============================================================
//  Voice
// ============================================================

void UAudioManager::PlayVoiceLine(USoundBase* VoiceLine, AActor* Speaker, int32 Priority)
{
    if (!VoiceLine || !Speaker) return;

    // Stop lower-priority voice lines from the same speaker
    for (int32 i = ActiveVoiceLines.Num() - 1; i >= 0; --i)
    {
        if (ActiveVoiceLines[i].Speaker.Get() == Speaker)
        {
            if (ActiveVoiceLines[i].Priority <= Priority)
            {
                if (ActiveVoiceLines[i].Component.IsValid())
                    ActiveVoiceLines[i].Component->Stop();
                ActiveVoiceLines.RemoveAt(i);
            }
            else
            {
                return; // Higher priority already playing
            }
        }
    }

    UWorld* World = Speaker->GetWorld();
    if (!World) return;

    UAudioComponent* Comp = UGameplayStatics::SpawnSoundAttached(
        VoiceLine, Speaker->GetRootComponent());

    if (Comp)
    {
        Comp->SetVolumeMultiplier(VoiceVolume * MasterVolume);
        FVoiceRequest Request;
        Request.Speaker   = Speaker;
        Request.Component = Comp;
        Request.Priority  = Priority;
        ActiveVoiceLines.Add(Request);
    }
}

void UAudioManager::StopVoiceLine(AActor* Speaker)
{
    for (int32 i = ActiveVoiceLines.Num() - 1; i >= 0; --i)
    {
        if (ActiveVoiceLines[i].Speaker.Get() == Speaker)
        {
            if (ActiveVoiceLines[i].Component.IsValid())
                ActiveVoiceLines[i].Component->Stop();
            ActiveVoiceLines.RemoveAt(i);
        }
    }
}

// ============================================================
//  SFX
// ============================================================

void UAudioManager::PlaySFXAtLocation(USoundBase* Sound, FVector Location,
                                        float VolumeMultiplier)
{
    if (!Sound) return;
    UWorld* World = GetOuter() ? GetOuter()->GetWorld() : nullptr;
    if (!World) return;

    UGameplayStatics::PlaySoundAtLocation(World, Sound, Location,
        VolumeMultiplier * SFXVolume * MasterVolume);
}

void UAudioManager::PlaySFXAttached(USoundBase* Sound, USceneComponent* AttachTo,
                                      FName SocketName)
{
    if (!Sound || !AttachTo) return;
    UAudioComponent* Comp = UGameplayStatics::SpawnSoundAttached(
        Sound, AttachTo, SocketName);
    if (Comp) Comp->SetVolumeMultiplier(SFXVolume * MasterVolume);
}

// ============================================================
//  Volume Settings
// ============================================================

void UAudioManager::SetMasterVolume(float Volume)
{
    MasterVolume = FMath::Clamp(Volume, 0.0f, 1.0f);
}

void UAudioManager::SetMusicVolumeSetting(float Volume)
{
    MusicVolume = FMath::Clamp(Volume, 0.0f, 1.0f);
}

void UAudioManager::SetSFXVolumeSetting(float Volume)
{
    SFXVolume = FMath::Clamp(Volume, 0.0f, 1.0f);
}

void UAudioManager::SetVoiceVolumeSetting(float Volume)
{
    VoiceVolume = FMath::Clamp(Volume, 0.0f, 1.0f);
}

// ============================================================
//  Internal
// ============================================================

void UAudioManager::TickMusicFades(float DeltaTime)
{
    for (auto& Pair : MusicLayers)
    {
        FMusicState& State = Pair.Value;
        if (!State.bFading || !State.Component.IsValid()) continue;

        State.Volume = FMath::FInterpConstantTo(
            State.Volume, State.TargetVolume, DeltaTime, State.FadeSpeed);

        State.Component->SetVolumeMultiplier(State.Volume);

        if (FMath::IsNearlyEqual(State.Volume, State.TargetVolume, 0.001f))
        {
            State.Volume  = State.TargetVolume;
            State.bFading = false;

            if (State.TargetVolume <= 0.0f)
            {
                State.Component->Stop();
            }
        }
    }

    // Clean up finished voice lines
    ActiveVoiceLines.RemoveAll([](const FVoiceRequest& R)
    {
        return !R.Component.IsValid() || !R.Component->IsPlaying();
    });
}
