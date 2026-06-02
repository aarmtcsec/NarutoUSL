// Copyright (c) 2025 Maze Runner Studios. All Rights Reserved.

#include "Core/GameMode/NarutoGameMode.h"
#include "Core/GameInstance/NarutoGameInstance.h"
#include "Character/Player/NarutoPlayerController.h"
#include "Character/Player/NarutoPlayerCharacter.h"
#include "GameFramework/PlayerStart.h"
#include "EngineUtils.h"
#include "TimerManager.h"

ANarutoGameMode::ANarutoGameMode(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 0.0f;
}

void ANarutoGameMode::InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage)
{
    Super::InitGame(MapName, Options, ErrorMessage);

    UE_LOG(LogTemp, Log, TEXT("[NarutoGameMode] InitGame — Map: %s"), *MapName);
}

void ANarutoGameMode::StartPlay()
{
    Super::StartPlay();

    UNarutoGameInstance* GI = UNarutoGameInstance::Get(this);
    if (GI && GI->GetCombatManager())
    {
        GI->GetCombatManager()->SetDifficultyScale(GlobalDifficultyScale);
    }
}

void ANarutoGameMode::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);
}

void ANarutoGameMode::HandleStartingNewPlayer_Implementation(APlayerController* NewPlayer)
{
    Super::HandleStartingNewPlayer_Implementation(NewPlayer);
}

AActor* ANarutoGameMode::FindPlayerStart_Implementation(AController* Player, const FString& IncomingName)
{
    // Prefer a PlayerStart tagged with the current session type.
    // Falls back to the default UE5 logic if none found.
    const FName SessionTag = *UEnum::GetValueAsString(CurrentSessionType);

    for (TActorIterator<APlayerStart> It(GetWorld()); It; ++It)
    {
        APlayerStart* Start = *It;
        if (Start && Start->PlayerStartTag == SessionTag)
        {
            return Start;
        }
    }

    return Super::FindPlayerStart_Implementation(Player, IncomingName);
}

void ANarutoGameMode::RestartPlayer(AController* NewPlayer)
{
    Super::RestartPlayer(NewPlayer);
}

void ANarutoGameMode::SetSessionType(EGameSessionType NewType)
{
    if (CurrentSessionType == NewType) return;

    CurrentSessionType = NewType;

    UE_LOG(LogTemp, Log, TEXT("[NarutoGameMode] Session type changed to: %s"),
        *UEnum::GetValueAsString(NewType));
}

void ANarutoGameMode::HandlePlayerDefeated(ANarutoPlayerController* PlayerController)
{
    if (!PlayerController) return;

    const float Delay = GetRespawnDelay();

    UE_LOG(LogTemp, Log, TEXT("[NarutoGameMode] Player defeated. Respawning in %.1f seconds."), Delay);

    GetWorldTimerManager().SetTimer(
        RespawnTimerHandle,
        FTimerDelegate::CreateUObject(this, &ANarutoGameMode::ExecuteRespawn, PlayerController),
        Delay,
        false
    );
}

float ANarutoGameMode::GetRespawnDelay() const
{
    switch (CurrentSessionType)
    {
        case EGameSessionType::BossEncounter: return BossEncounterRespawnDelay;
        case EGameSessionType::ArenaFight:    return ArenaRespawnDelay;
        default:                              return OpenWorldRespawnDelay;
    }
}

void ANarutoGameMode::SetGlobalDifficultyScale(float Scale)
{
    GlobalDifficultyScale = FMath::Clamp(Scale, 0.1f, 5.0f);

    UNarutoGameInstance* GI = UNarutoGameInstance::Get(this);
    if (GI && GI->GetCombatManager())
    {
        GI->GetCombatManager()->SetDifficultyScale(GlobalDifficultyScale);
    }
}

void ANarutoGameMode::ExecuteRespawn(ANarutoPlayerController* PlayerController)
{
    if (!PlayerController) return;

    RestartPlayer(PlayerController);

    UE_LOG(LogTemp, Log, TEXT("[NarutoGameMode] Player respawned."));
}
