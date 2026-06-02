// Copyright (c) 2025 Maze Runner Studios. All Rights Reserved.

#include "Character/Base/NarutoCharacterBase.h"
#include "Character/Components/HealthComponent.h"
#include "Character/Components/ChakraComponent.h"
#include "Character/Components/CombatComponent.h"
#include "Character/Components/ProgressionComponent.h"
#include "Character/Components/JutsuComponent.h"
#include "Character/Components/TransformationComponent.h"
#include "Character/Components/InventoryComponent.h"
#include "Character/Components/EquipmentComponent.h"
#include "Character/Components/ReputationComponent.h"
#include "Character/Data/NarutoCharacterData.h"
#include "Core/GameInstance/NarutoGameInstance.h"
#include "Core/Events/NarutoEventBus.h"
#include "MotionWarpingComponent.h"
#include "NarutoUSL.h"

int32 ANarutoCharacterBase::NextCombatantID = 1;

// ============================================================
//  Constructor
// ============================================================

ANarutoCharacterBase::ANarutoCharacterBase(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    PrimaryActorTick.bCanEverTick = true;

    // Assign a unique runtime ID
    CombatantID = NextCombatantID++;

    // Create all components — order matters for dependency resolution
    HealthComponent         = CreateDefaultSubobject<UHealthComponent>(TEXT("HealthComponent"));
    ChakraComponent         = CreateDefaultSubobject<UChakraComponent>(TEXT("ChakraComponent"));
    CombatComponent         = CreateDefaultSubobject<UCombatComponent>(TEXT("CombatComponent"));
    ProgressionComponent    = CreateDefaultSubobject<UProgressionComponent>(TEXT("ProgressionComponent"));
    JutsuComponent          = CreateDefaultSubobject<UJutsuComponent>(TEXT("JutsuComponent"));
    TransformationComponent = CreateDefaultSubobject<UTransformationComponent>(TEXT("TransformationComponent"));
    InventoryComponent      = CreateDefaultSubobject<UInventoryComponent>(TEXT("InventoryComponent"));
    EquipmentComponent      = CreateDefaultSubobject<UEquipmentComponent>(TEXT("EquipmentComponent"));
    ReputationComponent     = CreateDefaultSubobject<UReputationComponent>(TEXT("ReputationComponent"));
    MotionWarpingComponent  = CreateDefaultSubobject<UMotionWarpingComponent>(TEXT("MotionWarpingComponent"));
}

// ============================================================
//  ACharacter Interface
// ============================================================

void ANarutoCharacterBase::BeginPlay()
{
    Super::BeginPlay();

    if (CharacterData && !bIsInitialized)
    {
        InitializeFromData(CharacterData);
    }
}

void ANarutoCharacterBase::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (!bIsInitialized) return;

    // Tick stat modifier durations
    if (ProgressionComponent)
    {
        ProgressionComponent->TickStatModifiers(DeltaTime);
    }
}

void ANarutoCharacterBase::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    Super::EndPlay(EndPlayReason);
}

// ============================================================
//  Initialization
// ============================================================

void ANarutoCharacterBase::InitializeFromData(UNarutoCharacterData* InCharacterData)
{
    if (!InCharacterData)
    {
        UE_LOG(LogNarutoUSL, Error, TEXT("[NarutoCharacterBase] InitializeFromData called with null data on %s"),
            *GetName());
        return;
    }

    CharacterData = InCharacterData;

    ApplyCharacterDataToComponents();
    BindComponentEvents();

    bIsInitialized = true;

    UE_LOG(LogNarutoUSL, Log, TEXT("[NarutoCharacterBase] %s initialized from data asset: %s"),
        *GetName(), *InCharacterData->DisplayName.ToString());

    OnCharacterInitialized.Broadcast(this);
}

void ANarutoCharacterBase::ApplyCharacterDataToComponents()
{
    if (!CharacterData) return;

    const FCharacterBaseStats& Stats = CharacterData->BaseStats;

    // Health
    HealthComponent->InitializeHealth(Stats.MaxHealth);
    HealthComponent->SetRegenRate(Stats.HealthRegenRate);

    // Chakra
    ChakraComponent->InitializeChakra(
        Stats.MaxChakra,
        CharacterData->NatureAffinities != 0
            ? static_cast<EChakraNature>(CharacterData->NatureAffinities)
            : EChakraNature::None,
        Stats.ChakraRegenRate
    );

    // Combat
    CombatComponent->InitializeCombat(
        GetDefault<UNarutoGameSettings>()->MaxSubstitutionsPerCombat,
        CharacterData->Awakenings.Num() > 0
    );

    // Progression — build stat maps from CharacterData
    TMap<ECharacterStat, float> BaseStatMap;
    BaseStatMap.Add(ECharacterStat::MaxHealth,          Stats.MaxHealth);
    BaseStatMap.Add(ECharacterStat::MaxChakra,          Stats.MaxChakra);
    BaseStatMap.Add(ECharacterStat::MaxStamina,         Stats.MaxStamina);
    BaseStatMap.Add(ECharacterStat::PhysicalAttack,     Stats.PhysicalAttack);
    BaseStatMap.Add(ECharacterStat::ChakraAttack,       Stats.ChakraAttack);
    BaseStatMap.Add(ECharacterStat::PhysicalDefense,    Stats.PhysicalDefense);
    BaseStatMap.Add(ECharacterStat::ChakraDefense,      Stats.ChakraDefense);
    BaseStatMap.Add(ECharacterStat::Speed,              Stats.MovementSpeed);
    BaseStatMap.Add(ECharacterStat::ChakraRegen,        Stats.ChakraRegenRate);
    BaseStatMap.Add(ECharacterStat::HealthRegen,        Stats.HealthRegenRate);
    BaseStatMap.Add(ECharacterStat::CriticalChance,     Stats.CriticalChance);
    BaseStatMap.Add(ECharacterStat::CriticalMultiplier, Stats.CriticalMultiplier);

    const FCharacterLevelScaling& Scaling = CharacterData->LevelScaling;
    TMap<ECharacterStat, float> ScalingMap;
    ScalingMap.Add(ECharacterStat::MaxHealth,       Scaling.HealthPerLevel);
    ScalingMap.Add(ECharacterStat::MaxChakra,       Scaling.ChakraPerLevel);
    ScalingMap.Add(ECharacterStat::PhysicalAttack,  Scaling.PhysicalAttackPerLevel);
    ScalingMap.Add(ECharacterStat::ChakraAttack,    Scaling.ChakraAttackPerLevel);
    ScalingMap.Add(ECharacterStat::PhysicalDefense, Scaling.PhysicalDefensePerLevel);
    ScalingMap.Add(ECharacterStat::ChakraDefense,   Scaling.ChakraDefensePerLevel);

    ProgressionComponent->SetBaseStats(BaseStatMap);
    ProgressionComponent->SetLevelScaling(ScalingMap);
    ProgressionComponent->InitializeProgression(1, 1000.0f);

    // Reputation
    ReputationComponent->SetPrimaryFaction(CharacterData->PrimaryVillage);

    // Movement speed from stats
    GetCharacterMovement()->MaxWalkSpeed = Stats.MovementSpeed;
}

void ANarutoCharacterBase::BindComponentEvents()
{
    if (HealthComponent)
    {
        HealthComponent->OnDeath.AddDynamic(this, &ANarutoCharacterBase::HandleDeath);
    }

    if (ChakraComponent)
    {
        ChakraComponent->OnChakraDepleted.AddDynamic(this, &ANarutoCharacterBase::HandleChakraDepleted);
    }
}

// ============================================================
//  Component Event Handlers
// ============================================================

void ANarutoCharacterBase::HandleDeath(const FNarutoDamageEvent& KillingBlow)
{
    OnCharacterDeath.Broadcast(this, KillingBlow);

    // Publish to EventBus
    if (UNarutoGameInstance* GI = UNarutoGameInstance::Get(this))
    {
        if (UNarutoEventBus* Bus = GI->GetEventBus())
        {
            FCharacterDefeatedEvent Evt;
            Evt.DefeatedActor  = this;
            Evt.KillerActor    = KillingBlow.Instigator.Get();
            Evt.KillingBlow    = KillingBlow;
            Evt.DeathLocation  = GetActorLocation();
            Evt.Timestamp      = GetWorld()->GetTimeSeconds();
            Bus->OnCharacterDefeated.Broadcast(Evt);
        }
    }
}

void ANarutoCharacterBase::HandleChakraDepleted()
{
    // Subclasses override to handle chakra depletion (e.g., cancel transformations)
}

// ============================================================
//  ICombatant Implementation
// ============================================================

int32 ANarutoCharacterBase::GetCombatantID_Implementation() const { return CombatantID; }

FText ANarutoCharacterBase::GetCombatantDisplayName_Implementation() const
{
    return CharacterData ? CharacterData->DisplayName : FText::FromString(GetName());
}

bool ANarutoCharacterBase::IsAlive_Implementation() const
{
    return HealthComponent && !HealthComponent->IsDead();
}

bool ANarutoCharacterBase::IsInCombat_Implementation() const
{
    return CombatComponent && CombatComponent->IsInCombat();
}

ECombatFlags ANarutoCharacterBase::GetCombatFlags_Implementation() const
{
    return CombatComponent ? CombatComponent->GetCombatFlags() : ECombatFlags::None;
}

bool ANarutoCharacterBase::HasCombatFlag_Implementation(ECombatFlags Flag) const
{
    return CombatComponent && CombatComponent->HasFlag(Flag);
}

ECombatStance ANarutoCharacterBase::GetCombatStance_Implementation() const
{
    return CombatComponent ? CombatComponent->GetStance() : ECombatStance::Neutral;
}

int32 ANarutoCharacterBase::GetCurrentAttackFrame_Implementation() const
{
    return CombatComponent ? CombatComponent->GetCurrentAttackFrame() : 0;
}

float ANarutoCharacterBase::GetResolvedStat_Implementation(ECharacterStat Stat) const
{
    return ProgressionComponent ? ProgressionComponent->GetResolvedStat(Stat) : 0.0f;
}

float ANarutoCharacterBase::GetHealthPercent_Implementation() const
{
    return HealthComponent ? HealthComponent->GetHealthPercent() : 0.0f;
}

float ANarutoCharacterBase::GetChakraPercent_Implementation() const
{
    return ChakraComponent ? ChakraComponent->GetChakraPercent() : 0.0f;
}

void ANarutoCharacterBase::OnHitLanded_Implementation(ICombatant* Target, const FNarutoDamageEvent& DamageEvent)
{
    // Gain jutsu mastery on hit
    if (DamageEvent.SourceJutsuTag.IsValid() && ProgressionComponent)
    {
        ProgressionComponent->GainJutsuMastery(DamageEvent.SourceJutsuTag, 10.0f);
    }
}

void ANarutoCharacterBase::OnHitReceived_Implementation(ICombatant* Instigator, const FNarutoDamageEvent& DamageEvent)
{
    if (HealthComponent)
    {
        HealthComponent->ApplyDamage(DamageEvent);
    }
}

void ANarutoCharacterBase::OnBlockSuccessful_Implementation(ICombatant* Instigator, const FNarutoDamageEvent& DamageEvent)
{
    if (CombatComponent)
    {
        CombatComponent->DamageGuard(DamageEvent.FinalDamage * 0.1f);
    }
}

void ANarutoCharacterBase::OnParrySuccessful_Implementation(ICombatant* Instigator, const FNarutoDamageEvent& DamageEvent)
{
    // Grant brief invulnerability on successful parry
    if (HealthComponent)
    {
        HealthComponent->GrantInvulnerability(0.5f,
            FGameplayTag::RequestGameplayTag(TEXT("Combat.Parry.Invulnerability")));
    }
}

void ANarutoCharacterBase::OnSubstitutionActivated_Implementation(ICombatant* Instigator)
{
    if (CombatComponent)
    {
        CombatComponent->ConsumeSubstitution();
    }

    // Grant invulnerability during substitution teleport
    if (HealthComponent)
    {
        HealthComponent->GrantInvulnerability(0.3f,
            FGameplayTag::RequestGameplayTag(TEXT("Combat.Substitution.Invulnerability")));
    }
}

void ANarutoCharacterBase::OnCombatEntered_Implementation()
{
    if (CombatComponent) CombatComponent->SetInCombat(true);
}

void ANarutoCharacterBase::OnCombatExited_Implementation()
{
    if (CombatComponent)
    {
        CombatComponent->SetInCombat(false);
        CombatComponent->ResetCombo();
    }
}

void ANarutoCharacterBase::OnDefeated_Implementation(ICombatant* Killer)
{
    // Handled via HealthComponent::OnDeath → HandleDeath
}

void ANarutoCharacterBase::ApplyCombatFlags_Implementation(ECombatFlags Flags)
{
    if (CombatComponent) CombatComponent->AddFlag(Flags);
}

void ANarutoCharacterBase::RemoveCombatFlags_Implementation(ECombatFlags Flags)
{
    if (CombatComponent) CombatComponent->RemoveFlag(Flags);
}

void ANarutoCharacterBase::SetCombatStance_Implementation(ECombatStance NewStance)
{
    if (CombatComponent) CombatComponent->SetStance(NewStance);
}

void ANarutoCharacterBase::ApplyKnockback_Implementation(FVector Direction, float Force)
{
    LaunchCharacter(Direction.GetSafeNormal() * Force, true, true);
}

void ANarutoCharacterBase::ApplyLaunch_Implementation(FVector LaunchVelocity)
{
    LaunchCharacter(LaunchVelocity, true, true);
    if (CombatComponent) CombatComponent->AddFlag(ECombatFlags::IsAirborne);
}

void ANarutoCharacterBase::ApplyHitstun_Implementation(int32 HitstunFrames)
{
    if (CombatComponent) CombatComponent->ApplyHitstun(HitstunFrames);
}

// ============================================================
//  IChakraUser Implementation
// ============================================================

float ANarutoCharacterBase::GetCurrentChakra_Implementation() const
{
    return ChakraComponent ? ChakraComponent->GetCurrentChakra() : 0.0f;
}

float ANarutoCharacterBase::GetMaxChakra_Implementation() const
{
    return ChakraComponent ? ChakraComponent->GetMaxChakra() : 0.0f;
}

float ANarutoCharacterBase::GetChakraRegenRate_Implementation() const
{
    return ChakraComponent ? ChakraComponent->GetRegenRate() : 0.0f;
}

EChakraNature ANarutoCharacterBase::GetNatureAffinities_Implementation() const
{
    return ChakraComponent ? ChakraComponent->GetNatureAffinities() : EChakraNature::None;
}

bool ANarutoCharacterBase::HasNatureAffinity_Implementation(EChakraNature Nature) const
{
    return ChakraComponent && ChakraComponent->HasNatureAffinity(Nature);
}

bool ANarutoCharacterBase::CanAffordChakraCost_Implementation(const FChakraCost& Cost) const
{
    return ChakraComponent && ChakraComponent->CanAffordCost(Cost);
}

float ANarutoCharacterBase::ConsumeChakra_Implementation(float Amount, bool bAllowPartial)
{
    return ChakraComponent ? ChakraComponent->ConsumeChakra(Amount, bAllowPartial) : 0.0f;
}

float ANarutoCharacterBase::RestoreChakra_Implementation(float Amount)
{
    return ChakraComponent ? ChakraComponent->RestoreChakra(Amount) : 0.0f;
}

void ANarutoCharacterBase::SetChakra_Implementation(float NewValue)
{
    if (ChakraComponent) ChakraComponent->SetChakra(NewValue);
}

float ANarutoCharacterBase::TransferChakraTo_Implementation(TScriptInterface<IChakraUser> Recipient, float Amount)
{
    if (!ChakraComponent || !Recipient.GetObject()) return 0.0f;

    ANarutoCharacterBase* RecipientChar = Cast<ANarutoCharacterBase>(Recipient.GetObject());
    if (!RecipientChar || !RecipientChar->ChakraComponent) return 0.0f;

    return ChakraComponent->TransferChakraTo(RecipientChar->ChakraComponent, Amount);
}

bool ANarutoCharacterBase::CanShareChakra_Implementation() const
{
    return ChakraComponent && ChakraComponent->CanShareChakra();
}

void ANarutoCharacterBase::OnChakraDepleted_Implementation() {}
void ANarutoCharacterBase::OnChakraFullyRestored_Implementation() {}
void ANarutoCharacterBase::OnChakraRegenTick_Implementation(float DeltaTime, float RegenAmount) {}

// ============================================================
//  IDamageable Implementation
// ============================================================

float ANarutoCharacterBase::GetCurrentHealth_Implementation() const
{
    return HealthComponent ? HealthComponent->GetCurrentHealth() : 0.0f;
}

float ANarutoCharacterBase::GetMaxHealth_Implementation() const
{
    return HealthComponent ? HealthComponent->GetMaxHealth() : 0.0f;
}

bool ANarutoCharacterBase::IsDead_Implementation() const
{
    return HealthComponent && HealthComponent->IsDead();
}

bool ANarutoCharacterBase::IsInvulnerable_Implementation() const
{
    return HealthComponent && HealthComponent->IsInvulnerable();
}

float ANarutoCharacterBase::ApplyDamage_Implementation(const FNarutoDamageEvent& DamageEvent)
{
    return HealthComponent ? HealthComponent->ApplyDamage(DamageEvent) : 0.0f;
}

float ANarutoCharacterBase::ApplyHealing_Implementation(float Amount, FGameplayTag HealerTag)
{
    return HealthComponent ? HealthComponent->ApplyHealing(Amount, HealerTag) : 0.0f;
}

void ANarutoCharacterBase::ApplyDamageOverTimeTick_Implementation(float DamagePerTick,
    EDamageType DamageType, FGameplayTag SourceTag)
{
    if (HealthComponent)
    {
        HealthComponent->ApplyDamageOverTimeTick(DamagePerTick, DamageType, SourceTag);
    }
}

float ANarutoCharacterBase::GetDamageResistance_Implementation(EDamageType DamageType) const
{
    if (!CharacterData) return 1.0f;

    const float* Resistance = CharacterData->DamageResistances.Find(DamageType);
    return Resistance ? *Resistance : 1.0f;
}

void ANarutoCharacterBase::OnDamageReceived_Implementation(const FNarutoDamageEvent& DamageEvent) {}
void ANarutoCharacterBase::OnHealingReceived_Implementation(float Amount, FGameplayTag HealerTag) {}

void ANarutoCharacterBase::OnDeath_Implementation(const FNarutoDamageEvent& KillingBlow)
{
    // Disable collision and movement on death
    SetActorEnableCollision(false);
    GetCharacterMovement()->DisableMovement();
}

// ============================================================
//  IJutsuCaster Implementation — delegates to JutsuComponent
// ============================================================

TArray<UJutsuData*> ANarutoCharacterBase::GetKnownJutsu_Implementation() const
{
    return JutsuComponent ? JutsuComponent->GetKnownJutsu() : TArray<UJutsuData*>();
}

bool ANarutoCharacterBase::KnowsJutsu_Implementation(FGameplayTag JutsuTag) const
{
    return JutsuComponent && JutsuComponent->KnowsJutsu(JutsuTag);
}

float ANarutoCharacterBase::GetJutsuMastery_Implementation(FGameplayTag JutsuTag) const
{
    return ProgressionComponent ? ProgressionComponent->GetJutsuMastery(JutsuTag) : 0.0f;
}

bool ANarutoCharacterBase::CanCastJutsu_Implementation(FGameplayTag JutsuTag) const
{
    return JutsuComponent && JutsuComponent->CanCast(JutsuTag);
}

float ANarutoCharacterBase::GetJutsuCooldownRemaining_Implementation(FGameplayTag JutsuTag) const
{
    return JutsuComponent ? JutsuComponent->GetCooldownRemaining(JutsuTag) : 0.0f;
}

void ANarutoCharacterBase::OnJutsuCastBegin_Implementation(FGameplayTag JutsuTag, UJutsuData* JutsuData) {}
void ANarutoCharacterBase::OnJutsuActivated_Implementation(FGameplayTag JutsuTag, UJutsuData* JutsuData) {}
void ANarutoCharacterBase::OnJutsuCompleted_Implementation(FGameplayTag JutsuTag, UJutsuData* JutsuData) {}
void ANarutoCharacterBase::OnJutsuInterrupted_Implementation(FGameplayTag JutsuTag, UJutsuData* JutsuData) {}
void ANarutoCharacterBase::OnJutsuCooldownStarted_Implementation(FGameplayTag JutsuTag, float CooldownDuration) {}
void ANarutoCharacterBase::OnJutsuCooldownExpired_Implementation(FGameplayTag JutsuTag) {}

// ============================================================
//  IFactionMember Implementation — delegates to ReputationComponent
// ============================================================

EVillage ANarutoCharacterBase::GetPrimaryFaction_Implementation() const
{
    return ReputationComponent ? ReputationComponent->GetPrimaryFaction() : PrimaryVillage;
}

TArray<EVillage> ANarutoCharacterBase::GetAllFactions_Implementation() const
{
    return ReputationComponent ? ReputationComponent->GetAllFactions() : TArray<EVillage>();
}

EShinobi_Rank ANarutoCharacterBase::GetShinobiRank_Implementation() const
{
    return CharacterData ? CharacterData->ShinobiRank : EShinobi_Rank::Genin;
}

EReputationTier ANarutoCharacterBase::GetReputationWithFaction_Implementation(EVillage Faction) const
{
    return ReputationComponent ? ReputationComponent->GetReputationTier(Faction) : EReputationTier::Neutral;
}

bool ANarutoCharacterBase::IsHostileToFaction_Implementation(EVillage Faction) const
{
    return ReputationComponent && ReputationComponent->IsHostileTo(Faction);
}

bool ANarutoCharacterBase::IsAlliedWithFaction_Implementation(EVillage Faction) const
{
    return ReputationComponent && ReputationComponent->IsAlliedWith(Faction);
}

void ANarutoCharacterBase::OnReputationChanged_Implementation(EVillage Faction, float OldValue,
    float NewValue, EReputationTier NewTier) {}

void ANarutoCharacterBase::OnFactionChanged_Implementation(EVillage OldFaction, EVillage NewFaction)
{
    PrimaryVillage = NewFaction;
}
