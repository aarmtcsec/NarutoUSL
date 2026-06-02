// Copyright (c) 2025 Maze Runner Studios. All Rights Reserved.

#include "UI/Core/UIManager.h"
#include "Core/Events/NarutoEventBus.h"
#include "Blueprint/UserWidget.h"
#include "Kismet/GameplayStatics.h"
#include "NarutoUSL.h"

UUIManager::UUIManager()
{
    SubsystemName = TEXT("UIManager");
}

void UUIManager::Initialize(UNarutoEventBus* InEventBus)
{
    EventBus     = InEventBus;
    bInitialized = true;
    UE_LOG(LogNarutoUI, Log, TEXT("[UIManager] Initialized."));
}

void UUIManager::Shutdown()
{
    for (auto& Pair : WidgetInstances)
    {
        if (Pair.Value)
        {
            Pair.Value->RemoveFromParent();
        }
    }
    WidgetInstances.Empty();
    ScreenStack.Empty();
}

// ============================================================
//  Screen Management
// ============================================================

void UUIManager::PushScreen(EUIScreen Screen)
{
    // Hide current top
    if (ScreenStack.Num() > 0)
    {
        HideScreen(ScreenStack.Last());
    }

    ScreenStack.Add(Screen);
    ShowScreen(Screen);
}

void UUIManager::PopScreen()
{
    if (ScreenStack.IsEmpty()) return;

    HideScreen(ScreenStack.Last());
    ScreenStack.Pop();

    // Reveal new top
    if (ScreenStack.Num() > 0)
    {
        ShowScreen(ScreenStack.Last());
    }
}

void UUIManager::SetScreen(EUIScreen Screen)
{
    // Clear stack and set new screen
    for (EUIScreen S : ScreenStack)
    {
        HideScreen(S);
    }
    ScreenStack.Empty();
    ScreenStack.Add(Screen);
    ShowScreen(Screen);
}

EUIScreen UUIManager::GetActiveScreen() const
{
    return ScreenStack.IsEmpty() ? EUIScreen::None : ScreenStack.Last();
}

void UUIManager::SetCinematicMode(bool bCinematic)
{
    bCinematicMode = bCinematic;

    // Hide HUD during cinematics
    if (UUserWidget* HUD = WidgetInstances.FindRef(EUIScreen::HUD))
    {
        HUD->SetVisibility(bCinematic
            ? ESlateVisibility::Hidden
            : ESlateVisibility::Visible);
    }
}

void UUIManager::ShowScreen(EUIScreen Screen)
{
    UUserWidget* Widget = GetOrCreateWidget(Screen);
    if (Widget)
    {
        Widget->SetVisibility(ESlateVisibility::Visible);
    }
}

void UUIManager::HideScreen(EUIScreen Screen)
{
    UUserWidget* Widget = WidgetInstances.FindRef(Screen);
    if (Widget)
    {
        Widget->SetVisibility(ESlateVisibility::Hidden);
    }
}

// ============================================================
//  Damage Numbers
// ============================================================

void UUIManager::SpawnDamageNumber(float Amount, FVector WorldLocation,
                                    bool bIsCritical, bool bIsHealing)
{
    // Full implementation spawns a pooled 3D widget component at the hit location.
    // The widget animates upward and fades out.
    UE_LOG(LogNarutoUI, Verbose,
        TEXT("[UIManager] Damage number: %.0f%s at %s"),
        Amount,
        bIsCritical ? TEXT(" [CRIT]") : TEXT(""),
        *WorldLocation.ToString());
}

// ============================================================
//  Internal
// ============================================================

UUserWidget* UUIManager::GetOrCreateWidget(EUIScreen Screen)
{
    if (UUserWidget* Existing = WidgetInstances.FindRef(Screen))
    {
        return Existing;
    }

    const TSoftClassPtr<UUserWidget>* ClassPtr = ScreenClasses.Find(Screen);
    if (!ClassPtr || ClassPtr->IsNull()) return nullptr;

    TSubclassOf<UUserWidget> WidgetClass = ClassPtr->LoadSynchronous();
    if (!WidgetClass) return nullptr;

    APlayerController* PC = GetPlayerController();
    if (!PC) return nullptr;

    UUserWidget* NewWidget = CreateWidget<UUserWidget>(PC, WidgetClass);
    if (!NewWidget) return nullptr;

    const int32 ZOrder = (Screen == EUIScreen::HUD) ? 0 : 10;
    NewWidget->AddToViewport(ZOrder);
    NewWidget->SetVisibility(ESlateVisibility::Hidden);

    WidgetInstances.Add(Screen, NewWidget);
    return NewWidget;
}

APlayerController* UUIManager::GetPlayerController() const
{
    UWorld* World = GetOuter() ? GetOuter()->GetWorld() : nullptr;
    return World ? World->GetFirstPlayerController() : nullptr;
}
