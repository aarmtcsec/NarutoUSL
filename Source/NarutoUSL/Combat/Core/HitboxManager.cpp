// Copyright (c) 2025 Maze Runner Studios. All Rights Reserved.

#include "Combat/Core/HitboxManager.h"
#include "Character/Base/NarutoCharacterBase.h"
#include "Character/Components/CombatComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "DrawDebugHelpers.h"
#include "NarutoUSL.h"

UHitboxManager::UHitboxManager()
{
}

void UHitboxManager::Initialize(UWorld* InWorld)
{
    World = InWorld;
    UE_LOG(LogNarutoCombat, Log, TEXT("[HitboxManager] Initialized."));
}

void UHitboxManager::Shutdown()
{
    ClearAllHitboxes();
    RegisteredHurtboxes.Empty();
}

// ============================================================
//  Hitbox Registration
// ============================================================

uint32 UHitboxManager::RegisterHitbox(ANarutoCharacterBase* Owner,
                                       const FHitboxData& HitboxData,
                                       int32 MaxTargets)
{
    if (!Owner)
    {
        UE_LOG(LogNarutoCombat, Warning, TEXT("[HitboxManager] RegisterHitbox called with null owner."));
        return 0;
    }

    const uint32 ID = NextHitboxInstanceID++;

    FActiveHitbox& NewHitbox = ActiveHitboxes.Add(ID);
    NewHitbox.HitboxInstanceID  = ID;
    NewHitbox.Owner             = Owner;
    NewHitbox.HitboxData        = HitboxData;
    NewHitbox.MaxTargets        = MaxTargets;
    NewHitbox.bIsActive         = true;

    UE_LOG(LogNarutoCombat, Verbose,
        TEXT("[HitboxManager] Registered hitbox %u for %s (bone: %s)"),
        ID, *Owner->GetName(), *HitboxData.AttachBone.ToString());

    return ID;
}

void UHitboxManager::DeregisterHitbox(uint32 HitboxInstanceID)
{
    ActiveHitboxes.Remove(HitboxInstanceID);
}

void UHitboxManager::DeregisterAllHitboxesForOwner(ANarutoCharacterBase* Owner)
{
    TArray<uint32> ToRemove;
    for (const auto& Pair : ActiveHitboxes)
    {
        if (Pair.Value.Owner.Get() == Owner)
        {
            ToRemove.Add(Pair.Key);
        }
    }
    for (uint32 ID : ToRemove)
    {
        ActiveHitboxes.Remove(ID);
    }
}

void UHitboxManager::ClearAllHitboxes()
{
    ActiveHitboxes.Empty();
}

// ============================================================
//  Hurtbox Registration
// ============================================================

void UHitboxManager::RegisterHurtboxes(ANarutoCharacterBase* Character,
                                        const TArray<FHurtboxVolume>& Volumes)
{
    if (!Character) return;
    RegisteredHurtboxes.Add(Character, Volumes);
}

void UHitboxManager::UnregisterHurtboxes(ANarutoCharacterBase* Character)
{
    RegisteredHurtboxes.Remove(Character);
}

// ============================================================
//  Evaluation
// ============================================================

TArray<TPair<FActiveHitbox*, FHitQueryResult>> UHitboxManager::EvaluateAllHitboxes()
{
    TArray<TPair<FActiveHitbox*, FHitQueryResult>> AllHits;

    for (auto& HitboxPair : ActiveHitboxes)
    {
        FActiveHitbox& Hitbox = HitboxPair.Value;
        if (!Hitbox.bIsActive || !Hitbox.Owner.IsValid()) continue;

        TArray<FHitQueryResult> HitResults = EvaluateHitbox(Hitbox);
        for (FHitQueryResult& Result : HitResults)
        {
            AllHits.Add(TPair<FActiveHitbox*, FHitQueryResult>(&Hitbox, Result));
        }
    }

    return AllHits;
}

TArray<FHitQueryResult> UHitboxManager::EvaluateHitbox(FActiveHitbox& Hitbox)
{
    TArray<FHitQueryResult> Results;

    ANarutoCharacterBase* OwnerChar = Hitbox.Owner.Get();
    if (!OwnerChar) return Results;

    for (auto& HurtboxPair : RegisteredHurtboxes)
    {
        ANarutoCharacterBase* DefenderChar = HurtboxPair.Key.Get();
        if (!DefenderChar) continue;

        // Don't hit yourself
        if (DefenderChar == OwnerChar) continue;

        // Don't hit already-hit actors
        if (!Hitbox.CanHitActor(DefenderChar)) continue;

        // Don't hit dead characters
        if (DefenderChar->IsDead_Implementation()) continue;

        // Don't hit invulnerable characters
        if (DefenderChar->IsInvulnerable_Implementation()) continue;

        // Test against each hurtbox zone
        for (const FHurtboxVolume& Hurtbox : HurtboxPair.Value)
        {
            if (!Hurtbox.bActive) continue;

            FHitQueryResult HitResult;
            if (TestHitboxVsHurtbox(Hitbox, DefenderChar, Hurtbox, HitResult))
            {
                HitResult.HitActor = DefenderChar;
                HitResult.bIsValid = true;
                Results.Add(HitResult);

                // Register the hit to prevent multi-hit
                Hitbox.RegisterHit(DefenderChar);

                // Only register one zone hit per character per frame
                break;
            }
        }
    }

    return Results;
}

bool UHitboxManager::TestHitboxVsHurtbox(const FActiveHitbox& Hitbox,
                                          ANarutoCharacterBase* DefenderChar,
                                          const FHurtboxVolume& Hurtbox,
                                          FHitQueryResult& OutResult) const
{
    const FTransform HitboxTransform  = GetHitboxWorldTransform(Hitbox);
    const FTransform HurtboxTransform = GetHurtboxWorldTransform(DefenderChar, Hurtbox);

    const FVector HitboxCenter  = HitboxTransform.GetLocation();
    const FVector HurtboxCenter = HurtboxTransform.GetLocation();

    bool bOverlap = false;

    if (Hitbox.HitboxData.SphereRadius > 0.0f)
    {
        // Sphere vs AABB test
        const float DistSq = FVector::DistSquared(HitboxCenter, HurtboxCenter);
        const float RadiusSum = Hitbox.HitboxData.SphereRadius +
            Hurtbox.HalfExtent.GetMax();
        bOverlap = DistSq <= (RadiusSum * RadiusSum);
    }
    else
    {
        // AABB vs AABB test (axis-aligned approximation for performance)
        const FVector Delta = (HitboxCenter - HurtboxCenter).GetAbs();
        const FVector SumExtents = Hitbox.HitboxData.HalfExtent + Hurtbox.HalfExtent;
        bOverlap = (Delta.X <= SumExtents.X) &&
                   (Delta.Y <= SumExtents.Y) &&
                   (Delta.Z <= SumExtents.Z);
    }

    if (bOverlap)
    {
        OutResult.HitZone          = Hurtbox;
        OutResult.HitLocation      = (HitboxCenter + HurtboxCenter) * 0.5f;
        OutResult.HitNormal        = (HurtboxCenter - HitboxCenter).GetSafeNormal();
        OutResult.ZoneMultiplier   = Hurtbox.ZoneDamageMultiplier;
    }

    return bOverlap;
}

FTransform UHitboxManager::GetHitboxWorldTransform(const FActiveHitbox& Hitbox) const
{
    ANarutoCharacterBase* OwnerChar = Hitbox.Owner.Get();
    if (!OwnerChar) return FTransform::Identity;

    USkeletalMeshComponent* Mesh = OwnerChar->GetMesh();
    if (!Mesh) return FTransform::Identity;

    if (Hitbox.HitboxData.AttachBone != NAME_None)
    {
        const FTransform BoneTransform = Mesh->GetBoneTransform(
            Mesh->GetBoneIndex(Hitbox.HitboxData.AttachBone));
        return FTransform(BoneTransform.GetRotation(),
                          BoneTransform.TransformPosition(Hitbox.HitboxData.LocalOffset));
    }

    return FTransform(OwnerChar->GetActorRotation().Quaternion(),
                      OwnerChar->GetActorLocation() + Hitbox.HitboxData.LocalOffset);
}

FTransform UHitboxManager::GetHurtboxWorldTransform(ANarutoCharacterBase* Character,
                                                     const FHurtboxVolume& Hurtbox) const
{
    if (!Character) return FTransform::Identity;

    USkeletalMeshComponent* Mesh = Character->GetMesh();
    if (!Mesh) return FTransform::Identity;

    if (Hurtbox.AttachBone != NAME_None)
    {
        const FTransform BoneTransform = Mesh->GetBoneTransform(
            Mesh->GetBoneIndex(Hurtbox.AttachBone));
        return FTransform(BoneTransform.GetRotation(),
                          BoneTransform.TransformPosition(Hurtbox.LocalOffset));
    }

    return FTransform(Character->GetActorRotation().Quaternion(),
                      Character->GetActorLocation() + Hurtbox.LocalOffset);
}

// ============================================================
//  Debug
// ============================================================

void UHitboxManager::DrawDebugShapes(float Duration) const
{
#if ENABLE_DRAW_DEBUG
    UWorld* W = World.Get();
    if (!W) return;

    if (bDrawDebugHitboxes)
    {
        for (const auto& Pair : ActiveHitboxes)
        {
            const FActiveHitbox& Hitbox = Pair.Value;
            if (!Hitbox.bIsActive || !Hitbox.Owner.IsValid()) continue;

            const FTransform T = GetHitboxWorldTransform(Hitbox);

            if (Hitbox.HitboxData.SphereRadius > 0.0f)
            {
                DrawDebugSphere(W, T.GetLocation(), Hitbox.HitboxData.SphereRadius,
                    12, FColor::Red, false, Duration);
            }
            else
            {
                DrawDebugBox(W, T.GetLocation(), Hitbox.HitboxData.HalfExtent,
                    T.GetRotation(), FColor::Red, false, Duration);
            }
        }
    }

    if (bDrawDebugHurtboxes)
    {
        for (const auto& Pair : RegisteredHurtboxes)
        {
            ANarutoCharacterBase* Char = Pair.Key.Get();
            if (!Char) continue;

            for (const FHurtboxVolume& Hurtbox : Pair.Value)
            {
                if (!Hurtbox.bActive) continue;
                const FTransform T = GetHurtboxWorldTransform(Char, Hurtbox);

                if (Hurtbox.SphereRadius > 0.0f)
                {
                    DrawDebugSphere(W, T.GetLocation(), Hurtbox.SphereRadius,
                        12, FColor::Blue, false, Duration);
                }
                else
                {
                    DrawDebugBox(W, T.GetLocation(), Hurtbox.HalfExtent,
                        T.GetRotation(), FColor::Blue, false, Duration);
                }
            }
        }
    }
#endif
}
