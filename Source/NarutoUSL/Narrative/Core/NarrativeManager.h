// Copyright (c) 2025 Maze Runner Studios. All Rights Reserved.
// Naruto: Ultimate Shinobi Legacy
// NarrativeManager — Drives story progression, dialogue, and cinematic triggers.

#pragma once

#include "CoreMinimal.h"
#include "Core/Subsystems/NarutoSubsystem.h"
#include "GameplayTagContainer.h"
#include "NarrativeManager.generated.h"

class UQuestManager;
class UWorldStateManager;

// ============================================================
//  Dialogue Node
// ============================================================

USTRUCT(BlueprintType)
struct NARUTOUSL_API FDialogueNode
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadOnly) FGameplayTag  NodeTag;
    UPROPERTY(EditAnywhere, BlueprintReadOnly) FText         SpeakerName;
    UPROPERTY(EditAnywhere, BlueprintReadOnly) FText         DialogueText;
    UPROPERTY(EditAnywhere, BlueprintReadOnly) TSoftObjectPtr<USoundBase> VoiceLine;
    UPROPERTY(EditAnywhere, BlueprintReadOnly) TSoftObjectPtr<UAnimMontage> SpeakerMontage;

    /** Condition tags that must be active for this node to be available. */
    UPROPERTY(EditAnywhere, BlueprintReadOnly) FGameplayTagContainer ConditionTags;

    /** Tags set in WorldStateManager when this node is selected. */
    UPROPERTY(EditAnywhere, BlueprintReadOnly) FGameplayTagContainer ConsequenceTags;

    /** Next node tags (branching). Empty = end of dialogue. */
    UPROPERTY(EditAnywhere, BlueprintReadOnly) TArray<FGameplayTag> NextNodes;

    /** Whether this node is a player choice. */
    UPROPERTY(EditAnywhere, BlueprintReadOnly) bool bIsPlayerChoice = false;
};

// ============================================================
//  Dialogue Tree
// ============================================================

UCLASS(BlueprintType)
class NARUTOUSL_API UDialogueTree : public UPrimaryDataAsset
{
    GENERATED_BODY()
public:
    virtual FPrimaryAssetId GetPrimaryAssetId() const override
    {
        return FPrimaryAssetId(TEXT("DialogueTree"), GetFName());
    }

    UPROPERTY(EditAnywhere, BlueprintReadOnly) FGameplayTag EntryNodeTag;
    UPROPERTY(EditAnywhere, BlueprintReadOnly) TMap<FGameplayTag, FDialogueNode> Nodes;

    const FDialogueNode* GetNode(FGameplayTag Tag) const { return Nodes.Find(Tag); }
    const FDialogueNode* GetEntryNode() const { return GetNode(EntryNodeTag); }
};

// ============================================================
//  Delegates
// ============================================================

DECLARE_MULTICAST_DELEGATE_TwoParams(FOnDialogueNodeActivated,
    const FDialogueNode& /*Node*/, int32 /*NodeIndex*/);
DECLARE_MULTICAST_DELEGATE(FOnDialogueEnded);

// ============================================================
//  NarrativeManager
// ============================================================

/**
 * UNarrativeManager
 *
 * Drives all narrative systems:
 *   - Dialogue tree traversal
 *   - Cinematic sequence triggering
 *   - Story chapter tracking
 *   - NPC schedule management
 *   - Consequence application (world state changes from dialogue choices)
 */
UCLASS(NotBlueprintable)
class NARUTOUSL_API UNarrativeManager : public UNarutoSubsystem
{
    GENERATED_BODY()

public:

    void Initialize(UQuestManager* InQuestManager,
                    UWorldStateManager* InWorldStateManager);
    virtual void Shutdown() override;

    // ------------------------------------------------------------------
    //  Dialogue
    // ------------------------------------------------------------------

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    bool StartDialogue(UDialogueTree* Tree, AActor* Speaker, AActor* Listener);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void SelectDialogueChoice(int32 ChoiceIndex);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void AdvanceDialogue();

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void EndDialogue();

    UFUNCTION(BlueprintPure, Category = "Narrative")
    bool IsInDialogue() const { return bInDialogue; }

    UFUNCTION(BlueprintPure, Category = "Narrative")
    const FDialogueNode* GetCurrentNode() const;

    // ------------------------------------------------------------------
    //  Story Chapters
    // ------------------------------------------------------------------

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void SetCurrentChapter(int32 ChapterIndex);

    UFUNCTION(BlueprintPure, Category = "Narrative")
    int32 GetCurrentChapter() const { return CurrentChapter; }

    // ------------------------------------------------------------------
    //  Events
    // ------------------------------------------------------------------

    FOnDialogueNodeActivated OnDialogueNodeActivated;
    FOnDialogueEnded         OnDialogueEnded;

private:

    void ActivateNode(const FDialogueNode* Node);
    TArray<const FDialogueNode*> GetAvailableChoices() const;
    void ApplyConsequences(const FDialogueNode& Node);

    TWeakObjectPtr<UQuestManager>      QuestManager;
    TWeakObjectPtr<UWorldStateManager> WorldStateManager;

    TWeakObjectPtr<UDialogueTree>  ActiveTree;
    FGameplayTag                   CurrentNodeTag;
    TWeakObjectPtr<AActor>         ActiveSpeaker;
    TWeakObjectPtr<AActor>         ActiveListener;
    bool                           bInDialogue    = false;
    int32                          CurrentChapter = 0;
};
