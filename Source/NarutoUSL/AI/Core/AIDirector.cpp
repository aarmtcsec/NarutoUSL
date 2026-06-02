// Copyright (c) 2025 Maze Runner Studios. All Rights Reserved.

#include "AI/Core/AIDirector.h"
#include "Character/Enemy/NarutoEnemyBase.h"
#include "Core/Events/NarutoEventBus.h"
#include "NarutoUSL.h"

// ============================================================
//  FEnemyRecord helpers
// ============================================================

AActor* FEnemyRecord::GetHighestThreatTarget() const
{
    AActor* Best = nullptr;
    float   BestThreat = 0.0f;
    for (const FThreatEntry& Entry : ThreatList)
    {
        if (Entry.IsValid() && Entry.ThreatValue > BestThreat)
        {
            BestThreat = Entry.ThreatValue;
            Best       = Entry.ThreatSource.Get();
        }
    }
    return Best;
}

float FEnemyRecord::GetTotalThreat() const
{
    float Total = 0.0f;
    for (const FThreatEntry& E : ThreatList) Total += E.ThreatValue;
    return Total;
}

void FEnemyRecord::AddThreat(AActor* Source, float Amount)
{
    for (FThreatEntry& E : ThreatList)
    {
        if (E.ThreatSource.Get() == Source)
        {
            E.ThreatValue += Amount;
            E.LastUpdateTime = FPlatformTime::Seconds();
            return;
        }
    }
    FThreatEntry New;
    New.ThreatSource    = Source;
    New.ThreatValue     = Amount;
    New.LastUpdateTime  = FPlatformTime::Seconds();
    ThreatList.Add(New);
}

void FEnemyRecord::DecayThreat(float DecayRate, float DeltaTime)
{
    for (int32 i = ThreatList.Num() - 1; i >= 0; --i)
    {
        ThreatList[i].ThreatValue -= DecayRate * DeltaTime;
        if (ThreatList[i].ThreatValue <= 0.0f)
        {
            ThreatList.RemoveAt(i);
        }
    }
}

// ============================================================
//  UAIDirector
// ============================================================

UAIDirector::UAIDirector()
{
    SubsystemName = TEXT("AIDirector");
    bTickEnabled  = true;
    TickPriority  = ESubsystemTickPriority::PostPhysics;
}

void UAIDirector::Initialize(UNarutoEventBus* InEventBus)
{
    EventBus     = InEventBus;
    bInitialized = true;
    UE_LOG(LogNarutoAI, Log, TEXT("[AIDirector] Initialized."));
}

void UAIDirector::Shutdown()
{
    EnemyRecords.Empty();
    ActiveEnemyJutsu.Empty();
}

void UAIDirector::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    TickThreatDecay(DeltaTime);
    TickEncounterBudget(DeltaTime);
    TickAdaptiveDifficulty(DeltaTime);
}

// ============================================================
//  Registration
// ============================================================

void UAIDirector::RegisterEnemy(ANarutoEnemyBase* Enemy)
{
    if (!Enemy || FindRecord(Enemy)) return;
    FEnemyRecord Record;
    Record.Enemy = Enemy;
    EnemyRecords.Add(Record);
}

void UAIDirector::UnregisterEnemy(ANarutoEnemyBase* Enemy)
{
    EnemyRecords.RemoveAll([Enemy](const FEnemyRecord& R)
    {
        return R.Enemy.Get() == Enemy;
    });
    ActiveEnemyJutsu.Remove(Enemy);
    ReleaseAttackSlot(Enemy);
    ReleaseFlankSlot(Enemy);
}

// ============================================================
//  Threat
// ============================================================

void UAIDirector::AddThreat(ANarutoEnemyBase* Enemy, AActor* ThreatSource, float Amount)
{
    FEnemyRecord* Record = FindRecord(Enemy);
    if (Record) Record->AddThreat(ThreatSource, Amount);
}

void UAIDirector::ClearThreat(ANarutoEnemyBase* Enemy, AActor* ThreatSource)
{
    FEnemyRecord* Record = FindRecord(Enemy);
    if (!Record) return;
    Record->ThreatList.RemoveAll([ThreatSource](const FThreatEntry& E)
    {
        return E.ThreatSource.Get() == ThreatSource;
    });
}

AActor* UAIDirector::GetHighestThreatTarget(ANarutoEnemyBase* Enemy) const
{
    const FEnemyRecord* Record = EnemyRecords.FindByPredicate(
        [Enemy](const FEnemyRecord& R){ return R.Enemy.Get() == Enemy; });
    return Record ? Record->GetHighestThreatTarget() : nullptr;
}

float UAIDirector::GetThreatValue(ANarutoEnemyBase* Enemy, AActor* ThreatSource) const
{
    const FEnemyRecord* Record = EnemyRecords.FindByPredicate(
        [Enemy](const FEnemyRecord& R){ return R.Enemy.Get() == Enemy; });
    if (!Record) return 0.0f;
    for (const FThreatEntry& E : Record->ThreatList)
    {
        if (E.ThreatSource.Get() == ThreatSource) return E.ThreatValue;
    }
    return 0.0f;
}

// ============================================================
//  Encounter Budget
// ============================================================

bool UAIDirector::RequestAttackSlot(ANarutoEnemyBase* Enemy)
{
    if (!EncounterBudget.CanAddAttacker()) return false;
    ++EncounterBudget.CurrentAttackers;
    return true;
}

void UAIDirector::ReleaseAttackSlot(ANarutoEnemyBase* Enemy)
{
    EncounterBudget.CurrentAttackers =
        FMath::Max(0, EncounterBudget.CurrentAttackers - 1);
}

bool UAIDirector::RequestFlankSlot(ANarutoEnemyBase* Enemy)
{
    if (!EncounterBudget.CanAddFlanker()) return false;
    ++EncounterBudget.CurrentFlankers;
    return true;
}

void UAIDirector::ReleaseFlankSlot(ANarutoEnemyBase* Enemy)
{
    EncounterBudget.CurrentFlankers =
        FMath::Max(0, EncounterBudget.CurrentFlankers - 1);
}

// ============================================================
//  Coordination
// ============================================================

void UAIDirector::SignalFlank(FVector PlayerLocation, float Radius)
{
    for (FEnemyRecord& Record : EnemyRecords)
    {
        ANarutoEnemyBase* Enemy = Record.Enemy.Get();
        if (!Enemy) continue;
        const float Dist = FVector::Dist(Enemy->GetActorLocation(), PlayerLocation);
        if (Dist <= Radius && RequestFlankSlot(Enemy))
        {
            Record.bIsCoordinating = true;
            // Signal the enemy's AI controller to begin flanking
            // Full implementation calls the enemy's BehaviorTree blackboard
        }
    }
}

void UAIDirector::SignalRetreat(ANarutoEnemyBase* Initiator, float Radius)
{
    if (!Initiator) return;
    const FVector Origin = Initiator->GetActorLocation();
    for (FEnemyRecord& Record : EnemyRecords)
    {
        ANarutoEnemyBase* Enemy = Record.Enemy.Get();
        if (!Enemy || Enemy == Initiator) continue;
        if (FVector::Dist(Enemy->GetActorLocation(), Origin) <= Radius)
        {
            // Signal retreat via blackboard
            UE_LOG(LogNarutoAI, Verbose,
                TEXT("[AIDirector] Signaling retreat to %s"), *Enemy->GetName());
        }
    }
}

bool UAIDirector::IsJutsuInUse(FGameplayTag JutsuTag) const
{
    for (const auto& Pair : ActiveEnemyJutsu)
    {
        if (Pair.Value == JutsuTag) return true;
    }
    return false;
}

void UAIDirector::RegisterJutsuInUse(ANarutoEnemyBase* Enemy, FGameplayTag JutsuTag)
{
    if (Enemy) ActiveEnemyJutsu.Add(Enemy, JutsuTag);
}

void UAIDirector::UnregisterJutsuInUse(ANarutoEnemyBase* Enemy)
{
    if (Enemy) ActiveEnemyJutsu.Remove(Enemy);
}

// ============================================================
//  Adaptive Difficulty
// ============================================================

void UAIDirector::RecordPlayerDamageTaken(float Amount)
{
    PlayerDamageTakenRecent += Amount;
}

void UAIDirector::RecordPlayerDeath()
{
    ++PlayerDeathCount;
    // Immediately soften difficulty on death
    AdaptiveDifficultyScale = FMath::Max(MinDifficultyScale,
        AdaptiveDifficultyScale - 0.1f);
}

void UAIDirector::RecordPlayerComboLength(int32 ComboLength)
{
    PlayerAvgComboLength = FMath::Lerp(PlayerAvgComboLength,
        static_cast<float>(ComboLength), 0.1f);
}

// ============================================================
//  Tick Stages
// ============================================================

void UAIDirector::TickThreatDecay(float DeltaTime)
{
    for (FEnemyRecord& Record : EnemyRecords)
    {
        Record.DecayThreat(ThreatDecayRate, DeltaTime);
    }
}

void UAIDirector::TickEncounterBudget(float DeltaTime)
{
    // Validate that counted slots still have valid enemies
    int32 ValidAttackers = 0;
    for (const FEnemyRecord& Record : EnemyRecords)
    {
        if (Record.bIsEngaged && Record.Enemy.IsValid()) ++ValidAttackers;
    }
    EncounterBudget.CurrentAttackers = ValidAttackers;
}

void UAIDirector::TickAdaptiveDifficulty(float DeltaTime)
{
    DifficultyAdjustTimer += DeltaTime;
    if (DifficultyAdjustTimer < DifficultyAdjustInterval) return;
    DifficultyAdjustTimer = 0.0f;

    // Player taking lots of damage → ease off
    if (PlayerDamageTakenRecent > 500.0f)
    {
        AdaptiveDifficultyScale = FMath::Max(MinDifficultyScale,
            AdaptiveDifficultyScale - 0.05f);
    }
    // Player dominating (high combo, low damage taken) → increase challenge
    else if (PlayerDamageTakenRecent < 100.0f && PlayerAvgComboLength > 10.0f)
    {
        AdaptiveDifficultyScale = FMath::Min(MaxDifficultyScale,
            AdaptiveDifficultyScale + 0.05f);
    }

    PlayerDamageTakenRecent = 0.0f;

    UE_LOG(LogNarutoAI, Verbose,
        TEXT("[AIDirector] Adaptive difficulty: %.2f"), AdaptiveDifficultyScale);
}

void UAIDirector::UpdateEncounterBudgetScaling()
{
    // Scale budget with adaptive difficulty
    EncounterBudget.MaxSimultaneousAttackers =
        FMath::RoundToInt(3.0f * AdaptiveDifficultyScale);
    EncounterBudget.MaxSimultaneousFlank =
        FMath::RoundToInt(2.0f * AdaptiveDifficultyScale);
}

FEnemyRecord* UAIDirector::FindRecord(ANarutoEnemyBase* Enemy)
{
    return EnemyRecords.FindByPredicate([Enemy](const FEnemyRecord& R)
    {
        return R.Enemy.Get() == Enemy;
    });
}

TMap<FString, FString> UAIDirector::GetDebugInfo() const
{
    TMap<FString, FString> Info = Super::GetDebugInfo();
    Info.Add(TEXT("RegisteredEnemies"),   FString::FromInt(EnemyRecords.Num()));
    Info.Add(TEXT("ActiveAttackers"),     FString::FromInt(EncounterBudget.CurrentAttackers));
    Info.Add(TEXT("DifficultyScale"),     FString::SanitizeFloat(AdaptiveDifficultyScale));
    Info.Add(TEXT("PlayerDeaths"),        FString::FromInt(PlayerDeathCount));
    return Info;
}
