// Copyright (c) 2025 Maze Runner Studios. All Rights Reserved.

#include "Jutsu/Core/JutsuManager.h"
#include "Jutsu/Core/HandSealSystem.h"
#include "Character/Base/NarutoCharacterBase.h"
#include "Character/Components/JutsuComponent.h"
#include "Character/Components/ChakraComponent.h"
#include "Character/Components/CombatComponent.h"
#include "Character/Components/ProgressionComponent.h"
#include "Chakra/ChakraSystem.h"
#include "Combat/Core/CombatManager.h"
#include "Core/Events/NarutoEventBus.h"
#include "NarutoUSL.h"

UJutsuManager::UJutsuManager()
{
    SubsystemName = TEXT("JutsuManager");
    bTickEnabled  = true;
    TickPriority  = ESubsystemTickPriority::PostPhysics;
}

void UJutsuManager::Initialize(UChakraSystem* InChakraSystem,
                                 UCombatManager* InCombatManager,
                                 UNarutoEventBus* InEventBus)
{
    ChakraSystem  = InChakraSystem;
    CombatManager = InCombatManager;
    EventBus      = InEventBus;

    HandSealSystem = NewObject<UHandSealSystem>(this, TEXT("HandSealSystem"));
    HandSealSystem->Initialize();

    HandSealSystem->OnHandSealSequenceCompleted.AddUObject(
        this, &UJutsuManager::OnHandSealCompleted);

    bInitialized = true;
    UE_LOG(LogNarutoJutsu, Log, TEXT("[JutsuManager] Initialized."));
}

void UJutsuManager::Shutdown()
{
    ActiveCasts.Empty();
    if (HandSealSystem) HandSealSystem->Shutdown();
    UE_LOG(LogNarutoJutsu, Log, TEXT("[JutsuManager] Shutdown."));
}

// ============================================================
//  Main Tick
// ============================================================

void UJutsuManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (HandSealSystem) HandSealSystem->Tick(DeltaTime);
    TickActiveCasts(DeltaTime);
    TickChakraSystem(DeltaTime);
}

// ============================================================
//  Cast API
// ============================================================

uint32 UJutsuManager::RequestCast(ANarutoCharacterBase* Caster,
                                    FGameplayTag JutsuTag,
                                    AActor* PrimaryTarget,
                                    FVector AimLocation)
{
    if (!Caster || !JutsuTag.IsValid()) return 0;

    UJutsuComponent* JutsuComp = Caster->GetJutsuComponent();
    if (!JutsuComp) return 0;

    UJutsuData* Data = JutsuComp->GetJutsuDataByTag(JutsuTag);
    if (!Data) return 0;

    FString FailReason;
    if (!ValidateCastRequest(Caster, Data, FailReason))
    {
        UE_LOG(LogNarutoJutsu, Verbose,
            TEXT("[JutsuManager] Cast rejected for %s (%s): %s"),
            *Caster->GetName(), *JutsuTag.ToString(), *FailReason);
        return 0;
    }

    // Cancel any existing cast
    if (IsCasting(Caster)) CancelCast(Caster);

    // Consume chakra upfront
    UChakraComponent* ChakraComp = Caster->GetChakraComponent();
    if (ChakraComp && !ChakraComp->ConsumeCost(Data->ChakraCost))
    {
        UE_LOG(LogNarutoJutsu, Verbose,
            TEXT("[JutsuManager] %s cannot afford chakra cost for %s"),
            *Caster->GetName(), *JutsuTag.ToString());
        return 0;
    }

    // Create executor instance
    UClass* ExecutorClass = Data->ExecutorClass.IsNull()
        ? nullptr : Data->ExecutorClass.LoadSynchronous();

    UJutsuExecutorBase* Executor = ExecutorClass
        ? NewObject<UJutsuExecutorBase>(this, ExecutorClass)
        : nullptr;

    if (!Executor)
    {
        UE_LOG(LogNarutoJutsu, Warning,
            TEXT("[JutsuManager] No executor class for jutsu %s"), *JutsuTag.ToString());
        // Refund chakra
        if (ChakraComp) ChakraComp->RestoreChakra(
            ChakraComp->ResolveEffectiveCost(Data->ChakraCost));
        return 0;
    }

    // Build record
    FActiveCastRecord Record;
    Record.CastID    = NextCastID++;
    Record.Caster    = Caster;
    Record.JutsuData = Data;
    Record.Executor  = Executor;
    Record.Phase     = EJutsuExecutionPhase::HandSeals;

    BuildExecutionContext(Record, PrimaryTarget, AimLocation);

    // Notify caster
    Caster->OnJutsuCastBegin_Implementation(JutsuTag, Data);
    Executor->OnCastBegin(Record.Context);

    // Grant armor during cast if specified
    if (Data->bGrantsArmorDuringCast)
    {
        UCombatComponent* Combat = Caster->GetCombatComponent();
        if (Combat) Combat->AddFlag(ECombatFlags::HasSuperArmor);
    }

    // Begin hand seals
    const int32 MasteryLevel = Caster->GetProgressionComponent()
        ? Caster->GetProgressionComponent()->GetJutsuMasteryLevel(JutsuTag) : 0;

    HandSealSystem->BeginSequence(Caster, Data, MasteryLevel);

    ActiveCasts.Add(Record);
    ++TotalCastsThisSession;

    UE_LOG(LogNarutoJutsu, Log,
        TEXT("[JutsuManager] %s began casting %s (ID:%u)"),
        *Caster->GetName(), *Data->DisplayName.ToString(), Record.CastID);

    return Record.CastID;
}

void UJutsuManager::ReleaseCharge(ANarutoCharacterBase* Caster)
{
    FActiveCastRecord* Record = FindRecord(Caster);
    if (!Record || Record->Phase != EJutsuExecutionPhase::Charging) return;

    const UJutsuData* Data = Record->JutsuData.Get();
    if (!Data || !Data->bChargeable) return;

    Record->Context.ChargeFraction = Data->ChargeDuration > 0.0f
        ? FMath::Clamp(Record->ChargeTime / Data->ChargeDuration, 0.0f, 1.0f)
        : 1.0f;

    ActivateCast(*Record);
}

void UJutsuManager::InterruptCast(ANarutoCharacterBase* Caster)
{
    FActiveCastRecord* Record = FindRecord(Caster);
    if (!Record) return;

    const UJutsuData* Data = Record->JutsuData.Get();
    if (!Data || !Data->bInterruptible) return;

    if (HandSealSystem) HandSealSystem->InterruptSequence(
        Caster, Data->JutsuTag);

    if (Record->Executor) Record->Executor->OnInterrupted(Record->Context);

    Caster->OnJutsuInterrupted_Implementation(Data->JutsuTag, const_cast<UJutsuData*>(Data));

    // Remove armor
    if (Data->bGrantsArmorDuringCast)
    {
        UCombatComponent* Combat = Caster->GetCombatComponent();
        if (Combat) Combat->RemoveFlag(ECombatFlags::HasSuperArmor);
    }

    ActiveCasts.RemoveAll([Caster](const FActiveCastRecord& R)
    {
        return R.Caster.Get() == Caster;
    });

    ++TotalInterruptionsThisSession;
}

void UJutsuManager::CancelCast(ANarutoCharacterBase* Caster)
{
    if (HandSealSystem) HandSealSystem->CancelSequence(Caster);

    ActiveCasts.RemoveAll([Caster](const FActiveCastRecord& R)
    {
        return R.Caster.Get() == Caster;
    });
}

// ============================================================
//  Queries
// ============================================================

bool UJutsuManager::IsCasting(ANarutoCharacterBase* Caster) const
{
    return ActiveCasts.ContainsByPredicate([Caster](const FActiveCastRecord& R)
    {
        return R.Caster.Get() == Caster;
    });
}

EJutsuExecutionPhase UJutsuManager::GetCastPhase(ANarutoCharacterBase* Caster) const
{
    const FActiveCastRecord* Record = ActiveCasts.FindByPredicate(
        [Caster](const FActiveCastRecord& R) { return R.Caster.Get() == Caster; });
    return Record ? Record->Phase : EJutsuExecutionPhase::Idle;
}

float UJutsuManager::GetChargeProgress(ANarutoCharacterBase* Caster) const
{
    const FActiveCastRecord* Record = ActiveCasts.FindByPredicate(
        [Caster](const FActiveCastRecord& R) { return R.Caster.Get() == Caster; });

    if (!Record || Record->Phase != EJutsuExecutionPhase::Charging) return 0.0f;

    const UJutsuData* Data = Record->JutsuData.Get();
    return Data && Data->ChargeDuration > 0.0f
        ? FMath::Clamp(Record->ChargeTime / Data->ChargeDuration, 0.0f, 1.0f)
        : 0.0f;
}

// ============================================================
//  Internal Lifecycle
// ============================================================

bool UJutsuManager::ValidateCastRequest(ANarutoCharacterBase* Caster,
                                          UJutsuData* Data,
                                          FString& OutFailReason) const
{
    if (!Caster->IsAlive_Implementation())
    { OutFailReason = TEXT("Caster is dead"); return false; }

    UJutsuComponent* JutsuComp = Caster->GetJutsuComponent();
    if (!JutsuComp->CanCast(Data->JutsuTag))
    { OutFailReason = TEXT("CanCast returned false"); return false; }

    UCombatComponent* Combat = Caster->GetCombatComponent();
    if (Combat && Combat->IsInHitstun())
    { OutFailReason = TEXT("In hitstun"); return false; }

    return true;
}

void UJutsuManager::BuildExecutionContext(FActiveCastRecord& Record,
                                           AActor* PrimaryTarget,
                                           FVector AimLocation)
{
    ANarutoCharacterBase* Caster = Record.Caster.Get();
    UJutsuData* Data = Record.JutsuData.Get();

    Record.Context.Caster        = Caster;
    Record.Context.PrimaryTarget = PrimaryTarget;
    Record.Context.JutsuData     = Data;
    Record.Context.AimLocation   = AimLocation;
    Record.Context.AimDirection  = Caster->GetActorForwardVector();
    Record.Context.ChargeFraction = 1.0f;
    Record.Context.bIsAIControlled = !Caster->IsPlayerControlled();

    if (UProgressionComponent* Prog = Caster->GetProgressionComponent())
    {
        Record.Context.MasteryLevel =
            Prog->GetJutsuMasteryLevel(Data->JutsuTag);
        Record.Context.CasterChakraAttack =
            Prog->GetResolvedStat(ECharacterStat::ChakraAttack);
        Record.Context.CasterPhysicalAttack =
            Prog->GetResolvedStat(ECharacterStat::PhysicalAttack);
        Record.Context.CasterMasteryDamageMultiplier =
            Prog->GetJutsuDamageMultiplier(Data->JutsuTag);
        Record.Context.CasterMasteryCostMultiplier =
            Prog->GetJutsuCostMultiplier(Data->JutsuTag);
    }
}

void UJutsuManager::ActivateCast(FActiveCastRecord& Record)
{
    ANarutoCharacterBase* Caster = Record.Caster.Get();
    UJutsuData* Data = Record.JutsuData.Get();
    if (!Caster || !Data) return;

    Record.Phase = EJutsuExecutionPhase::Active;
    Record.Executor->OnActivated(Record.Context);
    Caster->OnJutsuActivated_Implementation(Data->JutsuTag, Data);

    if (Data->bChargeable && Record.Context.ChargeFraction < 1.0f)
    {
        // Already handled by ReleaseCharge
    }

    ExecuteInstantCast(Record);
}

void UJutsuManager::ExecuteInstantCast(FActiveCastRecord& Record)
{
    ANarutoCharacterBase* Caster = Record.Caster.Get();
    UJutsuData* Data = Record.JutsuData.Get();
    if (!Caster || !Data) return;

    const FJutsuExecutionResult Result =
        Record.Executor->Execute(Record.Context);

    // Gain mastery XP on successful cast
    if (Result.bSuccess && Caster->GetProgressionComponent())
    {
        Caster->GetProgressionComponent()->GainJutsuMastery(Data->JutsuTag, 5.0f);
    }

    // Publish event
    if (EventBus.IsValid())
    {
        FJutsuExecutedEvent Evt;
        Evt.Caster         = Caster;
        Evt.JutsuTag       = Data->JutsuTag;
        Evt.JutsuType      = Data->JutsuType;
        Evt.ChakraConsumed = ChakraSystem.IsValid()
            ? Caster->GetChakraComponent()->ResolveEffectiveCost(Data->ChakraCost) : 0.0f;
        Evt.Timestamp      = Caster->GetWorld()->GetTimeSeconds();
        EventBus->OnJutsuExecuted.Broadcast(Evt);
    }

    if (Data->bSustainable)
    {
        Record.Phase      = EJutsuExecutionPhase::Sustaining;
        Record.bSustaining = true;
    }
    else
    {
        CompleteCast(Record);
    }
}

void UJutsuManager::CompleteCast(FActiveCastRecord& Record)
{
    ANarutoCharacterBase* Caster = Record.Caster.Get();
    UJutsuData* Data = Record.JutsuData.Get();
    if (!Caster || !Data) return;

    Record.Executor->OnCompleted(Record.Context);
    Caster->OnJutsuCompleted_Implementation(Data->JutsuTag, Data);

    // Remove armor
    if (Data->bGrantsArmorDuringCast)
    {
        UCombatComponent* Combat = Caster->GetCombatComponent();
        if (Combat) Combat->RemoveFlag(ECombatFlags::HasSuperArmor);
    }

    StartCooldown(Record);

    ActiveCasts.RemoveAll([Caster](const FActiveCastRecord& R)
    {
        return R.Caster.Get() == Caster;
    });
}

void UJutsuManager::StartCooldown(FActiveCastRecord& Record)
{
    ANarutoCharacterBase* Caster = Record.Caster.Get();
    UJutsuData* Data = Record.JutsuData.Get();
    if (!Caster || !Data) return;

    UJutsuComponent* JutsuComp = Caster->GetJutsuComponent();
    if (!JutsuComp) return;

    const int32 MasteryLevel = Record.Context.MasteryLevel;
    const float EffectiveCooldown = Data->GetEffectiveCooldown(MasteryLevel);

    JutsuComp->StartCooldown(Data->JutsuTag, EffectiveCooldown);
    Caster->OnJutsuCooldownStarted_Implementation(Data->JutsuTag, EffectiveCooldown);
}

// ============================================================
//  Tick Stages
// ============================================================

void UJutsuManager::TickActiveCasts(float DeltaTime)
{
    for (int32 i = ActiveCasts.Num() - 1; i >= 0; --i)
    {
        FActiveCastRecord& Record = ActiveCasts[i];
        if (!Record.IsValid())
        {
            ActiveCasts.RemoveAt(i);
            continue;
        }

        ANarutoCharacterBase* Caster = Record.Caster.Get();
        UJutsuData* Data = Record.JutsuData.Get();

        switch (Record.Phase)
        {
            case EJutsuExecutionPhase::HandSeals:
                // Driven by HandSealSystem — nothing to tick here
                break;

            case EJutsuExecutionPhase::Charging:
            {
                Record.ChargeTime += DeltaTime;
                if (Record.ChargeTime >= Data->ChargeDuration)
                {
                    // Auto-release at max charge
                    Record.Context.ChargeFraction = 1.0f;
                    ActivateCast(Record);
                }
                break;
            }

            case EJutsuExecutionPhase::Active:
            {
                Record.PhaseTimer += DeltaTime;
                if (Data->ActiveDuration > 0.0f &&
                    Record.PhaseTimer >= Data->ActiveDuration)
                {
                    CompleteCast(Record);
                }
                break;
            }

            case EJutsuExecutionPhase::Sustaining:
            {
                // Drain sustain chakra cost
                UChakraComponent* Chakra = Caster->GetChakraComponent();
                if (Chakra)
                {
                    const float DrainAmount = Data->SustainCostPerSecond * DeltaTime;
                    if (Chakra->GetCurrentChakra() < DrainAmount)
                    {
                        // Out of chakra — end sustain
                        CompleteCast(Record);
                        break;
                    }
                    Chakra->ConsumeChakra(DrainAmount);
                }

                Record.Executor->OnSustainTick(Record.Context, DeltaTime);
                break;
            }

            default: break;
        }

        // Tick cooldowns on the jutsu component
        if (UJutsuComponent* JutsuComp = Caster->GetJutsuComponent())
        {
            JutsuComp->TickCooldowns(DeltaTime);
        }
    }

    // Also tick cooldowns for non-casting characters
    // (handled by their JutsuComponent directly in a full implementation)
}

void UJutsuManager::TickChakraSystem(float DeltaTime)
{
    // ChakraSystem ticks all registered ChakraComponents
    // This is called here to ensure chakra regen happens after jutsu costs
    if (ChakraSystem.IsValid())
    {
        ChakraSystem->Tick(DeltaTime);
    }
}

// ============================================================
//  Event Handlers
// ============================================================

void UJutsuManager::OnHandSealCompleted(ANarutoCharacterBase* Caster, bool bSuccess)
{
    FActiveCastRecord* Record = FindRecord(Caster);
    if (!Record) return;

    if (!bSuccess)
    {
        InterruptCast(Caster);
        return;
    }

    const UJutsuData* Data = Record->JutsuData.Get();
    if (!Data) return;

    if (Data->bChargeable)
    {
        Record->Phase     = EJutsuExecutionPhase::Charging;
        Record->ChargeTime = 0.0f;
    }
    else
    {
        ActivateCast(*Record);
    }
}

FActiveCastRecord* UJutsuManager::FindRecord(ANarutoCharacterBase* Caster)
{
    return ActiveCasts.FindByPredicate([Caster](const FActiveCastRecord& R)
    {
        return R.Caster.Get() == Caster;
    });
}

TMap<FString, FString> UJutsuManager::GetDebugInfo() const
{
    TMap<FString, FString> Info = Super::GetDebugInfo();
    Info.Add(TEXT("ActiveCasts"),       FString::FromInt(ActiveCasts.Num()));
    Info.Add(TEXT("TotalCasts"),        FString::FromInt(TotalCastsThisSession));
    Info.Add(TEXT("TotalInterruptions"), FString::FromInt(TotalInterruptionsThisSession));
    return Info;
}
