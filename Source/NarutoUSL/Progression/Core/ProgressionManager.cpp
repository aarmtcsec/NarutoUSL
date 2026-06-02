// Copyright (c) 2025 Maze Runner Studios. All Rights Reserved.

#include "Progression/Core/ProgressionManager.h"
#include "Character/Player/NarutoPlayerCharacter.h"
#include "Character/Components/ProgressionComponent.h"
#include "Core/Events/NarutoEventBus.h"
#include "NarutoUSL.h"

UProgressionManager::UProgressionManager()
{
    SubsystemName = TEXT("ProgressionManager");
    bTickEnabled  = true;
    TickPriority  = ESubsystemTickPriority::PostRender;
}

void UProgressionManager::Initialize(UNarutoEventBus* InEventBus,
                                       UFactionManager* InFactionManager)
{
    EventBus       = InEventBus;
    FactionManager = InFactionManager;
    bInitialized   = true;
    UE_LOG(LogNarutoUSL, Log, TEXT("[ProgressionManager] Initialized."));
}

void UProgressionManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    TotalPlaytimeSeconds += DeltaTime;
}

void UProgressionManager::Shutdown()
{
    SkillTreeNodes.Empty();
    UnlockedNodes.Empty();
}

// ============================================================
//  XP
// ============================================================

void UProgressionManager::GrantXP(float Amount, FGameplayTag SourceTag)
{
    if (Amount <= 0.0f) return;

    // Route to active player's ProgressionComponent
    if (ActivePlayer.IsValid())
    {
        UProgressionComponent* Prog = ActivePlayer->GetProgressionComponent();
        if (Prog)
        {
            Prog->GainXP(Amount, SourceTag);
        }
    }
}

// ============================================================
//  Skill Tree
// ============================================================

bool UProgressionManager::UnlockSkillNode(FGameplayTag NodeTag)
{
    if (!CanUnlockNode(NodeTag)) return false;

    FSkillTreeNode* Node = SkillTreeNodes.Find(NodeTag);
    if (!Node) return false;

    // Spend skill points
    if (ActivePlayer.IsValid())
    {
        UProgressionComponent* Prog = ActivePlayer->GetProgressionComponent();
        if (!Prog) return false;

        for (int32 i = 0; i < Node->SkillPointCost; ++i)
        {
            if (!Prog->SpendSkillPoint()) return false;
        }

        // Apply granted modifiers
        for (const FStatModifier& Mod : Node->GrantedModifiers)
        {
            Prog->AddStatModifier(Mod);
        }

        // Unlock granted jutsu
        if (UJutsuComponent* JutsuComp = ActivePlayer->GetJutsuComponent())
        {
            // Jutsu unlocking handled by JutsuManager in full implementation
        }
    }

    Node->bUnlocked = true;
    UnlockedNodes.Add(NodeTag);

    UE_LOG(LogNarutoUSL, Log,
        TEXT("[ProgressionManager] Skill node unlocked: %s"), *NodeTag.ToString());
    return true;
}

bool UProgressionManager::IsNodeUnlocked(FGameplayTag NodeTag) const
{
    return UnlockedNodes.Contains(NodeTag);
}

bool UProgressionManager::CanUnlockNode(FGameplayTag NodeTag) const
{
    if (IsNodeUnlocked(NodeTag)) return false;

    const FSkillTreeNode* Node = SkillTreeNodes.Find(NodeTag);
    if (!Node) return false;

    // Check level requirement
    if (ActivePlayer.IsValid())
    {
        UProgressionComponent* Prog = ActivePlayer->GetProgressionComponent();
        if (Prog && Prog->GetLevel() < Node->RequiredLevel) return false;

        // Check skill points
        if (Prog && Prog->GetAvailableSkillPoints() < Node->SkillPointCost) return false;
    }

    // Check prerequisites
    for (const FGameplayTag& Prereq : Node->Prerequisites)
    {
        if (!IsNodeUnlocked(Prereq)) return false;
    }

    return true;
}

const FSkillTreeNode* UProgressionManager::GetNodeData(FGameplayTag NodeTag) const
{
    return SkillTreeNodes.Find(NodeTag);
}

void UProgressionManager::RegisterSkillTreeNodes(const TArray<FSkillTreeNode>& Nodes)
{
    for (const FSkillTreeNode& Node : Nodes)
    {
        if (Node.NodeTag.IsValid())
        {
            SkillTreeNodes.Add(Node.NodeTag, Node);
        }
    }
}

// ============================================================
//  Playtime
// ============================================================

FString UProgressionManager::GetFormattedPlaytime() const
{
    const int32 TotalSeconds = FMath::FloorToInt(TotalPlaytimeSeconds);
    const int32 Hours   = TotalSeconds / 3600;
    const int32 Minutes = (TotalSeconds % 3600) / 60;
    const int32 Seconds = TotalSeconds % 60;
    return FString::Printf(TEXT("%02d:%02d:%02d"), Hours, Minutes, Seconds);
}

void UProgressionManager::SerializeToSave(FArchive& Ar)
{
    Ar << TotalPlaytimeSeconds;

    int32 Count = UnlockedNodes.Num();
    Ar << Count;

    if (Ar.IsSaving())
    {
        for (const FGameplayTag& Tag : UnlockedNodes)
        {
            FString TagStr = Tag.ToString();
            Ar << TagStr;
        }
    }
    else
    {
        UnlockedNodes.Empty();
        for (int32 i = 0; i < Count; ++i)
        {
            FString TagStr;
            Ar << TagStr;
            FGameplayTag Tag = FGameplayTag::RequestGameplayTag(FName(*TagStr), false);
            if (Tag.IsValid()) UnlockedNodes.Add(Tag);
        }
    }
}

void UProgressionManager::DeserializeFromSave(FArchive& Ar)
{
    SerializeToSave(Ar);
}
