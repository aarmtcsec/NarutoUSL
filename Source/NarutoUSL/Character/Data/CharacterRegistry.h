// Copyright (c) 2025 Maze Runner Studios. All Rights Reserved.
// Naruto: Ultimate Shinobi Legacy
// CharacterRegistry — Static registry of all 150 characters.
// Each entry provides the asset paths for every character's data asset,
// placeholder portrait, and first-person arms mesh.
//
// PLACEHOLDER TEXTURES:
//   All texture paths point to /Game/Characters/<Name>/Textures/T_<Name>_Portrait
//   and /Game/Characters/<Name>/Textures/T_<Name>_FullBody.
//   These are placeholder paths — the actual texture assets must be created
//   in the UE5 Content Browser at these paths by the art team.
//   The engine will show a default checkerboard until real textures are imported.
//
// To add real textures:
//   1. Import your PNG/TGA into UE5 at the path listed below.
//   2. The data asset will automatically reference it — no code change needed.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Core/Types/NarutoTypes.h"
#include "GameplayTagContainer.h"
#include "CharacterRegistry.generated.h"

// ============================================================
//  Character Registry Entry
// ============================================================

USTRUCT(BlueprintType)
struct NARUTOUSL_API FCharacterRegistryEntry
{
    GENERATED_BODY()

    /** Unique tag. Maps to DA_Character_<Name> asset. */
    UPROPERTY(EditAnywhere, BlueprintReadOnly) FGameplayTag CharacterTag;
    UPROPERTY(EditAnywhere, BlueprintReadOnly) FText        DisplayName;

    // Asset paths (soft references — loaded on demand)
    UPROPERTY(EditAnywhere, BlueprintReadOnly) FSoftObjectPath CharacterDataAsset;
    UPROPERTY(EditAnywhere, BlueprintReadOnly) FSoftObjectPath PortraitTexture;
    UPROPERTY(EditAnywhere, BlueprintReadOnly) FSoftObjectPath FullBodyTexture;
    UPROPERTY(EditAnywhere, BlueprintReadOnly) FSoftObjectPath FPArmsSkeletalMesh;
    UPROPERTY(EditAnywhere, BlueprintReadOnly) FSoftObjectPath CharacterSkeletalMesh;
    UPROPERTY(EditAnywhere, BlueprintReadOnly) FSoftObjectPath AnimBlueprint;

    UPROPERTY(EditAnywhere, BlueprintReadOnly) EVillage        Village        = EVillage::None;
    UPROPERTY(EditAnywhere, BlueprintReadOnly) EShinobi_Rank   Rank           = EShinobi_Rank::Genin;
    UPROPERTY(EditAnywhere, BlueprintReadOnly) bool            bUnlocked      = false;
    UPROPERTY(EditAnywhere, BlueprintReadOnly) bool            bIsPlaceholder = true;
};

// ============================================================
//  UCharacterRegistry — singleton data asset
// ============================================================

/**
 * UCharacterRegistry
 *
 * Primary data asset that lists every character in the game.
 * Loaded once at game start by the GameInstance.
 * All character selection UI, save data, and unlock logic reads from here.
 *
 * Asset path: /Game/Data/Characters/DA_CharacterRegistry
 */
UCLASS(BlueprintType)
class NARUTOUSL_API UCharacterRegistry : public UPrimaryDataAsset
{
    GENERATED_BODY()

public:

    virtual FPrimaryAssetId GetPrimaryAssetId() const override
    {
        return FPrimaryAssetId(TEXT("CharacterRegistry"), GetFName());
    }

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Registry")
    TArray<FCharacterRegistryEntry> Characters;

    UFUNCTION(BlueprintPure, Category = "Registry")
    const FCharacterRegistryEntry* FindByTag(FGameplayTag Tag) const;

    UFUNCTION(BlueprintPure, Category = "Registry")
    TArray<FCharacterRegistryEntry> GetCharactersByVillage(EVillage Village) const;

    UFUNCTION(BlueprintPure, Category = "Registry")
    TArray<FCharacterRegistryEntry> GetUnlockedCharacters() const;

    // ------------------------------------------------------------------
    //  Static helpers — build the default registry at startup
    // ------------------------------------------------------------------

    /**
     * Populates the registry with all 150 characters using placeholder paths.
     * Called once from GameInstance::Init() if the asset hasn't been
     * set up in the editor yet.
     */
    static TArray<FCharacterRegistryEntry> BuildDefaultRegistry();
};
