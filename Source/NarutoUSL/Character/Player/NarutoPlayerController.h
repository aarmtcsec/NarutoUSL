// Copyright (c) 2025 Maze Runner Studios. All Rights Reserved.
// Naruto: Ultimate Shinobi Legacy
// NarutoPlayerController — Manages HUD, pause, and player state persistence.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "NarutoPlayerController.generated.h"

class ANarutoPlayerCharacter;

UCLASS()
class NARUTOUSL_API ANarutoPlayerController : public APlayerController
{
    GENERATED_BODY()

public:

    ANarutoPlayerController();

    virtual void BeginPlay() override;
    virtual void OnPossess(APawn* InPawn) override;
    virtual void OnUnPossess() override;

    UFUNCTION(BlueprintCallable, Category = "UI")
    void ShowHUD();

    UFUNCTION(BlueprintCallable, Category = "UI")
    void HideHUD();

    UFUNCTION(BlueprintCallable, Category = "Game")
    void PauseGame();

    UFUNCTION(BlueprintCallable, Category = "Game")
    void ResumeGame();

    UFUNCTION(BlueprintPure, Category = "Character")
    ANarutoPlayerCharacter* GetNarutoCharacter() const;

    /** Called by GameMode when the player is defeated. */
    void HandlePlayerDefeated();

protected:

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UI")
    TSoftClassPtr<UUserWidget> HUDWidgetClass;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UI")
    TSoftClassPtr<UUserWidget> PauseMenuClass;

    UPROPERTY()
    TObjectPtr<UUserWidget> HUDWidget;

    UPROPERTY()
    TObjectPtr<UUserWidget> PauseMenuWidget;

    bool bIsPaused = false;
};
