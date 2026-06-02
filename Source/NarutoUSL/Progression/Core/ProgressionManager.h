// Copyright (c) 2025 Maze Runner Studios. All Rights Reserved.
// Naruto: Ultimate Shinobi Legacy
// ProgressionManager — Global progression subsystem. Coordinates XP grants,
// level-up broadcasts, and skill tree unlocks across all characters.

#pragma once

#include "CoreMinimal.h"
#include "Core/Subsystems/NarutoSubsystem.h"
#include "Core/Types/NarutoTypes.h"
#include "GameplayTagContainer.h"
#include "ProgressionManager.generated.h"

class UNarutoEventBus;
class UFactionManager;
class ANarutoPlayerCharacter;

// ============================================================
//  Skill Tree Node
// ============================================================

USTRUCT(BlueprintType)
struct NARUTOUSL_API FSkillTreeNode
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadOnly) FGameplayTag  NodeTag;
    UPROPERTY(EditAnywhere, BlueprintReadOnly) FText         DisplayName;
    UPROPERTY(EditAnywhere, BlueprintReadOnly) FText         Description;
    UPROPERTY(EditAnywhere, BlueprintReadOnly) int32         SkillPointCost = 1;
    UPROPERTY(EditAnywhere, BlueprintReadOnly) int32         RequiredLevel  = 1;
    UPROPERTY(EditAnywhere, BlueprintReadOnly) FGameplayTagContainer Prerequisites;
    UPROPERTY(EditAnywhere, BlueprintReadOnly) TArray<FStatModifier> GrantedModifiers;
    UPROPERTY(EditAnywhere, BlueprintReadOnly) TArray<FGameplayTag>  UnlockedJutsuTags;
    UPROPERTY(EditAnywhere, BlueprintReadOnly) TSoftObjectPtr<UTexture2D> Icon;
    UPROPERTY(BlueprintReadOnly) bool bUnlocked = false;
};

// ============================================================
//  ProgressionManager
// ============================================================

/**
 * UProgressionManager
 *
 * Global subsystem that:
 *   - Receives XP grant requests and routes them to the active player character
 *   - Manages the skill tree state (unlocked nodes, available points)
 *   - Validates skill tree unlock prerequisites
 *   - Broadcasts level-up events to the EventBus
 *   - Tracks total playtime
 */
UCLASS(NotBlueprintable)
class NARUTOUSL_API UProgressionManager : public UNarutoSubsystem
{
    GENERATED_BODY()

public:

    UProgressionManager();

    void Initialize(UNarutoEventBus* InEventBus, UFactionManager* InFactionManager);
    virtual void Tick(float DeltaTime) override;
    virtual void Shutdown() override;
    virtual void SerializeToSave(FArchive& Ar) override;
    virtual void DeserializeFromSave(FArchive& Ar) override;

    // ------------------------------------------------------------------
    //  XP
    // ------------------------------------------------------------------

    UFUNCTION(BlueprintCallable, Category = "Progression")
    void GrantXP(float Amount, FGameplayTag SourceTag);

    // ------------------------------------------------------------------
    //  Skill Tree
    // ------------------------------------------------------------------

    UFUNCTION(BlueprintCallable, Category = "Progression|SkillTree")
    bool UnlockSkillNode(FGameplayTag NodeTag);

    UFUNCTION(BlueprintPure, Category = "Progression|SkillTree")
    bool IsNodeUnlocked(FGameplayTag NodeTag) const;

    UFUNCTION(BlueprintPure, Category = "Progression|SkillTree")
    bool CanUnlockNode(FGameplayTag NodeTag) const;

    UFUNCTION(BlueprintPure, Category = "Progression|SkillTree")
    const FSkillTreeNode* GetNodeData(FGameplayTag NodeTag) const;

    void RegisterSkillTreeNodes(const TArray<FSkillTreeNode>& Nodes);

    // ------------------------------------------------------------------
    //  Playtime
    // ------------------------------------------------------------------

    UFUNCTION(BlueprintPure, Category = "Progression")
    float GetTotalPlaytimeSeconds() const { return TotalPlaytimeSeconds; }

    UFUNCTION(BlueprintPure, Category = "Progression")
    FString GetFormattedPlaytime() const;

private:

    void SetActivePlayer(ANarutoPlayerCharacter* Player);

    TMap<FGameplayTag, FSkillTreeNode> SkillTreeNodes;
    TSet<FGameplayTag>                 UnlockedNodes;

    TWeakObjectPtr<UNarutoEventBus>        EventBus;
    TWeakObjectPtr<UFactionManager>        FactionManager;
    TWeakObjectPtr<ANarutoPlayerCharacter> ActivePlayer;

    float TotalPlaytimeSeconds = 0.0f;
};
