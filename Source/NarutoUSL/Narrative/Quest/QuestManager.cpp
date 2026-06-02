// Copyright (c) 2025 Maze Runner Studios. All Rights Reserved.

#include "Narrative/Quest/QuestManager.h"
#include "Core/Events/NarutoEventBus.h"
#include "Narrative/Core/WorldStateManager.h"
#include "Narrative/Faction/FactionManager.h"
#include "NarutoUSL.h"

void UQuestManager::Initialize(UNarutoEventBus* InEventBus,
                                 UWorldStateManager* InWorldStateManager,
                                 UFactionManager* InFactionManager)
{
    EventBus          = InEventBus;
    WorldStateManager = InWorldStateManager;
    FactionManager    = InFactionManager;
    bInitialized      = true;
    UE_LOG(LogNarutoNarrative, Log, TEXT("[QuestManager] Initialized."));
}

void UQuestManager::Shutdown()
{
    QuestStates.Empty();
}

// ============================================================
//  Quest API
// ============================================================

bool UQuestManager::StartQuest(FGameplayTag QuestTag)
{
    if (!QuestTag.IsValid()) return false;

    FQuestRuntimeState& State = QuestStates.FindOrAdd(QuestTag);
    if (State.State == EQuestState::Active) return false;
    if (State.State == EQuestState::Completed) return false;

    State.QuestTag  = QuestTag;
    State.StartTime = 0.0f; // Set from world time in full implementation
    SetQuestState(QuestTag, EQuestState::Active);

    UE_LOG(LogNarutoNarrative, Log, TEXT("[QuestManager] Quest started: %s"),
        *QuestTag.ToString());
    return true;
}

void UQuestManager::CompleteQuest(FGameplayTag QuestTag, int32 BranchIndex)
{
    FQuestRuntimeState* State = QuestStates.Find(QuestTag);
    if (!State || State->State != EQuestState::Active) return;

    State->ChosenBranch  = BranchIndex;
    State->CompleteTime  = 0.0f;

    SetQuestState(QuestTag, EQuestState::Completed);

    UE_LOG(LogNarutoNarrative, Log, TEXT("[QuestManager] Quest completed: %s (branch %d)"),
        *QuestTag.ToString(), BranchIndex);
}

void UQuestManager::FailQuest(FGameplayTag QuestTag)
{
    SetQuestState(QuestTag, EQuestState::Failed);
}

void UQuestManager::AbandonQuest(FGameplayTag QuestTag)
{
    SetQuestState(QuestTag, EQuestState::Abandoned);
}

// ============================================================
//  Objectives
// ============================================================

void UQuestManager::AdvanceObjective(FGameplayTag QuestTag,
                                      FGameplayTag ObjectiveTag,
                                      int32 Amount)
{
    FQuestRuntimeState* State = QuestStates.Find(QuestTag);
    if (!State || !State->IsActive()) return;

    for (FQuestObjective& Obj : State->Objectives)
    {
        if (Obj.ObjectiveTag == ObjectiveTag)
        {
            Obj.CurrentCount = FMath::Min(Obj.CurrentCount + Amount, Obj.RequiredCount);
            if (Obj.CurrentCount >= Obj.RequiredCount)
            {
                Obj.bCompleted = true;
                UE_LOG(LogNarutoNarrative, Log,
                    TEXT("[QuestManager] Objective complete: %s / %s"),
                    *QuestTag.ToString(), *ObjectiveTag.ToString());
            }
            return;
        }
    }
}

void UQuestManager::CompleteObjective(FGameplayTag QuestTag, FGameplayTag ObjectiveTag)
{
    FQuestRuntimeState* State = QuestStates.Find(QuestTag);
    if (!State) return;

    for (FQuestObjective& Obj : State->Objectives)
    {
        if (Obj.ObjectiveTag == ObjectiveTag)
        {
            Obj.bCompleted   = true;
            Obj.CurrentCount = Obj.RequiredCount;
            return;
        }
    }
}

// ============================================================
//  Queries
// ============================================================

EQuestState UQuestManager::GetQuestState(FGameplayTag QuestTag) const
{
    const FQuestRuntimeState* State = QuestStates.Find(QuestTag);
    return State ? State->State : EQuestState::Locked;
}

bool UQuestManager::IsQuestActive(FGameplayTag QuestTag) const
{
    return GetQuestState(QuestTag) == EQuestState::Active;
}

bool UQuestManager::IsQuestComplete(FGameplayTag QuestTag) const
{
    return GetQuestState(QuestTag) == EQuestState::Completed;
}

TArray<FQuestRuntimeState> UQuestManager::GetActiveQuests() const
{
    TArray<FQuestRuntimeState> Result;
    for (const auto& Pair : QuestStates)
    {
        if (Pair.Value.IsActive()) Result.Add(Pair.Value);
    }
    return Result;
}

const FQuestRuntimeState* UQuestManager::GetQuestState_Full(FGameplayTag QuestTag) const
{
    return QuestStates.Find(QuestTag);
}

// ============================================================
//  Internal
// ============================================================

void UQuestManager::SetQuestState(FGameplayTag QuestTag, EQuestState NewState)
{
    FQuestRuntimeState& State = QuestStates.FindOrAdd(QuestTag);
    const EQuestState OldState = State.State;
    State.State = NewState;

    if (EventBus.IsValid())
    {
        FQuestStateChangedEvent Evt;
        Evt.QuestTag = QuestTag;
        Evt.OldState = OldState;
        Evt.NewState = NewState;
        EventBus->OnQuestStateChanged.Broadcast(Evt);
    }
}

void UQuestManager::DistributeRewards(const FQuestReward& Reward)
{
    // Full implementation routes rewards to ProgressionManager, EconomyManager, etc.
    UE_LOG(LogNarutoNarrative, Log,
        TEXT("[QuestManager] Distributing rewards: %.0f XP, %lld Ryo"),
        Reward.XPReward, Reward.RyoReward);

    // Set world flags
    if (WorldStateManager.IsValid())
    {
        for (const FGameplayTag& Flag : Reward.WorldFlagsToSet)
        {
            WorldStateManager->SetFlag(Flag, true);
        }
    }
}

void UQuestManager::SerializeToSave(FArchive& Ar)
{
    int32 Count = QuestStates.Num();
    Ar << Count;

    if (Ar.IsSaving())
    {
        for (auto& Pair : QuestStates)
        {
            FString TagStr = Pair.Key.ToString();
            uint8   StateVal = (uint8)Pair.Value.State;
            Ar << TagStr;
            Ar << StateVal;
        }
    }
    else
    {
        QuestStates.Empty();
        for (int32 i = 0; i < Count; ++i)
        {
            FString TagStr;
            uint8   StateVal = 0;
            Ar << TagStr;
            Ar << StateVal;

            FGameplayTag Tag = FGameplayTag::RequestGameplayTag(FName(*TagStr), false);
            if (Tag.IsValid())
            {
                FQuestRuntimeState& State = QuestStates.FindOrAdd(Tag);
                State.QuestTag = Tag;
                State.State    = (EQuestState)StateVal;
            }
        }
    }
}

void UQuestManager::DeserializeFromSave(FArchive& Ar)
{
    SerializeToSave(Ar);
}

float FQuestRuntimeState::GetOverallProgress() const
{
    if (Objectives.IsEmpty()) return IsComplete() ? 1.0f : 0.0f;

    float Total = 0.0f;
    int32 Required = 0;
    for (const FQuestObjective& Obj : Objectives)
    {
        if (!Obj.bOptional)
        {
            Total += Obj.GetProgress();
            ++Required;
        }
    }
    return Required > 0 ? Total / Required : 1.0f;
}
