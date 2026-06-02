// Copyright (c) 2025 Maze Runner Studios. All Rights Reserved.
// Naruto: Ultimate Shinobi Legacy
// HitboxManager — Evaluates hitboxes against hurtboxes each frame.
// Tracks hit registration to prevent multi-hit within a single active window.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Core/Types/NarutoTypes.h"
#include "HitboxManager.generated.h"

class ANarutoCharacterBase;
class UCombatComponent;

// ============================================================
//  Active Hitbox Registration
// ============================================================

/**
 * FActiveHitbox
 * Represents a hitbox that is currently active in the world.
 * Registered by CombatManager when an attack enters its active frames.
 * Deregistered when the attack exits active frames.
 */
USTRUCT()
struct NARUTOUSL_API FActiveHitbox
{
    GENERATED_BODY()

    /** Unique ID for this hitbox instance. */
    uint32 HitboxInstanceID = 0;

    /** The character that owns this hitbox. */
    TWeakObjectPtr<ANarutoCharacterBase> Owner;

    /** Hitbox definition from the attack's frame data. */
    FHitboxData HitboxData;

    /** Actors already hit by this hitbox instance (prevents multi-hit). */
    TSet<TWeakObjectPtr<AActor>> AlreadyHitActors;

    /** Maximum number of targets this hitbox can hit (0 = unlimited). */
    int32 MaxTargets = 0;

    /** Current hit count. */
    int32 HitCount = 0;

    /** Frame this hitbox was registered. */
    int32 RegistrationFrame = 0;

    /** Whether this hitbox is still active. */
    bool bIsActive = true;

    bool CanHitActor(AActor* Actor) const
    {
        if (!bIsActive) return false;
        if (MaxTargets > 0 && HitCount >= MaxTargets) return false;
        return !AlreadyHitActors.Contains(Actor);
    }

    void RegisterHit(AActor* Actor)
    {
        AlreadyHitActors.Add(Actor);
        ++HitCount;
    }
};

// ============================================================
//  Hurtbox Definition
// ============================================================

/**
 * FHurtboxVolume
 * Defines the vulnerable area of a character.
 * Characters have multiple hurtbox zones (head, torso, limbs)
 * for directional damage and hit reaction variation.
 */
USTRUCT()
struct NARUTOUSL_API FHurtboxVolume
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere) FName ZoneName;
    UPROPERTY(EditAnywhere) FName AttachBone;
    UPROPERTY(EditAnywhere) FVector LocalOffset = FVector::ZeroVector;
    UPROPERTY(EditAnywhere) FVector HalfExtent  = FVector(30.0f, 30.0f, 50.0f);
    UPROPERTY(EditAnywhere) float   SphereRadius = 0.0f;

    /** Damage multiplier for hits to this zone (e.g., head = 1.5). */
    UPROPERTY(EditAnywhere) float ZoneDamageMultiplier = 1.0f;

    /** Whether this zone is currently active (can be disabled during certain states). */
    UPROPERTY(EditAnywhere) bool bActive = true;
};

// ============================================================
//  Hit Query Result
// ============================================================

USTRUCT()
struct NARUTOUSL_API FHitQueryResult
{
    GENERATED_BODY()

    TWeakObjectPtr<AActor>  HitActor;
    FHurtboxVolume          HitZone;
    FVector                 HitLocation  = FVector::ZeroVector;
    FVector                 HitNormal    = FVector::UpVector;
    float                   ZoneMultiplier = 1.0f;
    bool                    bIsValid     = false;
};

// ============================================================
//  HitboxManager
// ============================================================

/**
 * UHitboxManager
 *
 * Owned by CombatManager. Evaluates all active hitboxes against
 * all registered hurtboxes each frame during the combat tick.
 *
 * Design principles:
 *   - Hitboxes are registered/deregistered by CombatManager
 *   - Evaluation uses swept shape traces, not overlap events
 *   - Hit registration is tracked per-hitbox-instance to prevent
 *     the same hitbox hitting the same target twice
 *   - Hurtbox zones are queried from the character's skeleton
 *     using bone transforms at evaluation time
 *
 * Performance:
 *   - Only evaluates hitboxes that are in their active frames
 *   - Uses spatial partitioning (UE5 physics scene) for broad phase
 *   - Narrow phase is a simple OBB vs OBB or sphere vs OBB test
 */
UCLASS(NotBlueprintable)
class NARUTOUSL_API UHitboxManager : public UObject
{
    GENERATED_BODY()

public:

    UHitboxManager();

    void Initialize(UWorld* InWorld);
    void Shutdown();

    // ------------------------------------------------------------------
    //  Hitbox Registration
    // ------------------------------------------------------------------

    /**
     * Registers a new active hitbox. Returns the instance ID.
     * Called by CombatManager when an attack enters active frames.
     */
    uint32 RegisterHitbox(ANarutoCharacterBase* Owner, const FHitboxData& HitboxData,
                          int32 MaxTargets = 0);

    /**
     * Deregisters a hitbox by instance ID.
     * Called by CombatManager when an attack exits active frames.
     */
    void DeregisterHitbox(uint32 HitboxInstanceID);

    /** Deregisters all hitboxes owned by the specified character. */
    void DeregisterAllHitboxesForOwner(ANarutoCharacterBase* Owner);

    /** Clears all active hitboxes. Called on combat reset. */
    void ClearAllHitboxes();

    // ------------------------------------------------------------------
    //  Hurtbox Registration
    // ------------------------------------------------------------------

    /**
     * Registers a character's hurtbox volumes.
     * Called when a character enters combat.
     */
    void RegisterHurtboxes(ANarutoCharacterBase* Character,
                           const TArray<FHurtboxVolume>& Volumes);

    void UnregisterHurtboxes(ANarutoCharacterBase* Character);

    // ------------------------------------------------------------------
    //  Evaluation
    // ------------------------------------------------------------------

    /**
     * Evaluates all active hitboxes against all registered hurtboxes.
     * Called once per combat tick by CombatManager.
     * Returns all hit results for this frame.
     */
    TArray<TPair<FActiveHitbox*, FHitQueryResult>> EvaluateAllHitboxes();

    // ------------------------------------------------------------------
    //  Debug
    // ------------------------------------------------------------------

    /** Draws all active hitboxes and hurtboxes as debug shapes. */
    void DrawDebugShapes(float Duration = 0.0f) const;

    UPROPERTY(EditAnywhere, Category = "Debug")
    bool bDrawDebugHitboxes = false;

    UPROPERTY(EditAnywhere, Category = "Debug")
    bool bDrawDebugHurtboxes = false;

private:

    // ------------------------------------------------------------------
    //  Internal
    // ------------------------------------------------------------------

    /**
     * Evaluates a single hitbox against all registered hurtboxes.
     * Returns all valid hit results.
     */
    TArray<FHitQueryResult> EvaluateHitbox(FActiveHitbox& Hitbox);

    /**
     * Tests a hitbox against a single hurtbox volume.
     * Returns true if they overlap, populates OutResult.
     */
    bool TestHitboxVsHurtbox(const FActiveHitbox& Hitbox,
                              ANarutoCharacterBase* DefenderChar,
                              const FHurtboxVolume& Hurtbox,
                              FHitQueryResult& OutResult) const;

    /**
     * Returns the world-space transform of a hitbox attached to a bone.
     */
    FTransform GetHitboxWorldTransform(const FActiveHitbox& Hitbox) const;

    /**
     * Returns the world-space transform of a hurtbox attached to a bone.
     */
    FTransform GetHurtboxWorldTransform(ANarutoCharacterBase* Character,
                                        const FHurtboxVolume& Hurtbox) const;

    // ------------------------------------------------------------------
    //  State
    // ------------------------------------------------------------------

    TWeakObjectPtr<UWorld> World;

    /** All currently active hitboxes. Key = instance ID. */
    TMap<uint32, FActiveHitbox> ActiveHitboxes;

    /** All registered hurtbox sets. Key = character pointer. */
    TMap<TWeakObjectPtr<ANarutoCharacterBase>, TArray<FHurtboxVolume>> RegisteredHurtboxes;

    uint32 NextHitboxInstanceID = 1;
};
