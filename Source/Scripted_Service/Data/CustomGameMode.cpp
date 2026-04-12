// Fill out your copyright notice in the Description page of Project Settings.


#include "CustomGameMode.h"
#include "EngineUtils.h"
#include "TableActor.h"

ACustomGameMode::ACustomGameMode()
{
    // No Tick needed — all stat changes are event-driven.
    PrimaryActorTick.bCanEverTick = false;
}

// ─────────────────────────────────────────────────────────────────────────────
//  BeginPlay
// ─────────────────────────────────────────────────────────────────────────────

void ACustomGameMode::BeginPlay()
{
    Super::BeginPlay();

    // Subscribe to every table so we catch all delivery and timeout events.
    SubscribeToAllTables();

    // Push an initial broadcast so the HUD can display "0 / $0.00" straight away.
    BroadcastStats();
}

void ACustomGameMode::SubscribeToAllTables()
{
    int32 TableCount = 0;

    for (TActorIterator<ATableActor> It(GetWorld()); It; ++It)
    {
        ATableActor* Table = *It;

        Table->OnOrderDelivered.AddDynamic(
            this, &ACustomGameMode::HandleOrderDelivered);

        Table->OnOrderExpired.AddDynamic(
            this, &ACustomGameMode::HandleOrderExpired);

        ++TableCount;
    }

    UE_LOG(LogTemp, Log,
        TEXT("ScriptedServiceGameMode: Subscribed to %d table(s)."), TableCount);

    if (TableCount == 0)
    {
        UE_LOG(LogTemp, Warning,
            TEXT("ScriptedServiceGameMode: No ATableActor found in the level. "
                 "Place at least one table and make sure it derives from ATableActor."));
    }
}

void ACustomGameMode::HandleOrderDelivered(int32 TableNumber, bool bCorrect)
{
    if (bCorrect)
    {
        Score       += PointsPerCorrectDelivery;
        TotalTips   += TipPerCorrectDelivery;
        OrdersCorrect++;

        UE_LOG(LogTemp, Log,
            TEXT("ScriptedServiceGameMode: Table %d — Correct delivery! "
                 "+%d pts  +$%.2f tip  (Score=%d, Tips=$%.2f)"),
            TableNumber, PointsPerCorrectDelivery, TipPerCorrectDelivery,
            Score, TotalTips);
    }
    else
    {
        Score = FMath::Max(0, Score - PenaltyPerWrongDelivery);
        OrdersWrong++;

        UE_LOG(LogTemp, Warning,
            TEXT("ScriptedServiceGameMode: Table %d — Wrong dish! "
                 "-%d pts  (Score=%d)"),
            TableNumber, PenaltyPerWrongDelivery, Score);
    }

    BroadcastStats();
}

void ACustomGameMode::HandleOrderExpired(int32 TableNumber)
{
    Score = FMath::Max(0, Score - PenaltyPerExpiredOrder);
    OrdersExpired++;

    UE_LOG(LogTemp, Warning,
        TEXT("ScriptedServiceGameMode: Table %d — Order expired! "
             "-%d pts  (Score=%d)"),
        TableNumber, PenaltyPerExpiredOrder, Score);

    BroadcastStats();
}


void ACustomGameMode::ResetStats()
{
    Score         = 0;
    TotalTips     = 0.0f;
    OrdersCorrect = 0;
    OrdersWrong   = 0;
    OrdersExpired = 0;

    UE_LOG(LogTemp, Log, TEXT("ScriptedServiceGameMode: Stats reset."));

    BroadcastStats();
}


FString ACustomGameMode::GetStatsDebugString() const
{
    return FString::Printf(
        TEXT("Score: %d\nTips: $%.2f\nCorrect: %d  Wrong: %d  Expired: %d"),
        Score, TotalTips, OrdersCorrect, OrdersWrong, OrdersExpired);
}


void ACustomGameMode::BroadcastStats()
{
    Score = FMath::Max(0, Score);

    OnStatsUpdated.Broadcast(Score, TotalTips, OrdersCorrect, OrdersWrong, OrdersExpired);
}