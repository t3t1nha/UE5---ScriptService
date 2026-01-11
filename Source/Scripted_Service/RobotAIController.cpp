// Fill out your copyright notice in the Description page of Project Settings.


#include "RobotAIController.h"

#include <string>

#include "RobotCharacter.h"

ARobotAIController::ARobotAIController()
{
}

void ARobotAIController::MoveToTableLocation(int32 TableNumber)
{
	// TODO: Find Actual Table actor by number

	FVector TestLocation = FVector(500.0f ,500.0f * TableNumber, 88.0f);

	MoveToLocation(TestLocation);

	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, ("Moving to table " + std::to_string(TableNumber)).data());
}

void ARobotAIController::MoveToKitchenLocation()
{
	// TODO: Find Actual Kitchen Location
	FVector TestLocation = FVector(0.0f, 0.0f,0.0f);

	MoveToLocation(TestLocation);
	
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, "Moving to Kitchen");
}

void ARobotAIController::OnMoveCompleted(FAIRequestID RequestID, const FPathFollowingResult& Result)
{
	Super::OnMoveCompleted(RequestID, Result);

	// Execute next instruction
	if (ARobotCharacter* Robot = Cast<ARobotCharacter>(GetPawn()))
	{
		Robot->ExecuteNextInstruction();
	}
}
