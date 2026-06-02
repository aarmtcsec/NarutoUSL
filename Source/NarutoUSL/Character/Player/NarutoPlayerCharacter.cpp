// Copyright (c) 2025 Maze Runner Studios. All Rights Reserved.
// Naruto: Ultimate Shinobi Legacy
// NarutoPlayerCharacter — First-person implementation.
//
// Camera is attached directly to the head socket on the full-body skeletal mesh.
// The full body is hidden from the local player (SetOwnerNoSee) but visible to
// other clients, AI, and the shadow pass.
// A separate FP arms mesh (SetOnlyOwnerSee) is rendered in front of the camera
// on a near depth pass — this is what the player actually sees.

#include "Character/Player/NarutoPlayerCharacter.h"
#include "Combat/Core/CombatManager.h"
#include "Jutsu/Core/JutsuManager.h"
#include "Character/Components/JutsuComponent.h"
#include "Character/Components/CombatComponent.h"
#include "Character/Components/TransformationComponent.h"
#include "Core/GameInstance/NarutoGameInstance.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Kismet/KismetMathLibrary.h"
#include "NarutoUSL.h"

// ============================================================
//  Constructor
// ============================================================

ANarutoPlayerCharacter::ANarutoPlayerCharacter(
    const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    PrimaryActorTick.bCanEverTick = true;

    // ------------------------------------------------------------------
    //  Full-body mesh — hidden from the local player, visible to everyone else.
    //  Still exists for: shadows, AI line-of-sight, multiplayer, ragdoll.
    // ------------------------------------------------------------------
    GetMesh()->SetOwnerNoSee(true);
    GetMesh()->bCastHiddenShadow = true; // shadow still renders even though mesh is hidden

    // ------------------------------------------------------------------
    //  First-person camera — no spring arm.
    //  Attaches to the head socket; bUsePawnControlRotation drives look direction.
    // ------------------------------------------------------------------
    FirstPersonCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
    FirstPersonCamera->SetupAttachment(GetMesh(), HeadSocketName);
    FirstPersonCamera->SetRelativeLocation(EyeSocketOffset);
    FirstPersonCamera->bUsePawnControlRotation = true;
    FirstPersonCamera->FieldOfView             = FieldOfView;

    // ------------------------------------------------------------------
    //  FP Arms mesh — only the owning player sees this.
    //  Attaches to the camera so it moves with the view exactly.
    //  Rendered on depth stencil value 1 (set in material) to prevent
    //  clipping through walls.
    // ------------------------------------------------------------------
    FPArmsMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("FPArmsMesh"));
    FPArmsMesh->SetupAttachment(FirstPersonCamera);
    FPArmsMesh->SetRelativeLocation(ArmsMeshOffset);
    FPArmsMesh->SetOnlyOwnerSee(true);           // invisible to other players / AI
    FPArmsMesh->bCastDynamicShadow  = false;     // arms don't cast shadows
    FPArmsMesh->bReceivesDecals     = false;
    FPArmsMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    // ------------------------------------------------------------------
    //  Movement — standard FPS config.
    //  bOrientRotationToMovement = false so the capsule faces where the
    //  camera faces, not where we're walking.
    // ------------------------------------------------------------------
    bUseControllerRotationYaw   = true;   // capsule yaw = controller yaw
    bUseControllerRotationPitch = false;
    bUseControllerRotationRoll  = false;

    GetCharacterMovement()->bOrientRotationToMovement = false;
    GetCharacterMovement()->RotationRate              = FRotator(0.0f, 720.0f, 0.0f);
    GetCharacterMovement()->JumpZVelocity             = 700.0f;
    GetCharacterMovement()->AirControl                = 0.35f;
    GetCharacterMovement()->MaxWalkSpeed              = 600.0f;
    GetCharacterMovement()->GravityScale              = 1.75f;

    CurrentFOV = FieldOfView;
}

// ============================================================
//  BeginPlay
// ============================================================

void ANarutoPlayerCharacter::BeginPlay()
{
    Super::BeginPlay();

    // Register Enhanced Input mapping context
    if (APlayerController* PC = Cast<APlayerController>(GetController()))
    {
        if (UEnhancedInputLocalPlayerSubsystem* Sub =
            ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(
                PC->GetLocalPlayer()))
        {
            if (DefaultMappingContext)
            {
                Sub->AddMappingContext(DefaultMappingContext, 0);
            }
        }
    }

    // Register with CombatManager
    if (UNarutoGameInstance* GI = UNarutoGameInstance::Get(this))
    {
        if (UCombatManager* CM = GI->GetCombatManager())
        {
            CM->RegisterCombatant(this);
        }
    }

    // Snap FOV to default
    CurrentFOV = FieldOfView;
    if (FirstPersonCamera)
    {
        FirstPersonCamera->FieldOfView = CurrentFOV;
    }
}

// ============================================================
//  Tick
// ============================================================

void ANarutoPlayerCharacter::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    TickLockOnValidation(DeltaTime);
    UpdateLockOnCamera(DeltaTime);

    if (TransformationComponent)
    {
        TransformationComponent->TickTransformation(DeltaTime);
    }

    // Smoothly interpolate FOV between normal and aiming values
    if (FirstPersonCamera)
    {
        const float TargetFOV = bIsAiming ? AimingFOV : FieldOfView;
        CurrentFOV = FMath::FInterpTo(CurrentFOV, TargetFOV, DeltaTime, FOVInterpSpeed);
        FirstPersonCamera->FieldOfView = CurrentFOV;
    }
}

// ============================================================
//  Input Binding
// ============================================================

void ANarutoPlayerCharacter::SetupPlayerInputComponent(
    UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);

    UEnhancedInputComponent* EIC =
        Cast<UEnhancedInputComponent>(PlayerInputComponent);
    if (!EIC) return;

    if (IA_Move)           EIC->BindAction(IA_Move,           ETriggerEvent::Triggered, this, &ANarutoPlayerCharacter::Input_Move);
    if (IA_Look)           EIC->BindAction(IA_Look,           ETriggerEvent::Triggered, this, &ANarutoPlayerCharacter::Input_Look);
    if (IA_Jump)           EIC->BindAction(IA_Jump,           ETriggerEvent::Started,   this, &ANarutoPlayerCharacter::Input_Jump);
    if (IA_LightAttack)    EIC->BindAction(IA_LightAttack,    ETriggerEvent::Started,   this, &ANarutoPlayerCharacter::Input_LightAttack);
    if (IA_HeavyAttack)    EIC->BindAction(IA_HeavyAttack,    ETriggerEvent::Started,   this, &ANarutoPlayerCharacter::Input_HeavyAttack);
    if (IA_Block)          EIC->BindAction(IA_Block,          ETriggerEvent::Started,   this, &ANarutoPlayerCharacter::Input_BlockPressed);
    if (IA_Block)          EIC->BindAction(IA_Block,          ETriggerEvent::Completed, this, &ANarutoPlayerCharacter::Input_BlockReleased);
    if (IA_Substitution)   EIC->BindAction(IA_Substitution,   ETriggerEvent::Started,   this, &ANarutoPlayerCharacter::Input_Substitution);
    if (IA_LockOn)         EIC->BindAction(IA_LockOn,         ETriggerEvent::Started,   this, &ANarutoPlayerCharacter::Input_LockOn);
    if (IA_SwitchTargetRight) EIC->BindAction(IA_SwitchTargetRight, ETriggerEvent::Started, this, &ANarutoPlayerCharacter::Input_SwitchTargetRight);
    if (IA_SwitchTargetLeft)  EIC->BindAction(IA_SwitchTargetLeft,  ETriggerEvent::Started, this, &ANarutoPlayerCharacter::Input_SwitchTargetLeft);
    if (IA_JutsuSlot1)     EIC->BindAction(IA_JutsuSlot1,     ETriggerEvent::Started,   this, &ANarutoPlayerCharacter::Input_JutsuSlot1);
    if (IA_JutsuSlot2)     EIC->BindAction(IA_JutsuSlot2,     ETriggerEvent::Started,   this, &ANarutoPlayerCharacter::Input_JutsuSlot2);
    if (IA_JutsuSlot3)     EIC->BindAction(IA_JutsuSlot3,     ETriggerEvent::Started,   this, &ANarutoPlayerCharacter::Input_JutsuSlot3);
    if (IA_JutsuSlot4)     EIC->BindAction(IA_JutsuSlot4,     ETriggerEvent::Started,   this, &ANarutoPlayerCharacter::Input_JutsuSlot4);
    if (IA_Awakening)      EIC->BindAction(IA_Awakening,      ETriggerEvent::Started,   this, &ANarutoPlayerCharacter::Input_Awakening);
    if (IA_Dash)           EIC->BindAction(IA_Dash,           ETriggerEvent::Started,   this, &ANarutoPlayerCharacter::Input_Dash);
}

// ============================================================
//  Input Handlers
// ============================================================

void ANarutoPlayerCharacter::Input_Move(const FInputActionValue& Value)
{
    if (!IsAlive_Implementation()) return;

    const FVector2D MV = Value.Get<FVector2D>();

    // In first-person: forward/right are relative to controller yaw (where we're looking)
    if (Controller)
    {
        const FRotator YawOnly(0.0f, Controller->GetControlRotation().Yaw, 0.0f);
        AddMovementInput(FRotationMatrix(YawOnly).GetUnitAxis(EAxis::X), MV.Y);
        AddMovementInput(FRotationMatrix(YawOnly).GetUnitAxis(EAxis::Y), MV.X);
    }
}

void ANarutoPlayerCharacter::Input_Look(const FInputActionValue& Value)
{
    const FVector2D LV = Value.Get<FVector2D>();
    AddControllerYawInput(LV.X);
    AddControllerPitchInput(LV.Y);
}

void ANarutoPlayerCharacter::Input_Jump(const FInputActionValue& Value)
{
    Jump();
}

void ANarutoPlayerCharacter::Input_LightAttack(const FInputActionValue& Value)
{
    if (UNarutoGameInstance* GI = UNarutoGameInstance::Get(this))
    {
        if (UCombatManager* CM = GI->GetCombatManager())
        {
            CM->ProcessAttackInput(this, EComboInputType::LightAttack);
        }
    }
}

void ANarutoPlayerCharacter::Input_HeavyAttack(const FInputActionValue& Value)
{
    // Heavy attack also enters aiming mode briefly for visual feedback
    bIsAiming = true;

    if (UNarutoGameInstance* GI = UNarutoGameInstance::Get(this))
    {
        if (UCombatManager* CM = GI->GetCombatManager())
        {
            CM->ProcessAttackInput(this, EComboInputType::HeavyAttack);
        }
    }

    // Reset aiming after a short delay
    FTimerHandle Timer;
    GetWorldTimerManager().SetTimer(Timer, [this]()
    {
        bIsAiming = false;
    }, 0.4f, false);
}

void ANarutoPlayerCharacter::Input_BlockPressed(const FInputActionValue& Value)
{
    if (UNarutoGameInstance* GI = UNarutoGameInstance::Get(this))
    {
        if (UCombatManager* CM = GI->GetCombatManager())
        {
            CM->ProcessBlockInputPressed(this);
        }
    }
}

void ANarutoPlayerCharacter::Input_BlockReleased(const FInputActionValue& Value)
{
    if (UNarutoGameInstance* GI = UNarutoGameInstance::Get(this))
    {
        if (UCombatManager* CM = GI->GetCombatManager())
        {
            CM->ProcessBlockInputReleased(this);
        }
    }
}

void ANarutoPlayerCharacter::Input_Substitution(const FInputActionValue& Value)
{
    if (UNarutoGameInstance* GI = UNarutoGameInstance::Get(this))
    {
        if (UCombatManager* CM = GI->GetCombatManager())
        {
            CM->ProcessSubstitutionInput(this);
        }
    }
}

void ANarutoPlayerCharacter::Input_LockOn(const FInputActionValue& Value)
{
    ToggleLockOn();
}

void ANarutoPlayerCharacter::Input_SwitchTargetRight(const FInputActionValue& Value)
{
    SwitchLockOnTarget(true);
}

void ANarutoPlayerCharacter::Input_SwitchTargetLeft(const FInputActionValue& Value)
{
    SwitchLockOnTarget(false);
}

void ANarutoPlayerCharacter::Input_JutsuSlot1(const FInputActionValue& Value) { ActivateJutsuSlot(0); }
void ANarutoPlayerCharacter::Input_JutsuSlot2(const FInputActionValue& Value) { ActivateJutsuSlot(1); }
void ANarutoPlayerCharacter::Input_JutsuSlot3(const FInputActionValue& Value) { ActivateJutsuSlot(2); }
void ANarutoPlayerCharacter::Input_JutsuSlot4(const FInputActionValue& Value) { ActivateJutsuSlot(3); }

void ANarutoPlayerCharacter::Input_Awakening(const FInputActionValue& Value)
{
    if (!TransformationComponent) return;

    if (TransformationComponent->IsTransformed())
    {
        TransformationComponent->DeactivateTransformation();
        if (CombatComponent) CombatComponent->DeactivateAwakening();
    }
    else if (CombatComponent && CombatComponent->CanAwaken())
    {
        TransformationComponent->ActivateTransformation(0);
        CombatComponent->ActivateAwakening(0);

        // Zoom out FOV briefly on awakening for dramatic effect
        if (FirstPersonCamera)
        {
            FirstPersonCamera->FieldOfView = FieldOfView * 1.15f;
        }
    }
}

void ANarutoPlayerCharacter::Input_Dash(const FInputActionValue& Value)
{
    // FP dash: thrust in the direction the camera is facing
    const FVector DashDir = FirstPersonCamera
        ? FirstPersonCamera->GetForwardVector()
        : GetActorForwardVector();

    // Zero out Z so we don't fly upward unless jumping
    const FVector HorizontalDash = FVector(DashDir.X, DashDir.Y, 0.0f).GetSafeNormal();
    LaunchCharacter(HorizontalDash * 900.0f, true, false);
}

// ============================================================
//  Jutsu Slots
// ============================================================

void ANarutoPlayerCharacter::ActivateJutsuSlot(int32 SlotIndex)
{
    if (!JutsuComponent) return;

    const FGameplayTag JutsuTag = JutsuComponent->GetJutsuInSlot(SlotIndex);
    if (!JutsuTag.IsValid()) return;

    // In FP mode the aim location is a raycast from the center of the screen
    FVector AimLocation = FVector::ZeroVector;
    AActor* Target      = nullptr;

    if (FirstPersonCamera)
    {
        FHitResult Hit;
        const FVector Start = FirstPersonCamera->GetComponentLocation();
        const FVector End   = Start + FirstPersonCamera->GetForwardVector() * 5000.0f;

        FCollisionQueryParams Params;
        Params.AddIgnoredActor(this);

        if (GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_Pawn, Params))
        {
            AimLocation = Hit.ImpactPoint;
            Target      = Cast<AActor>(Hit.GetActor());

            // If we hit a non-character actor, treat the hit point as AoE origin
            if (!Cast<ANarutoCharacterBase>(Target))
            {
                Target = nullptr;
            }
        }
        else
        {
            AimLocation = End;
        }
    }

    // If locked on, override target
    if (LockOnState.IsValid())
    {
        Target      = LockOnState.Target.Get();
        AimLocation = Target->GetActorLocation();
    }

    if (UNarutoGameInstance* GI = UNarutoGameInstance::Get(this))
    {
        if (UJutsuManager* JM = GI->GetJutsuManager())
        {
            JM->RequestCast(this, JutsuTag, Target, AimLocation);
        }
    }
}

// ============================================================
//  Lock-On (FP version)
// ============================================================

void ANarutoPlayerCharacter::ToggleLockOn()
{
    if (LockOnState.IsValid())
    {
        LockOnState.bActive = false;
        LockOnState.Target  = nullptr;
        return;
    }

    ANarutoCharacterBase* Best = FindBestLockOnTarget();
    if (Best)
    {
        LockOnState.bActive = true;
        LockOnState.Target  = Best;
    }
}

void ANarutoPlayerCharacter::SwitchLockOnTarget(bool bRight)
{
    if (!LockOnState.IsValid()) return;

    TArray<FOverlapResult> Overlaps;
    FCollisionQueryParams Params;
    Params.AddIgnoredActor(this);

    GetWorld()->OverlapMultiByChannel(Overlaps, GetActorLocation(),
        FQuat::Identity, ECC_Pawn,
        FCollisionShape::MakeSphere(LockOnMaxRange), Params);

    TArray<ANarutoCharacterBase*> Candidates;
    for (const FOverlapResult& R : Overlaps)
    {
        ANarutoCharacterBase* Char = Cast<ANarutoCharacterBase>(R.GetActor());
        if (Char && Char != this && Char != LockOnState.Target.Get() &&
            Char->IsAlive_Implementation() &&
            IsHostileToFaction_Implementation(Char->GetPrimaryFaction_Implementation()))
        {
            Candidates.Add(Char);
        }
    }

    if (Candidates.IsEmpty()) return;

    const FVector CurrentDir =
        (LockOnState.Target->GetActorLocation() - GetActorLocation()).GetSafeNormal2D();

    Candidates.Sort([&](const ANarutoCharacterBase& A, const ANarutoCharacterBase& B)
    {
        const FVector DirA = (A.GetActorLocation() - GetActorLocation()).GetSafeNormal2D();
        const FVector DirB = (B.GetActorLocation() - GetActorLocation()).GetSafeNormal2D();
        const float CrossA = FVector::CrossProduct(CurrentDir, DirA).Z;
        const float CrossB = FVector::CrossProduct(CurrentDir, DirB).Z;
        return bRight ? (CrossA > CrossB) : (CrossA < CrossB);
    });

    LockOnState.Target = Candidates[0];
}

ANarutoCharacterBase* ANarutoPlayerCharacter::GetLockOnTarget() const
{
    return LockOnState.IsValid() ? LockOnState.Target.Get() : nullptr;
}

ANarutoCharacterBase* ANarutoPlayerCharacter::FindBestLockOnTarget() const
{
    TArray<FOverlapResult> Overlaps;
    FCollisionQueryParams Params;
    Params.AddIgnoredActor(this);

    GetWorld()->OverlapMultiByChannel(Overlaps, GetActorLocation(),
        FQuat::Identity, ECC_Pawn,
        FCollisionShape::MakeSphere(LockOnMaxRange), Params);

    ANarutoCharacterBase* Best    = nullptr;
    float                 BestDot = -1.0f;

    // In FP mode, "forward" is the camera's forward vector
    const FVector Forward = FirstPersonCamera
        ? FirstPersonCamera->GetForwardVector()
        : GetActorForwardVector();

    for (const FOverlapResult& R : Overlaps)
    {
        ANarutoCharacterBase* Char = Cast<ANarutoCharacterBase>(R.GetActor());
        if (!Char || Char == this || !Char->IsAlive_Implementation()) continue;
        if (!IsHostileToFaction_Implementation(
                Char->GetPrimaryFaction_Implementation())) continue;

        const FVector ToChar = (Char->GetActorLocation() -
                                 GetActorLocation()).GetSafeNormal();
        const float   Dot    = FVector::DotProduct(Forward, ToChar);

        if (Dot > BestDot)
        {
            BestDot = Dot;
            Best    = Char;
        }
    }

    return Best;
}

// ============================================================
//  Lock-On Camera (FP)
// ============================================================

void ANarutoPlayerCharacter::UpdateLockOnCamera(float DeltaTime)
{
    // In first-person, lock-on softly rotates the controller toward the target.
    // The camera follows because bUsePawnControlRotation = true.
    if (!LockOnState.IsValid()) return;

    const FVector MyLoc     = GetActorLocation();
    const FVector TargetLoc = LockOnState.Target->GetActorLocation();

    const FRotator LookRot = UKismetMathLibrary::FindLookAtRotation(MyLoc, TargetLoc);
    const FRotator Current = GetControlRotation();

    // Only rotate pitch and yaw — never roll
    const FRotator Interp = FMath::RInterpTo(
        Current,
        FRotator(LookRot.Pitch * 0.6f, LookRot.Yaw, 0.0f),
        DeltaTime,
        LockOnCameraInterpSpeed);

    if (APlayerController* PC = Cast<APlayerController>(GetController()))
    {
        PC->SetControlRotation(Interp);
    }
}

void ANarutoPlayerCharacter::TickLockOnValidation(float DeltaTime)
{
    if (!LockOnState.bActive) return;

    if (!LockOnState.IsValid())
    {
        LockOnState.bActive = false;
        return;
    }

    const float Dist = FVector::Dist(
        GetActorLocation(), LockOnState.Target->GetActorLocation());

    if (Dist > LockOnMaxRange * 1.5f)
    {
        LockOnState.bActive = false;
    }
}

// ============================================================
//  Overrides
// ============================================================

void ANarutoPlayerCharacter::OnCombatEntered_Implementation()
{
    Super::OnCombatEntered_Implementation();
}

void ANarutoPlayerCharacter::OnCombatExited_Implementation()
{
    Super::OnCombatExited_Implementation();
    LockOnState.bActive = false;
    bIsAiming           = false;
}

void ANarutoPlayerCharacter::OnDefeated_Implementation(ICombatant* Killer)
{
    Super::OnDefeated_Implementation(Killer);

    LockOnState.bActive = false;
    bIsAiming           = false;

    if (APlayerController* PC = Cast<APlayerController>(GetController()))
    {
        DisableInput(PC);
    }
}
