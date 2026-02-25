// Fill out your copyright notice in the Description page of Project Settings.


#include "RobotAIController.h"
#include "RobotCharacter.h"
#include "Navigation/PathFollowingComponent.h"

ARobotAIController::ARobotAIController()
{
}

void ARobotAIController::MoveToSpecificLocation(FVector TargetLocation)
{
	FAIMoveRequest MoveRequest;
	MoveRequest.SetGoalLocation(TargetLocation);
	MoveRequest.SetAcceptanceRadius(5.0f);

	FNavPathSharedPtr NavPath;
	MoveTo(MoveRequest, &NavPath);

	UE_LOG(LogTemp, Log, TEXT("AI Controller: Moving to location %s"), *TargetLocation.ToString());
}

void ARobotAIController::OnMoveCompleted(FAIRequestID RequestID, const FPathFollowingResult& Result)
{
	Super::OnMoveCompleted(RequestID, Result);

	if (Result.IsSuccess())
	{
		UE_LOG(LogTemp, Log, TEXT("AI Controller: Movement successful"));

		// Notify listeners (the command)
		if (OnMoveComplete.IsBound())
		{
			OnMoveComplete.Execute();
		}

		// ALSO notify the old system
		ARobotCharacter* Robot = Cast<ARobotCharacter>(GetPawn());
		if (Robot)
		{
			Robot->OnMovementComplete();
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("AI Controller: Movement failed"));
	}
}