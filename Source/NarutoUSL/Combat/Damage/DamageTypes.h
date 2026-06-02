// Copyright (c) 2025 Maze Runner Studios. All Rights Reserved.
// Naruto: Ultimate Shinobi Legacy
// DamageTypes — UE5 damage type classes for the damage pipeline.
// One class per damage category. Used by UE's damage system and
// our custom DamageCalculator for routing and resistance lookups.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/DamageType.h"
#include "Core/Types/NarutoTypes.h"
#include "DamageTypes.generated.h"

// ============================================================
//  Base Naruto Damage Type
// ============================================================

/**
 * UNarutoDamageTypeBase
 * All Naruto damage types derive from this. Carries the EDamageType
 * enum value so the DamageCalculator can look up resistances without
 * a chain of casts.
 */
UCLASS(Abstract)
class NARUTOUSL_API UNarutoDamageTypeBase : public UDamageType
{
    GENERATED_BODY()

public:

    UNarutoDamageTypeBase()
    {
        bCausedByWorld          = false;
        bScaleMomentumByMass    = false;
        DamageImpulse           = 0.0f;
    }

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Damage")
    EDamageType NarutoDamageType = EDamageType::Physical;

    /** Whether this damage type can be blocked. */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Damage")
    bool bBlockable = true;

    /** Whether this damage type can be parried. */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Damage")
    bool bParriable = true;

    /** Whether this damage type triggers hitstun. */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Damage")
    bool bCausesHitstun = true;

    /** Whether this damage type can be substituted against. */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Damage")
    bool bSubstitutable = true;
};

// ============================================================
//  Concrete Damage Types
// ============================================================

UCLASS()
class NARUTOUSL_API UDT_Physical : public UNarutoDamageTypeBase
{
    GENERATED_BODY()
public:
    UDT_Physical() { NarutoDamageType = EDamageType::Physical; }
};

UCLASS()
class NARUTOUSL_API UDT_Chakra : public UNarutoDamageTypeBase
{
    GENERATED_BODY()
public:
    UDT_Chakra() { NarutoDamageType = EDamageType::Chakra; bParriable = false; }
};

UCLASS()
class NARUTOUSL_API UDT_Fire : public UNarutoDamageTypeBase
{
    GENERATED_BODY()
public:
    UDT_Fire() { NarutoDamageType = EDamageType::Fire; bParriable = false; }
};

UCLASS()
class NARUTOUSL_API UDT_Water : public UNarutoDamageTypeBase
{
    GENERATED_BODY()
public:
    UDT_Water() { NarutoDamageType = EDamageType::Water; bParriable = false; }
};

UCLASS()
class NARUTOUSL_API UDT_Earth : public UNarutoDamageTypeBase
{
    GENERATED_BODY()
public:
    UDT_Earth() { NarutoDamageType = EDamageType::Earth; }
};

UCLASS()
class NARUTOUSL_API UDT_Wind : public UNarutoDamageTypeBase
{
    GENERATED_BODY()
public:
    UDT_Wind() { NarutoDamageType = EDamageType::Wind; bParriable = false; }
};

UCLASS()
class NARUTOUSL_API UDT_Lightning : public UNarutoDamageTypeBase
{
    GENERATED_BODY()
public:
    UDT_Lightning() { NarutoDamageType = EDamageType::Lightning; }
};

UCLASS()
class NARUTOUSL_API UDT_Genjutsu : public UNarutoDamageTypeBase
{
    GENERATED_BODY()
public:
    // Genjutsu bypasses physical block and parry
    UDT_Genjutsu()
    {
        NarutoDamageType = EDamageType::Genjutsu;
        bBlockable       = false;
        bParriable       = false;
        bSubstitutable   = false;
    }
};

UCLASS()
class NARUTOUSL_API UDT_True : public UNarutoDamageTypeBase
{
    GENERATED_BODY()
public:
    // True damage bypasses all mitigation
    UDT_True()
    {
        NarutoDamageType = EDamageType::True;
        bBlockable       = false;
        bParriable       = false;
        bSubstitutable   = false;
        bCausesHitstun   = false;
    }
};
