// Copyright (c) 2025 Maze Runner Studios. All Rights Reserved.
// Naruto: Ultimate Shinobi Legacy
// NarutoPlayerCharacter — Human-controlled character. First-person perspective.
// Camera attaches directly to the head bone. No spring arm, no third-person view.
// Arms mesh (first-person arms) rendered on a separate depth pass so it never
// clips into geometry. Lock-on rotates the camera to face the target without
// showing the player body.

#pragma once

#include "CoreMinimal.h"
#include "Character/Base/NarutoCharacterBase.h"
#include "InputActionValue.h"
#include "NarutoPlayerCharacter.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UInputMappingContext;
class UInputAction;
class UCombatManager;
class UJutsuManager;

// ============================================================
//  Lock-On State
// ============================================================

USTRUCT()
struct NARUTOUSL_API FLockOnState
{
    GENERATED_BODY()

    TWeakObjectPtr<ANarutoCharacterBase> Target;
    bool  bActive          = false;
    float SwitchCooldown   = 0.0f;

    bool IsValid() const { return bActive && Target.IsValid() && Target->IsAlive_Implementation(); }
};

// ============================================================
//  NarutoPlayerCharacter
// ============================================================

/**
 * ANarutoPlayerCharacter
 *
 * First-person player character. The player never sees their own body —
 * only the first-person arms mesh rendered in front of the camera.
 *
 * Camera setup:
 *   - UCameraComponent attaches to the head socket on the skeletal mesh
 *   - No spring arm — camera IS the head
 *   - bUsePawnControlRotation = true on the camera
 *   - Full body mesh still exists for shadows, multiplayer, and lock-on
 *     indicators — it is hidden from the local player's view using
 *     SetOwnerNoSee(true)
 *   - FP arms mesh (USkeletalMeshComponent) is rendered only to the owner
 *     using SetOnlyOwnerSee(true), on a near-clip depth pass to prevent
 *     geometry clipping
 *
 * All combat logic is delegated to CombatManager.
 * All jutsu logic is delegated to JutsuManager.
 */
UCLASS()
class NARUTOUSL_API ANarutoPlayerCharacter : public ANarutoCharacterBase
{
    GENERATED_BODY()

public:

    ANarutoPlayerCharacter(const FObjectInitializer& ObjectInitializer);

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;
    virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

    // ------------------------------------------------------------------
    //  Camera
    // ------------------------------------------------------------------

    /** First-person camera — attached directly to the head socket. No spring arm. */
    UFUNCTION(BlueprintPure, Category = "Camera")
    UCameraComponent* GetFirstPersonCamera() const { return FirstPersonCamera; }

    /** First-person arms mesh — only visible to the owning player. */
    UFUNCTION(BlueprintPure, Category = "Camera")
    USkeletalMeshComponent* GetArmsMesh() const { return FPArmsMesh; }

    // Kept for API compatibility — returns nullptr in FP mode
    UFUNCTION(BlueprintPure, Category = "Camera")
    UCameraComponent* GetFollowCamera() const { return FirstPersonCamera; }

    UFUNCTION(BlueprintPure, Category = "Camera")
    USpringArmComponent* GetCameraBoom() const { return nullptr; }

    // ------------------------------------------------------------------
    //  Lock-On
    // ------------------------------------------------------------------

    UFUNCTION(BlueprintCallable, Category = "Combat|LockOn")
    void ToggleLockOn();

    UFUNCTION(BlueprintCallable, Category = "Combat|LockOn")
    void SwitchLockOnTarget(bool bRight);

    UFUNCTION(BlueprintPure, Category = "Combat|LockOn")
    bool IsLockedOn() const { return LockOnState.IsValid(); }

    UFUNCTION(BlueprintPure, Category = "Combat|LockOn")
    ANarutoCharacterBase* GetLockOnTarget() const;

    // ------------------------------------------------------------------
    //  Jutsu Slots
    // ------------------------------------------------------------------

    UFUNCTION(BlueprintCallable, Category = "Jutsu")
    void ActivateJutsuSlot(int32 SlotIndex);

    // ------------------------------------------------------------------
    //  Overrides
    // ------------------------------------------------------------------

    virtual void OnCombatEntered_Implementation() override;
    virtual void OnCombatExited_Implementation() override;
    virtual void OnDefeated_Implementation(ICombatant* Killer) override;

protected:

    // ------------------------------------------------------------------
    //  Input Handlers
    // ------------------------------------------------------------------

    void Input_Move(const FInputActionValue& Value);
    void Input_Look(const FInputActionValue& Value);
    void Input_Jump(const FInputActionValue& Value);
    void Input_LightAttack(const FInputActionValue& Value);
    void Input_HeavyAttack(const FInputActionValue& Value);
    void Input_BlockPressed(const FInputActionValue& Value);
    void Input_BlockReleased(const FInputActionValue& Value);
    void Input_Substitution(const FInputActionValue& Value);
    void Input_LockOn(const FInputActionValue& Value);
    void Input_SwitchTargetRight(const FInputActionValue& Value);
    void Input_SwitchTargetLeft(const FInputActionValue& Value);
    void Input_JutsuSlot1(const FInputActionValue& Value);
    void Input_JutsuSlot2(const FInputActionValue& Value);
    void Input_JutsuSlot3(const FInputActionValue& Value);
    void Input_JutsuSlot4(const FInputActionValue& Value);
    void Input_Awakening(const FInputActionValue& Value);
    void Input_Dash(const FInputActionValue& Value);

    // ------------------------------------------------------------------
    //  Lock-On Helpers
    // ------------------------------------------------------------------

    ANarutoCharacterBase* FindBestLockOnTarget() const;
    void UpdateLockOnCamera(float DeltaTime);
    void TickLockOnValidation(float DeltaTime);

    // ------------------------------------------------------------------
    //  Components
    // ------------------------------------------------------------------

    /**
     * First-person camera attached to the head socket.
     * bUsePawnControlRotation = true so mouse look drives the view directly.
     */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera",
        meta = (AllowPrivateAccess = "true"))
    TObjectPtr<UCameraComponent> FirstPersonCamera;

    /**
     * First-person arms skeletal mesh.
     * SetOnlyOwnerSee(true)  — only the local player sees this.
     * SetOwnerNoSee(false)   — the owner DOES see it.
     * Rendered on depth stencil 1 so it always draws in front of world geometry.
     */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera",
        meta = (AllowPrivateAccess = "true"))
    TObjectPtr<USkeletalMeshComponent> FPArmsMesh;

    // Spring arm removed — not used in first-person mode.
    // Kept as nullptr stub so external code that calls GetCameraBoom() doesn't crash.
    TObjectPtr<USpringArmComponent> CameraBoom = nullptr;

    // ------------------------------------------------------------------
    //  Input Actions
    // ------------------------------------------------------------------

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
    TObjectPtr<UInputMappingContext> DefaultMappingContext;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
    TObjectPtr<UInputAction> IA_Move;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
    TObjectPtr<UInputAction> IA_Look;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
    TObjectPtr<UInputAction> IA_Jump;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
    TObjectPtr<UInputAction> IA_LightAttack;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
    TObjectPtr<UInputAction> IA_HeavyAttack;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
    TObjectPtr<UInputAction> IA_Block;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
    TObjectPtr<UInputAction> IA_Substitution;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
    TObjectPtr<UInputAction> IA_LockOn;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
    TObjectPtr<UInputAction> IA_SwitchTargetRight;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
    TObjectPtr<UInputAction> IA_SwitchTargetLeft;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
    TObjectPtr<UInputAction> IA_JutsuSlot1;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
    TObjectPtr<UInputAction> IA_JutsuSlot2;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
    TObjectPtr<UInputAction> IA_JutsuSlot3;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
    TObjectPtr<UInputAction> IA_JutsuSlot4;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
    TObjectPtr<UInputAction> IA_Awakening;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
    TObjectPtr<UInputAction> IA_Dash;

    // ------------------------------------------------------------------
    //  Camera Config
    // ------------------------------------------------------------------

    /** Vertical socket offset from the head bone center to the eye position. */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Camera")
    FVector EyeSocketOffset = FVector(0.0f, 0.0f, 8.0f);

    /** Name of the head socket on the skeletal mesh the camera attaches to. */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Camera")
    FName HeadSocketName = TEXT("head");

    /** Field of view in degrees. */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Camera",
        meta = (ClampMin = "60", ClampMax = "120"))
    float FieldOfView = 90.0f;

    /** FOV while aiming / using a jutsu with a narrow targeting cone. */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Camera",
        meta = (ClampMin = "40", ClampMax = "90"))
    float AimingFOV = 70.0f;

    /** How fast FOV interpolates between normal and aiming. */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Camera",
        meta = (ClampMin = "1", ClampMax = "30"))
    float FOVInterpSpeed = 10.0f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Camera",
        meta = (ClampMin = "0.1", ClampMax = "10.0"))
    float LockOnCameraInterpSpeed = 5.0f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat|LockOn",
        meta = (ClampMin = "100", ClampMax = "5000"))
    float LockOnMaxRange = 2000.0f;

    /** Arms mesh offset relative to the camera. Tune in editor for hand positioning. */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Camera")
    FVector ArmsMeshOffset = FVector(10.0f, 0.0f, -160.0f);

    // ------------------------------------------------------------------
    //  State
    // ------------------------------------------------------------------

    FLockOnState LockOnState;

    bool  bIsAiming      = false;
    float CurrentFOV     = 90.0f;
};
