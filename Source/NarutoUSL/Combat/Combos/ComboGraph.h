// Copyright (c) 2025 Maze Runner Studios. All Rights Reserved.
// Naruto: Ultimate Shinobi Legacy
// ComboGraph — Data-driven directed graph of combo transitions.
// Each node is an attack. Edges are input conditions that advance the combo.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "Core/Types/NarutoTypes.h"
#include "ComboGraph.generated.h"

// ============================================================
//  Input Condition
// ============================================================

UENUM(BlueprintType)
enum class EComboInputType : uint8
{
    LightAttack     UMETA(DisplayName = "Light Attack"),
    HeavyAttack     UMETA(DisplayName = "Heavy Attack"),
    JutsuAttack     UMETA(DisplayName = "Jutsu Attack"),
    DirectionalLight UMETA(DisplayName = "Directional Light"),
    DirectionalHeavy UMETA(DisplayName = "Directional Heavy"),
    AirLight        UMETA(DisplayName = "Air Light"),
    AirHeavy        UMETA(DisplayName = "Air Heavy"),
    Launcher        UMETA(DisplayName = "Launcher"),
    Grab            UMETA(DisplayName = "Grab"),
};

// ============================================================
//  Combo Node
// ============================================================

/**
 * FComboNode
 * A single attack in the combo graph.
 * Contains the attack's frame data, hitboxes, and all possible
 * transitions to the next node based on player input.
 */
USTRUCT(BlueprintType)
struct NARUTOUSL_API FComboNode
{
    GENERATED_BODY()

    /** Unique identifier for this node within the graph. */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combo")
    FName NodeID;

    /** Display name for editor and debug UI. */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combo")
    FText DisplayName;

    /** Animation montage to play for this attack. */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combo")
    TSoftObjectPtr<UAnimMontage> AttackMontage;

    /** Frame data for this attack. */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combo")
    FAttackFrameData FrameData;

    /** Hitboxes active during this attack's active frames. */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combo")
    TArray<FHitboxData> Hitboxes;

    /** Base damage for this attack (before scaling). */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combo", meta = (ClampMin = "0"))
    float BaseDamage = 50.0f;

    /** Damage type for this attack. */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combo")
    EDamageType DamageType = EDamageType::Physical;

    /** Whether this attack launches the target into the air. */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combo")
    bool bIsLauncher = false;

    /** Whether this attack is an aerial attack (requires IsAirborne flag). */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combo")
    bool bRequiresAirborne = false;

    /** Whether this attack ends the combo (no further transitions). */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combo")
    bool bIsFinisher = false;

    /**
     * Transitions from this node.
     * Key = EComboInputType, Value = target NodeID.
     * Evaluated during the cancel window.
     */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combo")
    TMap<EComboInputType, FName> Transitions;

    /** Tags required on the attacker to use this node (e.g., SageMode). */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combo")
    FGameplayTagContainer RequiredAttackerTags;

    /** Tags required on the target to use this node (e.g., IsAirborne). */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combo")
    FGameplayTagContainer RequiredTargetTags;

    bool IsValid() const { return !NodeID.IsNone(); }
};

// ============================================================
//  ComboGraph Data Asset
// ============================================================

/**
 * UComboGraphData
 *
 * Data asset defining the full combo graph for a character.
 * One asset per character. Contains all nodes and the entry
 * point for each input type.
 *
 * Naming convention: DA_ComboGraph_Naruto, DA_ComboGraph_Sasuke, etc.
 */
UCLASS(BlueprintType)
class NARUTOUSL_API UComboGraphData : public UPrimaryDataAsset
{
    GENERATED_BODY()

public:

    virtual FPrimaryAssetId GetPrimaryAssetId() const override
    {
        return FPrimaryAssetId(TEXT("ComboGraph"), GetFName());
    }

    /** All nodes in this combo graph. Key = NodeID. */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combo")
    TMap<FName, FComboNode> Nodes;

    /** Entry node IDs for each input type (first hit of a combo). */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combo")
    TMap<EComboInputType, FName> EntryNodes;

    /** Returns the node with the specified ID. Returns nullptr if not found. */
    const FComboNode* GetNode(FName NodeID) const
    {
        return Nodes.Find(NodeID);
    }

    /** Returns the entry node for the specified input type. */
    const FComboNode* GetEntryNode(EComboInputType InputType) const
    {
        const FName* EntryID = EntryNodes.Find(InputType);
        return EntryID ? GetNode(*EntryID) : nullptr;
    }

    /**
     * Returns the next node given the current node and input.
     * Returns nullptr if no valid transition exists.
     */
    const FComboNode* GetNextNode(FName CurrentNodeID, EComboInputType Input) const
    {
        const FComboNode* Current = GetNode(CurrentNodeID);
        if (!Current) return nullptr;

        const FName* NextID = Current->Transitions.Find(Input);
        return NextID ? GetNode(*NextID) : nullptr;
    }
};

// ============================================================
//  ComboSystem Runtime State
// ============================================================

/**
 * FComboState
 * Runtime state of the combo system for a single character.
 * Owned by ComboSystem, one instance per active combatant.
 */
USTRUCT()
struct NARUTOUSL_API FComboState
{
    GENERATED_BODY()

    /** The combo graph data asset for this character. */
    TWeakObjectPtr<UComboGraphData> GraphData;

    /** Currently active node ID. NAME_None = not in a combo. */
    FName CurrentNodeID = NAME_None;

    /** Buffered input received during the cancel window. */
    TOptional<EComboInputType> BufferedInput;

    /** Whether we are currently in a cancel window. */
    bool bInCancelWindow = false;

    /** Whether a combo is currently active. */
    bool bComboActive = false;

    bool IsInCombo() const { return bComboActive && !CurrentNodeID.IsNone(); }

    void Reset()
    {
        CurrentNodeID  = NAME_None;
        BufferedInput.Reset();
        bInCancelWindow = false;
        bComboActive    = false;
    }
};
