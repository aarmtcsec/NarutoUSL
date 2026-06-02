// Copyright (c) 2025 Maze Runner Studios. All Rights Reserved.
// Naruto: Ultimate Shinobi Legacy
// UIManager — Owns and controls all HUD widgets and screen transitions.

#pragma once

#include "CoreMinimal.h"
#include "Core/Subsystems/NarutoSubsystem.h"
#include "UIManager.generated.h"

class UNarutoEventBus;
class UUserWidget;

UENUM(BlueprintType)
enum class EUIScreen : uint8
{
    None        UMETA(DisplayName = "None"),
    HUD         UMETA(DisplayName = "HUD"),
    MainMenu    UMETA(DisplayName = "Main Menu"),
    PauseMenu   UMETA(DisplayName = "Pause Menu"),
    Inventory   UMETA(DisplayName = "Inventory"),
    Map         UMETA(DisplayName = "Map"),
    QuestLog    UMETA(DisplayName = "Quest Log"),
    SkillTree   UMETA(DisplayName = "Skill Tree"),
    CharScreen  UMETA(DisplayName = "Character Screen"),
    Codex       UMETA(DisplayName = "Codex"),
    Settings    UMETA(DisplayName = "Settings"),
    PhotoMode   UMETA(DisplayName = "Photo Mode"),
    LoadingScreen UMETA(DisplayName = "Loading Screen"),
    DeathScreen   UMETA(DisplayName = "Death Screen"),
};

/**
 * UUIManager
 *
 * Central UI orchestrator. Manages widget lifecycle, screen stack,
 * cinematic mode (hides HUD), and transition animations.
 * All widgets are created lazily on first request.
 */
UCLASS(NotBlueprintable)
class NARUTOUSL_API UUIManager : public UNarutoSubsystem
{
    GENERATED_BODY()

public:

    UUIManager();

    void Initialize(UNarutoEventBus* InEventBus);
    virtual void Shutdown() override;

    // ------------------------------------------------------------------
    //  Screen Management
    // ------------------------------------------------------------------

    UFUNCTION(BlueprintCallable, Category = "UI")
    void PushScreen(EUIScreen Screen);

    UFUNCTION(BlueprintCallable, Category = "UI")
    void PopScreen();

    UFUNCTION(BlueprintCallable, Category = "UI")
    void SetScreen(EUIScreen Screen);

    UFUNCTION(BlueprintPure, Category = "UI")
    EUIScreen GetActiveScreen() const;

    UFUNCTION(BlueprintCallable, Category = "UI")
    void SetCinematicMode(bool bCinematic);

    UFUNCTION(BlueprintPure, Category = "UI")
    bool IsInCinematicMode() const { return bCinematicMode; }

    UFUNCTION(BlueprintCallable, Category = "UI")
    void ShowScreen(EUIScreen Screen);

    UFUNCTION(BlueprintCallable, Category = "UI")
    void HideScreen(EUIScreen Screen);

    // ------------------------------------------------------------------
    //  Damage Numbers
    // ------------------------------------------------------------------

    UFUNCTION(BlueprintCallable, Category = "UI")
    void SpawnDamageNumber(float Amount, FVector WorldLocation,
                           bool bIsCritical, bool bIsHealing);

    // ------------------------------------------------------------------
    //  Widget Classes (set in GameInstance Blueprint)
    // ------------------------------------------------------------------

    UPROPERTY(EditAnywhere, Category = "UI|Classes")
    TMap<EUIScreen, TSoftClassPtr<UUserWidget>> ScreenClasses;

private:

    UUserWidget* GetOrCreateWidget(EUIScreen Screen);
    APlayerController* GetPlayerController() const;

    TMap<EUIScreen, TObjectPtr<UUserWidget>> WidgetInstances;
    TArray<EUIScreen>                        ScreenStack;
    TWeakObjectPtr<UNarutoEventBus>          EventBus;
    bool                                     bCinematicMode = false;
};
