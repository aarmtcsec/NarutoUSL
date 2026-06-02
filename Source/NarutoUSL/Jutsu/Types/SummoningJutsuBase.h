// Copyright (c) 2025 Maze Runner Studios. All Rights Reserved.
// Naruto: Ultimate Shinobi Legacy
// SummoningJutsuBase — Base executor for all Summoning Jutsu.

#pragma once

#include "CoreMinimal.h"
#include "Jutsu/Core/JutsuExecutor.h"
#include "SummoningJutsuBase.generated.h"

class ANarutoCharacterBase;

UENUM(BlueprintType)
enum class ESummonBehavior : uint8
{
    Companion   UMETA(DisplayName = "Companion (follows caster)"),
    Guardian    UMETA(DisplayName = "Guardian (protects caster)"),
    Attacker    UMETA(DisplayName = "Attacker (targets enemies)"),
    Transport   UMETA(DisplayName = "Transport (caster rides)"),
    OneShot     UMETA(DisplayName = "One-Shot Attack"),
};

/**
 * USummoningJutsuBase
 *
 * Base class for all Summoning Jutsu executors. Handles:
 *   - Summoning contract validation (character must have signed the contract)
 *   - Blood cost (small health cost for summoning)
 *   - Summon actor spawning and AI initialization
 *   - Summon lifetime management
 *   - Multiple summon types (Gamabunta, Manda, Katsuyu, etc.)
 *
 * Summons are full ANarutoCharacterBase actors with their own AI.
 */
UCLASS(Abstract, BlueprintType, Blueprintable)
class NARUTOUSL_API USummoningJutsuBase : public UJutsuExecutorBase
{
    GENERATED_BODY()

public:

    virtual FJutsuExecutionResult Execute_Implementation(
        const FJutsuExecutionContext& Context) override;

    virtual void OnCompleted(const FJutsuExecutionContext& Context) override;

protected:

    virtual ANarutoCharacterBase* SpawnSummon(const FJutsuExecutionContext& Context);
    virtual void InitializeSummonAI(ANarutoCharacterBase* Summon,
                                     const FJutsuExecutionContext& Context);
    virtual void DismissSummon(ANarutoCharacterBase* Summon);

    bool ValidateSummoningContract(const FJutsuExecutionContext& Context) const;

    /** The character class to spawn as the summon. */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Summoning")
    TSoftClassPtr<ANarutoCharacterBase> SummonClass;

    /** How the summon behaves after being summoned. */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Summoning")
    ESummonBehavior SummonBehavior = ESummonBehavior::Companion;

    /** Lifetime of the summon in seconds. 0 = permanent until dismissed. */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Summoning", meta = (ClampMin = "0"))
    float SummonLifetime = 60.0f;

    /** Blood cost as fraction of max health. */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Summoning",
        meta = (ClampMin = "0", ClampMax = "0.1"))
    float BloodCostFraction = 0.01f;

    /** Tag required on the caster to use this summon (contract tag). */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Summoning")
    FGameplayTag RequiredContractTag;

    /** Currently active summon (weak ref). */
    TWeakObjectPtr<ANarutoCharacterBase> ActiveSummon;
};
