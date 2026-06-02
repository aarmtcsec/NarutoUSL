// Copyright (c) 2025 Maze Runner Studios. All Rights Reserved.

#include "Combat/Core/CombatManager.h"
#include "Combat/Core/HitboxManager.h"
#include "Combat/Combos/ComboSystem.h"
#include "Combat/Counters/SubstitutionSystem.h"
#include "Combat/Counters/ParrySystem.h"
#include "Combat/Damage/DamageCalculator.h"
#include "Character/Base/NarutoCharacterBase.h"
#include "Character/Components/CombatComponent.h"
#include "Character/Components/HealthComponent.h"
#include "Character/Components/ProgressionComponent.h"
#include "Character/Data/NarutoCharacterData.h"
#include "Core/Events/NarutoEventBus.h"
#include "Core/Settings/NarutoGameSettings.h"
#include "NarutoUSL.h"

UCombatManager::UCombatManager()
{
    SubsystemName  = TEXT("CombatManager");
    bTickEnabled   = true;
    TickPriority   = ESubsystemTickPriority::Physics;
}

// ============================================================
//  Initialization
// ============================================================

void UCombatManager::Initialize(UChakraSystem* InChakraSystem, UNarutoEventBus* InEventBus)
{
    ChakraSystem = InChakraSystem;
    EventBus     = InEventBus;

    // Create owned subsystems
    HitboxManager     = NewObject<UHitboxManager>(this,     TEXT("HitboxManager"));
    ComboSystem       = NewObject<UComboSystem>(this,       TEXT("ComboSystem"));
    SubstitutionSystem = NewObject<USubstitutionSystem>(this, TEXT("SubstitutionSystem"));
    ParrySystem       = NewObject<UParrySystem>(this,       TEXT("ParrySystem"));

    // Initialize subsystems
    if (GetOuter() && GetOuter()->GetWorld())
    {
        HitboxManager->Initialize(GetOuter()->GetWorld());
    }
    ComboSystem->Initialize();
    SubstitutionSystem->Initialize();
    ParrySystem->Initialize();

    // Bind combo events
    ComboSystem->OnComboNodeActivated.AddUObject(
        this, &UCombatManager::OnComboNodeActivated);

    bInitialized = true;
    UE_LOG(LogNarutoCombat, Log, TEXT("[CombatManager] Initialized."));
}

void UCombatManager::Shutdown()
{
    if (HitboxManager)     HitboxManager->Shutdown();
    if (ComboSystem)       ComboSystem->Shutdown();
    if (SubstitutionSystem) SubstitutionSystem->Shutdown();
    if (ParrySystem)       ParrySystem->Shutdown();

    ActiveCombatants.Empty();
    UE_LOG(LogNarutoCombat, Log, TEXT("[CombatManager] Shutdown."));
}

// ============================================================
//  Main Tick
// ============================================================

void UCombatManager::Tick(float DeltaTime)
{
    if (bCombatPaused || !bInitialized) return;

    Super::Tick(DeltaTime);

    // Stage 1: Advance attack frames
    TickAttackFrames(DeltaTime);

    // Stage 2: Evaluate hitboxes and resolve hits
    TickHitboxEvaluation();

    // Stage 3: Tick timers and state machines
    TickComboTimers(DeltaTime);
    TickSubstitutionCooldowns(DeltaTime);
    TickGuardRecovery(DeltaTime);
    TickHitstun();
    TickParryWindows(DeltaTime);
}

// ============================================================
//  Combatant Registration
// ============================================================

void UCombatManager::RegisterCombatant(ANarutoCharacterBase* Character)
{
    if (!Character || IsCombatantRegistered(Character)) return;

    FActiveCombatantRecord Record;
    Record.Character = Character;
    ActiveCombatants.Add(Record);

    // Register default hurtboxes
    TArray<FHurtboxVolume> DefaultHurtboxes;

    FHurtboxVolume Torso;
    Torso.ZoneName   = TEXT("Torso");
    Torso.AttachBone = TEXT("spine_02");
    Torso.HalfExtent = FVector(25.0f, 20.0f, 35.0f);
    Torso.ZoneDamageMultiplier = 1.0f;
    DefaultHurtboxes.Add(Torso);

    FHurtboxVolume Head;
    Head.ZoneName   = TEXT("Head");
    Head.AttachBone = TEXT("head");
    Head.HalfExtent = FVector(15.0f, 15.0f, 15.0f);
    Head.ZoneDamageMultiplier = 1.5f;
    DefaultHurtboxes.Add(Head);

    FHurtboxVolume Legs;
    Legs.ZoneName   = TEXT("Legs");
    Legs.AttachBone = TEXT("thigh_l");
    Legs.HalfExtent = FVector(15.0f, 15.0f, 30.0f);
    Legs.ZoneDamageMultiplier = 0.8f;
    DefaultHurtboxes.Add(Legs);

    HitboxManager->RegisterHurtboxes(Character, DefaultHurtboxes);

    // Register combo graph if character has one
    if (Character->GetCharacterData() && Character->GetCharacterData()->CharacterTag.IsValid())
    {
        // Combo graph is loaded by the character's JutsuComponent
        // and passed here via RegisterCombatant overload in full implementation
    }

    UE_LOG(LogNarutoCombat, Log, TEXT("[CombatManager] Registered combatant: %s"),
        *Character->GetName());
}

void UCombatManager::UnregisterCombatant(ANarutoCharacterBase* Character)
{
    ActiveCombatants.RemoveAll([Character](const FActiveCombatantRecord& R)
    {
        return R.Character.Get() == Character;
    });

    HitboxManager->DeregisterAllHitboxesForOwner(Character);
    ComboSystem->UnregisterCombatant(Character);

    UE_LOG(LogNarutoCombat, Log, TEXT("[CombatManager] Unregistered combatant: %s"),
        *Character->GetName());
}

bool UCombatManager::IsCombatantRegistered(ANarutoCharacterBase* Character) const
{
    return ActiveCombatants.ContainsByPredicate([Character](const FActiveCombatantRecord& R)
    {
        return R.Character.Get() == Character;
    });
}

// ============================================================
//  Attack Execution
// ============================================================

void UCombatManager::BeginAttack(ANarutoCharacterBase* Character, const FComboNode& Node)
{
    if (!Character) return;

    UCombatComponent* Combat = Character->GetCombatComponent();
    if (!Combat) return;

    // Set frame data on the combat component
    Combat->BeginAttack(Node.FrameData);

    // Register hitboxes
    for (const FHitboxData& Hitbox : Node.Hitboxes)
    {
        HitboxManager->RegisterHitbox(Character, Hitbox, 0);
    }

    // Update record
    FActiveCombatantRecord* Record = FindRecord(Character);
    if (Record)
    {
        Record->bIsAttacking      = true;
        Record->CurrentAttackFrame = 0;
        Record->FrameAccumulator  = 0.0f;
    }

    UE_LOG(LogNarutoCombat, Verbose,
        TEXT("[CombatManager] %s began attack: %s"),
        *Character->GetName(), *Node.NodeID.ToString());
}

void UCombatManager::CancelAttack(ANarutoCharacterBase* Character)
{
    if (!Character) return;

    UCombatComponent* Combat = Character->GetCombatComponent();
    if (Combat) Combat->EndAttack();

    HitboxManager->DeregisterAllHitboxesForOwner(Character);
    ComboSystem->OnAttackInterrupted(Character);

    FActiveCombatantRecord* Record = FindRecord(Character);
    if (Record)
    {
        Record->bIsAttacking       = false;
        Record->CurrentAttackFrame = -1;
    }
}

// ============================================================
//  Input Processing
// ============================================================

void UCombatManager::ProcessAttackInput(ANarutoCharacterBase* Character, EComboInputType InputType)
{
    if (!Character || bCombatPaused) return;

    UCombatComponent* Combat = Character->GetCombatComponent();
    if (!Combat || Combat->IsInHitstun()) return;

    const FComboNode* Node = ComboSystem->ProcessInput(Character, InputType);
    if (Node)
    {
        BeginAttack(Character, *Node);
    }
}

void UCombatManager::ProcessBlockInputPressed(ANarutoCharacterBase* Character)
{
    if (!Character) return;

    UCombatComponent* Combat = Character->GetCombatComponent();
    if (Combat && !Combat->IsGuardBroken())
    {
        Combat->AddFlag(ECombatFlags::CanBlock);
        ParrySystem->OnBlockInputPressed(Character);
    }
}

void UCombatManager::ProcessBlockInputReleased(ANarutoCharacterBase* Character)
{
    if (!Character) return;

    UCombatComponent* Combat = Character->GetCombatComponent();
    if (Combat)
    {
        Combat->RemoveFlag(ECombatFlags::CanBlock);
        ParrySystem->OnBlockInputReleased(Character);
    }
}

void UCombatManager::ProcessSubstitutionInput(ANarutoCharacterBase* Character)
{
    // Substitution is reactive — it's triggered by TryExecuteSubstitution
    // when a hit is incoming. This input just sets a flag that the
    // SubstitutionSystem checks during hit resolution.
    if (!Character) return;

    UCombatComponent* Combat = Character->GetCombatComponent();
    if (Combat && Combat->CanSubstitute())
    {
        Combat->AddFlag(ECombatFlags::CanSubstitute);
    }
}

// ============================================================
//  Damage Pipeline
// ============================================================

void UCombatManager::ResolveDamageEvent(ANarutoCharacterBase* Attacker,
                                         ANarutoCharacterBase* Defender,
                                         const FHitboxData& HitboxData,
                                         const FComboNode* SourceNode)
{
    if (!Attacker || !Defender) return;
    if (Defender->IsDead_Implementation()) return;

    // 1. Check substitution
    FNarutoDamageEvent PreEvent;
    PreEvent.DamageType   = HitboxData.DamageType;
    PreEvent.Instigator   = Attacker;
    PreEvent.Target       = Defender;
    PreEvent.SourceJutsuTag = SourceNode ? FGameplayTag() : FGameplayTag();

    if (SubstitutionSystem->TryExecuteSubstitution(Defender, Attacker, PreEvent))
    {
        ++TotalSubstitutions;
        return; // Hit negated
    }

    // 2. Evaluate parry
    const EParryResult ParryResult =
        ParrySystem->EvaluateIncomingHit(Defender, Attacker, PreEvent);

    // 3. Build damage resolution context
    FDamageResolutionContext Context;
    Context.Attacker          = Attacker;
    Context.Defender          = Defender;
    Context.RawDamage         = HitboxData.DamageMultiplier *
                                (SourceNode ? SourceNode->BaseDamage : 50.0f) *
                                DifficultyScale;
    Context.DamageType        = HitboxData.DamageType;
    Context.HitboxData        = HitboxData;
    Context.bDefenderIsBlocking = (ParryResult == EParryResult::NormalBlock ||
                                   ParryResult == EParryResult::PerfectGuard);
    Context.bIsPerfectGuard   = (ParryResult == EParryResult::PerfectGuard);
    Context.bIsParry          = (ParryResult == EParryResult::Parry);
    Context.bAttackerAwakened = Attacker->HasCombatFlag_Implementation(ECombatFlags::IsAwakened);
    Context.ComboHitIndex     = Attacker->GetCombatComponent()
                                    ? Attacker->GetCombatComponent()->GetComboCount() : 0;
    Context.ExternalMultiplier = 1.0f;

    if (Context.bAttackerAwakened)
    {
        // Apply awakening attack multiplier from character data
        if (Attacker->GetCharacterData() &&
            Attacker->GetCharacterData()->Awakenings.IsValidIndex(
                Attacker->GetCombatComponent()->GetCurrentAwakeningIndex()))
        {
            const FAwakeningData& Awakening = Attacker->GetCharacterData()->Awakenings[
                Attacker->GetCombatComponent()->GetCurrentAwakeningIndex()];
            Context.ExternalMultiplier = Awakening.AttackMultiplier;
        }
    }

    // 4. Resolve damage
    const FDamageResolutionResult Result = UDamageCalculator::Resolve(Context);

    // 5. Apply parry rewards (stagger attacker, grant counter window)
    if (ParryResult == EParryResult::Parry || ParryResult == EParryResult::PerfectGuard)
    {
        ++TotalParries;
        Attacker->OnParrySuccessful_Implementation(Cast<ICombatant>(Defender), Result.ResolvedEvent);
        Defender->OnParrySuccessful_Implementation(Cast<ICombatant>(Attacker), Result.ResolvedEvent);
        return;
    }

    // 6. Apply damage
    if (Result.FinalDamage > 0.0f)
    {
        Defender->OnHitReceived_Implementation(Cast<ICombatant>(Attacker), Result.ResolvedEvent);
        Attacker->OnHitLanded_Implementation(Cast<ICombatant>(Defender), Result.ResolvedEvent);

        // Apply hitstun
        if (!Result.bIsBlocked && !Result.bIsParried)
        {
            const int32 HitstunFrames = HitboxData.DamageType != EDamageType::True
                ? FMath::RoundToInt(Result.FinalDamage * 0.1f + 10.0f)
                : 0;

            if (HitstunFrames > 0)
            {
                Defender->ApplyHitstun_Implementation(HitstunFrames);
                CancelAttack(Defender);
            }

            // Apply knockback
            if (HitboxData.KnockbackForce > 0.0f)
            {
                Defender->ApplyKnockback_Implementation(
                    HitboxData.KnockbackDirection, HitboxData.KnockbackForce);
            }

            // Apply launch
            if (HitboxData.bLaunches)
            {
                Defender->ApplyLaunch_Implementation(
                    FVector(0.0f, 0.0f, HitboxData.KnockbackForce * 1.5f));
            }
        }

        // Increment combo
        if (UCombatComponent* AttackerCombat = Attacker->GetCombatComponent())
        {
            AttackerCombat->IncrementCombo();
        }

        ++TotalHitsThisSession;
        TotalDamageThisSession += FMath::RoundToInt(Result.FinalDamage);

        // Publish jutsu executed event
        if (EventBus.IsValid() && Context.SourceJutsuTag.IsValid())
        {
            FJutsuExecutedEvent JutsuEvt;
            JutsuEvt.Caster    = Attacker;
            JutsuEvt.JutsuTag  = Context.SourceJutsuTag;
            JutsuEvt.Timestamp = Attacker->GetWorld()->GetTimeSeconds();
            EventBus->OnJutsuExecuted.Broadcast(JutsuEvt);
        }
    }

#if !UE_BUILD_SHIPPING
    UE_LOG(LogNarutoCombat, Verbose,
        TEXT("[CombatManager] Hit: %s → %s | Dmg: %.1f%s%s"),
        *Attacker->GetName(), *Defender->GetName(),
        Result.FinalDamage,
        Result.bIsCritical ? TEXT(" [CRIT]") : TEXT(""),
        Result.bIsBlocked  ? TEXT(" [BLOCKED]") : TEXT(""));
#endif
}

// ============================================================
//  Tick Stages
// ============================================================

void UCombatManager::TickAttackFrames(float DeltaTime)
{
    const UNarutoGameSettings* Settings = GetDefault<UNarutoGameSettings>();
    const float FrameDuration = 1.0f / static_cast<float>(Settings->CombatFrameRate);

    for (FActiveCombatantRecord& Record : ActiveCombatants)
    {
        if (!Record.IsValid() || !Record.bIsAttacking) continue;

        ANarutoCharacterBase* Character = Record.Character.Get();
        UCombatComponent* Combat = Character->GetCombatComponent();
        if (!Combat) continue;

        Record.FrameAccumulator += DeltaTime;

        // Advance frames at the configured combat frame rate
        while (Record.FrameAccumulator >= FrameDuration)
        {
            Record.FrameAccumulator -= FrameDuration;

            const bool bStillActive = Combat->AdvanceAttackFrame();

            // Notify combo system of cancel window state
            if (Combat->IsInCancelWindow())
            {
                ComboSystem->OnCancelWindowOpened(Character);
            }

            if (!bStillActive)
            {
                Record.bIsAttacking = false;
                HitboxManager->DeregisterAllHitboxesForOwner(Character);
                break;
            }
        }

        // Tick hitstun
        Combat->TickHitstun();
    }
}

void UCombatManager::TickHitboxEvaluation()
{
    auto HitResults = HitboxManager->EvaluateAllHitboxes();

    for (auto& HitPair : HitResults)
    {
        FActiveHitbox* Hitbox = HitPair.Key;
        const FHitQueryResult& QueryResult = HitPair.Value;

        if (!Hitbox || !Hitbox->Owner.IsValid()) continue;

        ANarutoCharacterBase* Attacker = Hitbox->Owner.Get();
        ANarutoCharacterBase* Defender = Cast<ANarutoCharacterBase>(QueryResult.HitActor.Get());

        if (!Defender) continue;

        // Apply zone damage multiplier to hitbox
        FHitboxData ModifiedHitbox = Hitbox->HitboxData;
        ModifiedHitbox.DamageMultiplier *= QueryResult.ZoneMultiplier;

        // Find the source combo node
        const FComboNode* SourceNode = ComboSystem->GetCurrentNode(Attacker);

        ResolveDamageEvent(Attacker, Defender, ModifiedHitbox, SourceNode);
    }
}

void UCombatManager::TickComboTimers(float DeltaTime)
{
    for (FActiveCombatantRecord& Record : ActiveCombatants)
    {
        if (!Record.IsValid()) continue;
        UCombatComponent* Combat = Record.Character->GetCombatComponent();
        if (Combat) Combat->TickComboTimer(DeltaTime);
    }
}

void UCombatManager::TickSubstitutionCooldowns(float DeltaTime)
{
    for (FActiveCombatantRecord& Record : ActiveCombatants)
    {
        if (!Record.IsValid()) continue;
        UCombatComponent* Combat = Record.Character->GetCombatComponent();
        if (Combat) Combat->TickSubstitutionCooldown(DeltaTime);
    }
}

void UCombatManager::TickGuardRecovery(float DeltaTime)
{
    for (FActiveCombatantRecord& Record : ActiveCombatants)
    {
        if (!Record.IsValid()) continue;
        UCombatComponent* Combat = Record.Character->GetCombatComponent();
        if (Combat) Combat->TickGuardRecovery(DeltaTime);
    }
}

void UCombatManager::TickHitstun()
{
    // Hitstun is ticked per-frame in TickAttackFrames via Combat->TickHitstun()
}

void UCombatManager::TickParryWindows(float DeltaTime)
{
    if (ParrySystem) ParrySystem->Tick(DeltaTime);
}

// ============================================================
//  Configuration
// ============================================================

void UCombatManager::SetDifficultyScale(float Scale)
{
    DifficultyScale = FMath::Clamp(Scale, 0.1f, 5.0f);
}

void UCombatManager::SetCombatPaused(bool bPaused)
{
    bCombatPaused = bPaused;
}

// ============================================================
//  Internal
// ============================================================

FActiveCombatantRecord* UCombatManager::FindRecord(ANarutoCharacterBase* Character)
{
    return ActiveCombatants.FindByPredicate([Character](const FActiveCombatantRecord& R)
    {
        return R.Character.Get() == Character;
    });
}

void UCombatManager::OnComboNodeActivated(ANarutoCharacterBase* Character,
                                           const FComboNode& Node, int32 ComboIndex)
{
    // Called by ComboSystem when a node is activated via input.
    // BeginAttack is called from ProcessAttackInput, not here,
    // to avoid double-activation.
}

void UCombatManager::PublishCombatEncounterEvent(bool bStarted)
{
    if (!EventBus.IsValid()) return;

    FCombatEncounterEvent Evt;
    Evt.bCombatStarted = bStarted;
    EventBus->OnCombatEncounterChanged.Broadcast(Evt);
}

// ============================================================
//  Debug
// ============================================================

TMap<FString, FString> UCombatManager::GetDebugInfo() const
{
    TMap<FString, FString> Info = Super::GetDebugInfo();
    Info.Add(TEXT("ActiveCombatants"),    FString::FromInt(ActiveCombatants.Num()));
    Info.Add(TEXT("TotalHits"),           FString::FromInt(TotalHitsThisSession));
    Info.Add(TEXT("TotalDamage"),         FString::FromInt(TotalDamageThisSession));
    Info.Add(TEXT("TotalSubstitutions"),  FString::FromInt(TotalSubstitutions));
    Info.Add(TEXT("TotalParries"),        FString::FromInt(TotalParries));
    Info.Add(TEXT("DifficultyScale"),     FString::SanitizeFloat(DifficultyScale));
    Info.Add(TEXT("Paused"),              bCombatPaused ? TEXT("YES") : TEXT("NO"));
    return Info;
}
