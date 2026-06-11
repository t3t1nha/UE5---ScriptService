// Fill out your copyright notice in the Description page of Project Settings.


#include "CustomGameMode.h"
#include "EngineUtils.h"
#include "TableActor.h"

ACustomGameMode::ACustomGameMode()
{
    // No Tick needed — all stat changes are event-driven.
    PrimaryActorTick.bCanEverTick = false;
}

void ACustomGameMode::BeginPlay()
{
    Super::BeginPlay();

    SubscribeToAllTables();
    
    BroadcastStats();
}

void ACustomGameMode::SubscribeToAllTables()
{
    NumberOfTables = 0;

    for (TActorIterator<ATableActor> It(GetWorld()); It; ++It)
    {
        ATableActor* Table = *It;

        Table->OnOrderDelivered.AddDynamic(
            this, &ACustomGameMode::HandleOrderDelivered);

        Table->OnOrderExpired.AddDynamic(
            this, &ACustomGameMode::HandleOrderExpired);

        ++NumberOfTables;
    }

    if (NumberOfTables == 0)
    {
        UE_LOG(LogTemp, Warning,
            TEXT("ScriptedServiceGameMode: No ATableActor found in the level. "
                 "Place at least one table and make sure it derives from ATableActor."));
    }
}

void ACustomGameMode::HandleOrderDelivered(int32 TableNumber, float IngredientPrice)
{
        Score       += PointsPerCorrectDelivery;
        TotalTips   += IngredientPrice;

    BroadcastStats();
}

void ACustomGameMode::HandleOrderExpired(int32 TableNumber)
{
    Score = FMath::Max(0, Score - PenaltyPerExpiredOrder);

    BroadcastStats();
}


void ACustomGameMode::ResetStats()
{
    Score         = 0;
    MaxNumberOfOrders = 2;
    TotalTips     = 20.0f;

    UE_LOG(LogTemp, Log, TEXT("ScriptedServiceGameMode: Stats reset."));

    BroadcastStats();
}

bool ACustomGameMode::IncreaseOrderCount()
{
    if (ActiveOrders >= MaxNumberOfOrders)
    {
        return false;
    }
    
    ActiveOrders++;
    return true;
}

bool ACustomGameMode::SpendMoney(float Amount)
{
    if (TotalTips < Amount)
    {
        return false;
    }

    TotalTips -= Amount;
    BroadcastStats();

    return true;
}

void ACustomGameMode::UpdateMaxNumberOfOrder(int value)
{
    if (MaxNumberOfOrders >= NumberOfTables){
        MaxNumberOfOrders += value;
    }
}


void ACustomGameMode::BroadcastStats()
{
    Score = FMath::Max(0, Score);

    OnStatsUpdated.Broadcast(Score, TotalTips);
}
