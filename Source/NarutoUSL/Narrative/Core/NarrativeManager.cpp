// Copyright (c) 2025 Maze Runner Studios. All Rights Reserved.

#include "Narrative/Core/NarrativeManager.h"
#include "Narrative/Quest/QuestManager.h"
#include "Narrative/Core/WorldStateManager.h"
#include "NarutoUSL.h"

void UNarrativeManager::Initialize(UQuestManager* InQuestManager,
                                     UWorldStateManager* InWorldStateManager)
{
    QuestManager      = InQuestManager;
    WorldStateManager = InWorldStateManager;
    bInitialized      = true;
    UE_LOG(LogNarutoNarrative, Log, TEXT("[NarrativeManager] Initialized."));
}

void UNarrativeManager::Shutdown()
{
    EndDialogue();
}

bool UNarrativeManager::StartDialogue(UDialogueTree* Tree, AActor* Speaker, AActor* Listener)
{
    if (!Tree || bInDialogue) return false;

    ActiveTree     = Tree;
    ActiveSpeaker  = Speaker;
    ActiveListener = Listener;
    bInDialogue    = true;

    const FDialogueNode* Entry = Tree->GetEntryNode();
    if (!Entry)
    {
        EndDialogue();
        return false;
    }

    ActivateNode(Entry);
    return true;
}

void UNarrativeManager::SelectDialogueChoice(int32 ChoiceIndex)
{
    if (!bInDialogue || !ActiveTree.IsValid()) return;

    const FDialogueNode* Current = GetCurrentNode();
    if (!Current) return;

    TArray<const FDialogueNode*> Choices = GetAvailableChoices();
    if (!Choices.IsValidIndex(ChoiceIndex)) return;

    ApplyConsequences(*Current);
    ActivateNode(Choices[ChoiceIndex]);
}

void UNarrativeManager::AdvanceDialogue()
{
    if (!bInDialogue || !ActiveTree.IsValid()) return;

    const FDialogueNode* Current = GetCurrentNode();
    if (!Current || Current->bIsPlayerChoice) return;

    ApplyConsequences(*Current);

    if (Current->NextNodes.IsEmpty())
    {
        EndDialogue();
        return;
    }

    // Auto-advance to first valid next node
    for (const FGameplayTag& NextTag : Current->NextNodes)
    {
        const FDialogueNode* Next = ActiveTree->GetNode(NextTag);
        if (Next)
        {
            ActivateNode(Next);
            return;
        }
    }

    EndDialogue();
}

void UNarrativeManager::EndDialogue()
{
    if (!bInDialogue) return;

    bInDialogue    = false;
    CurrentNodeTag = FGameplayTag();
    ActiveTree     = nullptr;
    ActiveSpeaker  = nullptr;
    ActiveListener = nullptr;

    OnDialogueEnded.Broadcast();
}

const FDialogueNode* UNarrativeManager::GetCurrentNode() const
{
    if (!ActiveTree.IsValid() || !CurrentNodeTag.IsValid()) return nullptr;
    return ActiveTree->GetNode(CurrentNodeTag);
}

void UNarrativeManager::SetCurrentChapter(int32 ChapterIndex)
{
    CurrentChapter = FMath::Max(0, ChapterIndex);
    UE_LOG(LogNarutoNarrative, Log,
        TEXT("[NarrativeManager] Chapter set to %d."), CurrentChapter);
}

void UNarrativeManager::ActivateNode(const FDialogueNode* Node)
{
    if (!Node) return;

    CurrentNodeTag = Node->NodeTag;
    OnDialogueNodeActivated.Broadcast(*Node, 0);

    UE_LOG(LogNarutoNarrative, Verbose,
        TEXT("[NarrativeManager] Dialogue node: %s — %s"),
        *Node->SpeakerName.ToString(), *Node->DialogueText.ToString());
}

TArray<const FDialogueNode*> UNarrativeManager::GetAvailableChoices() const
{
    TArray<const FDialogueNode*> Choices;
    const FDialogueNode* Current = GetCurrentNode();
    if (!Current || !ActiveTree.IsValid()) return Choices;

    for (const FGameplayTag& NextTag : Current->NextNodes)
    {
        const FDialogueNode* Next = ActiveTree->GetNode(NextTag);
        if (!Next) continue;

        // Check condition tags
        if (WorldStateManager.IsValid() &&
            !Next->ConditionTags.IsEmpty() &&
            !WorldStateManager->HasAllFlags(Next->ConditionTags))
        {
            continue;
        }

        Choices.Add(Next);
    }

    return Choices;
}

void UNarrativeManager::ApplyConsequences(const FDialogueNode& Node)
{
    if (WorldStateManager.IsValid() && !Node.ConsequenceTags.IsEmpty())
    {
        WorldStateManager->SetFlags(Node.ConsequenceTags, true);
    }
}
