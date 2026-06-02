// Copyright (c) 2025 Maze Runner Studios. All Rights Reserved.

#include "Character/Player/NarutoPlayerController.h"
#include "Character/Player/NarutoPlayerCharacter.h"
#include "Blueprint/UserWidget.h"
#include "NarutoUSL.h"

ANarutoPlayerController::ANarutoPlayerController()
{
    bShowMouseCursor = false;
}

void ANarutoPlayerController::BeginPlay()
{
    Super::BeginPlay();
    ShowHUD();
}

void ANarutoPlayerController::OnPossess(APawn* InPawn)
{
    Super::OnPossess(InPawn);
}

void ANarutoPlayerController::OnUnPossess()
{
    Super::OnUnPossess();
}

void ANarutoPlayerController::ShowHUD()
{
    if (!HUDWidget && !HUDWidgetClass.IsNull())
    {
        TSubclassOf<UUserWidget> Class = HUDWidgetClass.LoadSynchronous();
        if (Class)
        {
            HUDWidget = CreateWidget<UUserWidget>(this, Class);
            if (HUDWidget) HUDWidget->AddToViewport();
        }
    }
    else if (HUDWidget)
    {
        HUDWidget->SetVisibility(ESlateVisibility::Visible);
    }
}

void ANarutoPlayerController::HideHUD()
{
    if (HUDWidget) HUDWidget->SetVisibility(ESlateVisibility::Hidden);
}

void ANarutoPlayerController::PauseGame()
{
    if (bIsPaused) return;
    bIsPaused = true;
    SetPause(true);
    bShowMouseCursor = true;

    if (!PauseMenuWidget && !PauseMenuClass.IsNull())
    {
        TSubclassOf<UUserWidget> Class = PauseMenuClass.LoadSynchronous();
        if (Class)
        {
            PauseMenuWidget = CreateWidget<UUserWidget>(this, Class);
            if (PauseMenuWidget) PauseMenuWidget->AddToViewport(10);
        }
    }
    else if (PauseMenuWidget)
    {
        PauseMenuWidget->SetVisibility(ESlateVisibility::Visible);
    }
}

void ANarutoPlayerController::ResumeGame()
{
    if (!bIsPaused) return;
    bIsPaused = false;
    SetPause(false);
    bShowMouseCursor = false;

    if (PauseMenuWidget)
        PauseMenuWidget->SetVisibility(ESlateVisibility::Hidden);
}

ANarutoPlayerCharacter* ANarutoPlayerController::GetNarutoCharacter() const
{
    return Cast<ANarutoPlayerCharacter>(GetPawn());
}

void ANarutoPlayerController::HandlePlayerDefeated()
{
    HideHUD();
    // Death screen shown by GameMode after respawn delay
}
