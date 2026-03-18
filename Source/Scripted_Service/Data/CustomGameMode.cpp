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

// ─────────────────────────────────────────────────────────────────────────────
//  SubscribeToAllTables
// ─────────────────────────────────────────────────────────────────────────────

void ACustomGameMode::SubscribeToAllTables()
{
    int32 TableCount = 0;

    // TActorIterator<ATableActor> walks every ATableActor that is currently
    // placed (or spawned) in the persistent level.
    // Docs: https://dev.epicgames.com/documentation/en-us/unreal-engine/API/Runtime/Engine/EngineUtils/TActorIterator
    for (TActorIterator<ATableActor> It(GetWorld()); It; ++It)
    {
        ATableActor* Table = *It;

        // OnOrderDelivered → HandleOrderDelivered
        // AddDynamic requires the target to be a UObject and the function
        // to be a UFUNCTION — both conditions are met here.
        Table->OnOrderDelivered.AddDynamic(
            this, &ACustomGameMode::HandleOrderDelivered);

        // OnOrderExpired → HandleOrderExpired
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

// ─────────────────────────────────────────────────────────────────────────────
//  HandleOrderDelivered  (bound to every table's OnOrderDelivered)
// ─────────────────────────────────────────────────────────────────────────────

void ACustomGameMode::HandleOrderDelivered(int32 TableNumber, bool bCorrect)
{
    if (bCorrect)
    {
        // ── Correct delivery ─────────────────────────────────────────────────
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
        // ── Wrong delivery ───────────────────────────────────────────────────
        // Clamp so score never goes negative.
        Score = FMath::Max(0, Score - PenaltyPerWrongDelivery);
        OrdersWrong++;

        UE_LOG(LogTemp, Warning,
            TEXT("ScriptedServiceGameMode: Table %d — Wrong dish! "
                 "-%d pts  (Score=%d)"),
            TableNumber, PenaltyPerWrongDelivery, Score);
    }

    BroadcastStats();
}

// ─────────────────────────────────────────────────────────────────────────────
//  HandleOrderExpired  (bound to every table's OnOrderExpired)
// ─────────────────────────────────────────────────────────────────────────────

void ACustomGameMode::HandleOrderExpired(int32 TableNumber)
{
    // Clamp so score never goes negative.
    Score = FMath::Max(0, Score - PenaltyPerExpiredOrder);
    OrdersExpired++;

    UE_LOG(LogTemp, Warning,
        TEXT("ScriptedServiceGameMode: Table %d — Order expired! "
             "-%d pts  (Score=%d)"),
        TableNumber, PenaltyPerExpiredOrder, Score);

    BroadcastStats();
}

// ─────────────────────────────────────────────────────────────────────────────
//  ResetStats
// ─────────────────────────────────────────────────────────────────────────────

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

// ─────────────────────────────────────────────────────────────────────────────
//  GetStatsDebugString
// ─────────────────────────────────────────────────────────────────────────────

FString ACustomGameMode::GetStatsDebugString() const
{
    return FString::Printf(
        TEXT("Score: %d\nTips: $%.2f\nCorrect: %d  Wrong: %d  Expired: %d"),
        Score, TotalTips, OrdersCorrect, OrdersWrong, OrdersExpired);
}

// ─────────────────────────────────────────────────────────────────────────────
//  BroadcastStats  (private — called after every mutation)
// ─────────────────────────────────────────────────────────────────────────────

void ACustomGameMode::BroadcastStats()
{
    // Guard: score must never be negative (belt-and-suspenders after the clamps
    // in the handlers above, but important if BroadcastStats is ever called
    // from other paths in the future).
    Score = FMath::Max(0, Score);

    // Notify all bound listeners (the HUD widget, Blueprint actors, etc.)
    OnStatsUpdated.Broadcast(Score, TotalTips, OrdersCorrect, OrdersWrong, OrdersExpired);
}