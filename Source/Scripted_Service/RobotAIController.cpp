// Fill out your copyright notice in the Description page of Project Settings.


#include "RobotAIController.h"
#include <string>
#include "EngineUtils.h"
#include "RobotCharacter.h"
#include "TableActor.h"

ARobotAIController::ARobotAIController()
{
}

void ARobotAIController::MoveToTableLocation(int32 TableNumber)
{
	FindTableManager();
    
	if (!TableManager)
	{
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Red, TEXT("No TableManager found!"));
		}
		return;
	}
    
	ATableActor* Table = TableManager->FindTableByNumber(TableNumber);
    
	if (Table)
	{
		MoveToActor(Table);
        
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Yellow, 
				FString::Printf(TEXT("Moving to Table %d"), TableNumber));
		}
	}
}

void ARobotAIController::MoveToKitchenLocation()
{
	FindTableManager();
    
	if (!TableManager)
	{
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Red, TEXT("No TableManager found!"));
		}
		return;
	}
    
	// Try to move to the kitchen counter if it exists
	AKitchenCounter* Counter = TableManager->GetKitchenCounter();
    
	if (Counter)
	{
		MoveToActor(Counter, 100.0f); // Stop 100cm away
        
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Yellow, TEXT("Moving to Kitchen Counter"));
		}
	}
	else
	{
		// Fallback to stored location
		FVector KitchenLoc = TableManager->GetKitchenLocation();
		MoveToLocation(KitchenLoc);
        
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Yellow, TEXT("Moving to Kitchen Location"));
		}
	}
}

void ARobotAIController::OnMoveCompleted(FAIRequestID RequestID, const FPathFollowingResult& Result)
{
	Super::OnMoveCompleted(RequestID, Result);
    
	if (GEngine)
	{
		FString ResultStr = Result.IsSuccess() ? TEXT("SUCCESS") : TEXT("FAILED");
		GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Magenta, 
			FString::Printf(TEXT("Movement completed: %s"), *ResultStr));
	}
    
	if (ARobotCharacter* Robot = Cast<ARobotCharacter>(GetPawn()))
	{
		Robot->ExecuteNextInstruction();
	}
}

void ARobotAIController::FindTableManager()
{
	if (!TableManager)
	{
		for (TActorIterator<ATableManager> It(GetWorld()); It; ++It)
		{
			TableManager = *It;
			break;
		}
	}
}
