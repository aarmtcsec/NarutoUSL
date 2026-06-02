// Copyright (c) 2025 Maze Runner Studios. All Rights Reserved.

#include "Combat/Combos/ComboSystem.h"
#include "Character/Base/NarutoCharacterBase.h"
#include "Character/Components/CombatComponent.h"
#include "NarutoUSL.h"

void UComboSystem::Initialize()
{
    UE_LOG(LogNarutoCombat, Log, TEXT("[ComboSystem] Initialized."));
}

void UComboSystem::Shutdown()
{
    ComboStates.Empty();
}

// ============================================================
//  Registration
// ============================================================

void UComboSystem::RegisterCombatant(ANarutoCharacterBase* Character, UComboGraphData* GraphData)
{
    if (!Character || !GraphData) return;

    FComboState& State = ComboStates.FindOrAdd(Character);
    State.GraphData = GraphData;
    State.Reset();
}

void UComboSystem::UnregisterCombatant(ANarutoCharacterBase* Character)
{
    ComboStates.Remove(Character);
}

// ============================================================
//  Input Processing
// ============================================================

const FComboNode* UComboSystem::ProcessInput(ANarutoCharacterBase* Character, EComboInputType Input)
{
    if (!Character) return nullptr;

    FComboState* State = ComboStates.Find(Character);
    if (!State || !State->GraphData.IsValid()) return nullptr;

    UComboGraphData* Graph = State->GraphData.Get();

    // Not in a combo — try to start one from the entry node
    if (!State->IsInCombo())
    {
        const FComboNode* EntryNode = Graph->GetEntryNode(Input);
        if (!EntryNode || !ValidateNodeRequirements(Character, *EntryNode))
        {
            return nullptr;
        }

        return ActivateNode(Character, *State, EntryNode);
    }

    // In a combo — try to transition
    if (State->bInCancelWindow)
    {
        const FComboNode* NextNode = Graph->GetNextNode(State->CurrentNodeID, Input);
        if (NextNode && ValidateNodeRequirements(Character, *NextNode))
        {
            return ActivateNode(Character, *State, NextNode);
        }
    }
    else
    {
        // Outside cancel window — buffer the input
        State->BufferedInput = Input;
        UE_LOG(LogNarutoCombat, Verbose,
            TEXT("[ComboSystem] Buffered input for %s."), *Character->GetName());
    }

    return nullptr;
}

void UComboSystem::BufferInput(ANarutoCharacterBase* Character, EComboInputType Input)
{
    FComboState* State = ComboStates.Find(Character);
    if (State)
    {
        State->BufferedInput = Input;
    }
}

// ============================================================
//  State Control
// ============================================================

void UComboSystem::OnCancelWindowOpened(ANarutoCharacterBase* Character)
{
    FComboState* State = ComboStates.Find(Character);
    if (!State) return;

    State->bInCancelWindow = true;

    // Consume buffered input immediately
    if (State->BufferedInput.IsSet())
    {
        const EComboInputType Buffered = State->BufferedInput.GetValue();
        State->BufferedInput.Reset();
        ProcessInput(Character, Buffered);
    }
}

void UComboSystem::OnCancelWindowClosed(ANarutoCharacterBase* Character)
{
    FComboState* State = ComboStates.Find(Character);
    if (State)
    {
        State->bInCancelWindow = false;
        State->BufferedInput.Reset();
    }
}

void UComboSystem::OnAttackInterrupted(ANarutoCharacterBase* Character)
{
    FComboState* State = ComboStates.Find(Character);
    if (!State) return;

    State->Reset();
    OnComboFinished.Broadcast(Character);
}

void UComboSystem::OnComboTimeout(ANarutoCharacterBase* Character)
{
    FComboState* State = ComboStates.Find(Character);
    if (!State) return;

    State->Reset();
    OnComboFinished.Broadcast(Character);
}

// ============================================================
//  Queries
// ============================================================

bool UComboSystem::IsInCombo(ANarutoCharacterBase* Character) const
{
    const FComboState* State = ComboStates.Find(Character);
    return State && State->IsInCombo();
}

const FComboNode* UComboSystem::GetCurrentNode(ANarutoCharacterBase* Character) const
{
    const FComboState* State = ComboStates.Find(Character);
    if (!State || !State->IsInCombo() || !State->GraphData.IsValid()) return nullptr;

    return State->GraphData->GetNode(State->CurrentNodeID);
}

bool UComboSystem::HasBufferedInput(ANarutoCharacterBase* Character) const
{
    const FComboState* State = ComboStates.Find(Character);
    return State && State->BufferedInput.IsSet();
}

// ============================================================
//  Internal
// ============================================================

const FComboNode* UComboSystem::ActivateNode(ANarutoCharacterBase* Character,
                                              FComboState& State,
                                              const FComboNode* Node)
{
    if (!Node) return nullptr;

    State.CurrentNodeID  = Node->NodeID;
    State.bComboActive   = true;
    State.bInCancelWindow = false;

    ++GlobalComboIndex;

    UE_LOG(LogNarutoCombat, Verbose,
        TEXT("[ComboSystem] %s activated combo node: %s"),
        *Character->GetName(), *Node->NodeID.ToString());

    OnComboNodeActivated.Broadcast(Character, *Node, GlobalComboIndex);

    if (Node->bIsFinisher)
    {
        State.Reset();
        OnComboFinished.Broadcast(Character);
    }

    return Node;
}

bool UComboSystem::ValidateNodeRequirements(ANarutoCharacterBase* Character,
                                             const FComboNode& Node) const
{
    if (!Character) return false;

    // Check airborne requirement
    if (Node.bRequiresAirborne)
    {
        UCombatComponent* Combat = Character->GetCombatComponent();
        if (!Combat || !Combat->HasFlag(ECombatFlags::IsAirborne))
        {
            return false;
        }
    }

    // Check required attacker tags
    // (In a full implementation, query the character's active gameplay tags)
    // For now, tag requirements are validated by the character's ability system

    return true;
}
