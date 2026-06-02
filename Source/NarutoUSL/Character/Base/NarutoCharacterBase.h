// Copyright (c) 2025 Maze Runner Studios. All Rights Reserved.
// Naruto: Ultimate Shinobi Legacy
// NarutoCharacterBase — Root character class for all playable and AI characters.
// Every character in the game (player, NPC, enemy, boss) derives from this.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "GameplayTagContainer.h"
#include "Core/Types/NarutoTypes.h"
#include "Core/Interfaces/ICombatant.h"
#include "Core/Interfaces/IChakraUser.h"
#include "Core/Interfaces/IDamageable.h"
#include "Core/Interfaces/IJutsuCaster.h"
#include "Core/Interfaces/IFactionMember.h"
#include "NarutoCharacterBase.generated.h"

// Forward declarations
class UHealthComponent;
class UChakraComponent;
class UCombatComponent;
class UProgressionComponent;
class UInventoryComponent;
class UEquipmentComponent;
class UJutsuComponent;
class UTransformationComponent;
class UReputationComponent;
class UNarutoCharacterData;
class USpringArmComponent;
class UCameraComponent;
class UMotionWarpingComponent;

// ============================================================
//  Delegates
// ============================================================

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCharacterInitialized, ANarutoCharacterBase*, Character);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnCharacterDeath,
    ANarutoCharacterBase*, DeadCharacter, const FNarutoDamageEvent&, KillingBlow);

// ============================================================
//  NarutoCharacterBase
// ============================================================

/**
 * ANarutoCharacterBase
 *
 * Root class for every character in Naruto: Ultimate Shinobi Legacy.
 * Owns all character components and implements all core interfaces.
 * Subclasses specialize behavior:
 *   - ANarutoPlayerCharacter  (player input, camera)
 *   - ANarutoNPCBase          (schedule, dialogue)
 *   - ANarutoEnemyBase        (AI combat)
 *   - ANarutoBossBase         (phases, mechanics)
 *
 * Component ownership:
 *   All components are created in the constructor and initialized
 *   in InitializeFromData() after the CharacterData asset is set.
 *   No component should be null after BeginPlay().
 *
 * Interface implementation:
 *   All interface methods delegate to the appropriate component.
 *   The character class itself contains no stat or resource logic.
 */
UCLASS(Abstract)
class NARUTOUSL_API ANarutoCharacterBase : public ACharacter,
    public ICombatant,
    public IChakraUser,
    public IDamageable,
    public IJutsuCaster,
    public IFactionMember
{
    GENERATED_BODY()

public:

    ANarutoCharacterBase(const FObjectInitializer& ObjectInitializer);

    // ------------------------------------------------------------------
    //  ACharacter Interface
    // ------------------------------------------------------------------

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

    // ------------------------------------------------------------------
    //  Initialization
    // ------------------------------------------------------------------

    /**
     * Initializes all components from the provided CharacterData asset.
     * Must be called after the actor is spawned and before combat begins.
     * Automatically called in BeginPlay if CharacterData is set.
     */
    UFUNCTION(BlueprintCallable, Category = "Character")
    void InitializeFromData(UNarutoCharacterData* InCharacterData);

    UFUNCTION(BlueprintPure, Category = "Character")
    bool IsInitialized() const { return bIsInitialized; }

    UFUNCTION(BlueprintPure, Category = "Character")
    UNarutoCharacterData* GetCharacterData() const { return CharacterData; }

    // ------------------------------------------------------------------
    //  Component Accessors
    // ------------------------------------------------------------------

    UFUNCTION(BlueprintPure, Category = "Character|Components")
    UHealthComponent* GetHealthComponent() const { return HealthComponent; }

    UFUNCTION(BlueprintPure, Category = "Character|Components")
    UChakraComponent* GetChakraComponent() const { return ChakraComponent; }

    UFUNCTION(BlueprintPure, Category = "Character|Components")
    UCombatComponent* GetCombatComponent() const { return CombatComponent; }

    UFUNCTION(BlueprintPure, Category = "Character|Components")
    UProgressionComponent* GetProgressionComponent() const { return ProgressionComponent; }

    UFUNCTION(BlueprintPure, Category = "Character|Components")
    UJutsuComponent* GetJutsuComponent() const { return JutsuComponent; }

    UFUNCTION(BlueprintPure, Category = "Character|Components")
    UTransformationComponent* GetTransformationComponent() const { return TransformationComponent; }

    UFUNCTION(BlueprintPure, Category = "Character|Components")
    UInventoryComponent* GetInventoryComponent() const { return InventoryComponent; }

    UFUNCTION(BlueprintPure, Category = "Character|Components")
    UEquipmentComponent* GetEquipmentComponent() const { return EquipmentComponent; }

    UFUNCTION(BlueprintPure, Category = "Character|Components")
    UReputationComponent* GetReputationComponent() const { return ReputationComponent; }

    // ------------------------------------------------------------------
    //  ICombatant Interface
    // ------------------------------------------------------------------

    virtual int32 GetCombatantID_Implementation() const override;
    virtual FText GetCombatantDisplayName_Implementation() const override;
    virtual bool IsAlive_Implementation() const override;
    virtual bool IsInCombat_Implementation() const override;
    virtual ECombatFlags GetCombatFlags_Implementation() const override;
    virtual bool HasCombatFlag_Implementation(ECombatFlags Flag) const override;
    virtual ECombatStance GetCombatStance_Implementation() const override;
    virtual int32 GetCurrentAttackFrame_Implementation() const override;
    virtual float GetResolvedStat_Implementation(ECharacterStat Stat) const override;
    virtual float GetHealthPercent_Implementation() const override;
    virtual float GetChakraPercent_Implementation() const override;
    virtual void OnHitLanded_Implementation(ICombatant* Target, const FNarutoDamageEvent& DamageEvent) override;
    virtual void OnHitReceived_Implementation(ICombatant* Instigator, const FNarutoDamageEvent& DamageEvent) override;
    virtual void OnBlockSuccessful_Implementation(ICombatant* Instigator, const FNarutoDamageEvent& DamageEvent) override;
    virtual void OnParrySuccessful_Implementation(ICombatant* Instigator, const FNarutoDamageEvent& DamageEvent) override;
    virtual void OnSubstitutionActivated_Implementation(ICombatant* Instigator) override;
    virtual void OnCombatEntered_Implementation() override;
    virtual void OnCombatExited_Implementation() override;
    virtual void OnDefeated_Implementation(ICombatant* Killer) override;
    virtual void ApplyCombatFlags_Implementation(ECombatFlags Flags) override;
    virtual void RemoveCombatFlags_Implementation(ECombatFlags Flags) override;
    virtual void SetCombatStance_Implementation(ECombatStance NewStance) override;
    virtual void ApplyKnockback_Implementation(FVector Direction, float Force) override;
    virtual void ApplyLaunch_Implementation(FVector LaunchVelocity) override;
    virtual void ApplyHitstun_Implementation(int32 HitstunFrames) override;

    // ------------------------------------------------------------------
    //  IChakraUser Interface
    // ------------------------------------------------------------------

    virtual float GetCurrentChakra_Implementation() const override;
    virtual float GetMaxChakra_Implementation() const override;
    virtual float GetChakraRegenRate_Implementation() const override;
    virtual EChakraNature GetNatureAffinities_Implementation() const override;
    virtual bool HasNatureAffinity_Implementation(EChakraNature Nature) const override;
    virtual bool CanAffordChakraCost_Implementation(const FChakraCost& Cost) const override;
    virtual float ConsumeChakra_Implementation(float Amount, bool bAllowPartial) override;
    virtual float RestoreChakra_Implementation(float Amount) override;
    virtual void SetChakra_Implementation(float NewValue) override;
    virtual float TransferChakraTo_Implementation(TScriptInterface<IChakraUser> Recipient, float Amount) override;
    virtual bool CanShareChakra_Implementation() const override;
    virtual void OnChakraDepleted_Implementation() override;
    virtual void OnChakraFullyRestored_Implementation() override;
    virtual void OnChakraRegenTick_Implementation(float DeltaTime, float RegenAmount) override;

    // ------------------------------------------------------------------
    //  IDamageable Interface
    // ------------------------------------------------------------------

    virtual float GetCurrentHealth_Implementation() const override;
    virtual float GetMaxHealth_Implementation() const override;
    virtual float GetHealthPercent_IDamageable_Implementation() const;
    virtual bool IsDead_Implementation() const override;
    virtual bool IsInvulnerable_Implementation() const override;
    virtual float ApplyDamage_Implementation(const FNarutoDamageEvent& DamageEvent) override;
    virtual float ApplyHealing_Implementation(float Amount, FGameplayTag HealerTag) override;
    virtual void ApplyDamageOverTimeTick_Implementation(float DamagePerTick, EDamageType DamageType, FGameplayTag SourceTag) override;
    virtual float GetDamageResistance_Implementation(EDamageType DamageType) const override;
    virtual void OnDamageReceived_Implementation(const FNarutoDamageEvent& DamageEvent) override;
    virtual void OnHealingReceived_Implementation(float Amount, FGameplayTag HealerTag) override;
    virtual void OnDeath_Implementation(const FNarutoDamageEvent& KillingBlow) override;

    // ------------------------------------------------------------------
    //  IJutsuCaster Interface
    // ------------------------------------------------------------------

    virtual TArray<UJutsuData*> GetKnownJutsu_Implementation() const override;
    virtual bool KnowsJutsu_Implementation(FGameplayTag JutsuTag) const override;
    virtual float GetJutsuMastery_Implementation(FGameplayTag JutsuTag) const override;
    virtual bool CanCastJutsu_Implementation(FGameplayTag JutsuTag) const override;
    virtual float GetJutsuCooldownRemaining_Implementation(FGameplayTag JutsuTag) const override;
    virtual void OnJutsuCastBegin_Implementation(FGameplayTag JutsuTag, UJutsuData* JutsuData) override;
    virtual void OnJutsuActivated_Implementation(FGameplayTag JutsuTag, UJutsuData* JutsuData) override;
    virtual void OnJutsuCompleted_Implementation(FGameplayTag JutsuTag, UJutsuData* JutsuData) override;
    virtual void OnJutsuInterrupted_Implementation(FGameplayTag JutsuTag, UJutsuData* JutsuData) override;
    virtual void OnJutsuCooldownStarted_Implementation(FGameplayTag JutsuTag, float CooldownDuration) override;
    virtual void OnJutsuCooldownExpired_Implementation(FGameplayTag JutsuTag) override;

    // ------------------------------------------------------------------
    //  IFactionMember Interface
    // ------------------------------------------------------------------

    virtual EVillage GetPrimaryFaction_Implementation() const override;
    virtual TArray<EVillage> GetAllFactions_Implementation() const override;
    virtual EShinobi_Rank GetShinobiRank_Implementation() const override;
    virtual EReputationTier GetReputationWithFaction_Implementation(EVillage Faction) const override;
    virtual bool IsHostileToFaction_Implementation(EVillage Faction) const override;
    virtual bool IsAlliedWithFaction_Implementation(EVillage Faction) const override;
    virtual void OnReputationChanged_Implementation(EVillage Faction, float OldValue, float NewValue, EReputationTier NewTier) override;
    virtual void OnFactionChanged_Implementation(EVillage OldFaction, EVillage NewFaction) override;

    // ------------------------------------------------------------------
    //  Events
    // ------------------------------------------------------------------

    UPROPERTY(BlueprintAssignable, Category = "Character|Events")
    FOnCharacterInitialized OnCharacterInitialized;

    UPROPERTY(BlueprintAssignable, Category = "Character|Events")
    FOnCharacterDeath OnCharacterDeath;

protected:

    // ------------------------------------------------------------------
    //  Initialization Helpers
    // ------------------------------------------------------------------

    virtual void InitializeComponents();
    virtual void ApplyCharacterDataToComponents();
    virtual void BindComponentEvents();

    // ------------------------------------------------------------------
    //  Component Event Handlers
    // ------------------------------------------------------------------

    UFUNCTION()
    void HandleDeath(const FNarutoDamageEvent& KillingBlow);

    UFUNCTION()
    void HandleChakraDepleted();

    // ------------------------------------------------------------------
    //  Configuration
    // ------------------------------------------------------------------

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character")
    TObjectPtr<UNarutoCharacterData> CharacterData;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character")
    EVillage PrimaryVillage = EVillage::None;

    // ------------------------------------------------------------------
    //  Components
    // ------------------------------------------------------------------

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components",
        meta = (AllowPrivateAccess = "true"))
    TObjectPtr<UHealthComponent> HealthComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components",
        meta = (AllowPrivateAccess = "true"))
    TObjectPtr<UChakraComponent> ChakraComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components",
        meta = (AllowPrivateAccess = "true"))
    TObjectPtr<UCombatComponent> CombatComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components",
        meta = (AllowPrivateAccess = "true"))
    TObjectPtr<UProgressionComponent> ProgressionComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components",
        meta = (AllowPrivateAccess = "true"))
    TObjectPtr<UJutsuComponent> JutsuComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components",
        meta = (AllowPrivateAccess = "true"))
    TObjectPtr<UTransformationComponent> TransformationComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components",
        meta = (AllowPrivateAccess = "true"))
    TObjectPtr<UInventoryComponent> InventoryComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components",
        meta = (AllowPrivateAccess = "true"))
    TObjectPtr<UEquipmentComponent> EquipmentComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components",
        meta = (AllowPrivateAccess = "true"))
    TObjectPtr<UReputationComponent> ReputationComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components",
        meta = (AllowPrivateAccess = "true"))
    TObjectPtr<UMotionWarpingComponent> MotionWarpingComponent;

    // ------------------------------------------------------------------
    //  State
    // ------------------------------------------------------------------

    bool bIsInitialized = false;
    int32 CombatantID   = -1;

    static int32 NextCombatantID;
};
