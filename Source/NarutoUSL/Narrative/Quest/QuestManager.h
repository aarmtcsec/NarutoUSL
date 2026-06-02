// Copyright (c) 2025 Maze Runner Studios. All Rights Reserved.
// Naruto: Ultimate Shinobi Legacy
// QuestManager — Tracks all quest states, objectives, and rewards.

#pragma once

#include "CoreMinimal.h"
#include "Core/Subsystems/NarutoSubsystem.h"
#include "Core/Types/NarutoTypes.h"
#include "GameplayTagContainer.h"
#include "QuestManager.generated.h"

class UNarutoEventBus;
class UWorldStateManager;
class UFactionManager;

// ============================================================
//  Quest Objective
// ============================================================

USTRUCT(BlueprintType)
struct NARUTOUSL_API FQuestObjective
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadOnly) FGameplayTag ObjectiveTag;
    UPROPERTY(EditAnywhere, BlueprintReadOnly) FText        Description;
    UPROPERTY(EditAnywhere, BlueprintReadOnly) int32        RequiredCount  = 1;
    UPROPERTY(BlueprintReadOnly)               int32        CurrentCount   = 0;
    UPROPERTY(BlueprintReadOnly)               bool         bCompleted     = false;
    UPROPERTY(EditAnywhere, BlueprintReadOnly) bool         bOptional      = false;
    UPROPERTY(EditAnywhere, BlueprintReadOnly) FGameplayTagContainer RequiredWorldFlags;

    bool IsComplete() const { return bCompleted || CurrentCount >= RequiredCount; }
    float GetProgress() const
    {
        return RequiredCount > 0
            ? FMath::Clamp((float)CurrentCount / RequiredCount, 0.0f, 1.0f)
            : (bCompleted ? 1.0f : 0.0f);
    }
};

// ============================================================
//  Quest Reward
// ============================================================

USTRUCT(BlueprintType)
struct NARUTOUSL_API FQuestReward
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadOnly) float        XPReward       = 0.0f;
    UPROPERTY(EditAnywhere, BlueprintReadOnly) int64        RyoReward      = 0;
    UPROPERTY(EditAnywhere, BlueprintReadOnly) TArray<FGameplayTag> ItemRewards;
    UPROPERTY(EditAnywhere, BlueprintReadOnly) TArray<FGameplayTag> JutsuRewards;
    UPROPERTY(EditAnywhere, BlueprintReadOnly) TArray<FGameplayTag> TitleRewards;
    UPROPERTY(EditAnywhere, BlueprintReadOnly) EVillage     ReputationFaction = EVillage::None;
    UPROPERTY(EditAnywhere, BlueprintReadOnly) float        ReputationAmount  = 0.0f;
    UPROPERTY(EditAnywhere, BlueprintReadOnly) TArray<FGameplayTag> WorldFlagsToSet;
};

// ============================================================
//  Quest Runtime State
// ============================================================

USTRUCT(BlueprintType)
struct NARUTOUSL_API FQuestRuntimeState
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly) FGameplayTag          QuestTag;
    UPROPERTY(BlueprintReadOnly) EQuestState           State       = EQuestState::Locked;
    UPROPERTY(BlueprintReadOnly) TArray<FQuestObjective> Objectives;
    UPROPERTY(BlueprintReadOnly) float                 StartTime   = 0.0f;
    UPROPERTY(BlueprintReadOnly) float                 CompleteTime = 0.0f;
    UPROPERTY(BlueprintReadOnly) int32                 ChosenBranch = -1;

    bool IsActive()    const { return State == EQuestState::Active; }
    bool IsComplete()  const { return State == EQuestState::Completed; }
    float GetOverallProgress() const;
};

// ============================================================
//  QuestManager
// ============================================================

/**
 * UQuestManager
 *
 * Tracks all quest states. Handles:
 *   - Quest activation (when prerequisites are met)
 *   - Objective progress tracking
 *   - Branch selection (player choices affect outcomes)
 *   - Reward distribution on completion
 *   - World state flag updates on completion
 *   - Reputation changes on completion
 */
UCLASS(NotBlueprintable)
class NARUTOUSL_API UQuestManager : public UNarutoSubsystem
{
    GENERATED_BODY()

public:

    void Initialize(UNarutoEventBus* InEventBus,
                    UWorldStateManager* InWorldStateManager,
                    UFactionManager* InFactionManager);

    virtual void Shutdown() override;

    // ------------------------------------------------------------------
    //  Quest API
    // ------------------------------------------------------------------

    UFUNCTION(BlueprintCallable, Category = "Quest")
    bool StartQuest(FGameplayTag QuestTag);

    UFUNCTION(BlueprintCallable, Category = "Quest")
    void CompleteQuest(FGameplayTag QuestTag, int32 BranchIndex = -1);

    UFUNCTION(BlueprintCallable, Category = "Quest")
    void FailQuest(FGameplayTag QuestTag);

    UFUNCTION(BlueprintCallable, Category = "Quest")
    void AbandonQuest(FGameplayTag QuestTag);

    // ------------------------------------------------------------------
    //  Objective Progress
    // ------------------------------------------------------------------

    UFUNCTION(BlueprintCallable, Category = "Quest")
    void AdvanceObjective(FGameplayTag QuestTag, FGameplayTag ObjectiveTag, int32 Amount = 1);

    UFUNCTION(BlueprintCallable, Category = "Quest")
    void CompleteObjective(FGameplayTag QuestTag, FGameplayTag ObjectiveTag);

    // ------------------------------------------------------------------
    //  Queries
    // ------------------------------------------------------------------

    UFUNCTION(BlueprintPure, Category = "Quest")
    EQuestState GetQuestState(FGameplayTag QuestTag) const;

    UFUNCTION(BlueprintPure, Category = "Quest")
    bool IsQuestActive(FGameplayTag QuestTag) const;

    UFUNCTION(BlueprintPure, Category = "Quest")
    bool IsQuestComplete(FGameplayTag QuestTag) const;

    UFUNCTION(BlueprintPure, Category = "Quest")
    TArray<FQuestRuntimeState> GetActiveQuests() const;

    UFUNCTION(BlueprintPure, Category = "Quest")
    const FQuestRuntimeState* GetQuestState_Full(FGameplayTag QuestTag) const;

    // ------------------------------------------------------------------
    //  Serialization
    // ------------------------------------------------------------------

    virtual void SerializeToSave(FArchive& Ar) override;
    virtual void DeserializeFromSave(FArchive& Ar) override;

private:

    void DistributeRewards(const FQuestReward& Reward);
    void SetQuestState(FGameplayTag QuestTag, EQuestState NewState);

    TMap<FGameplayTag, FQuestRuntimeState> QuestStates;

    TWeakObjectPtr<UNarutoEventBus>     EventBus;
    TWeakObjectPtr<UWorldStateManager>  WorldStateManager;
    TWeakObjectPtr<UFactionManager>     FactionManager;
};
