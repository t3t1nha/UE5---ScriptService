// Fill out your copyright notice in the Description page of Project Settings.


#include "MoveCommand.h"

#include "RobotAIController.h"
#include "RobotCharacter.h"

void UMoveCommand::InitializeMoveCommand(ARobotCharacter* InRobot, const FVector& InTargetLocation)
{
	Initialize(InRobot);
	TargetLocation = InTargetLocation;
}

bool UMoveCommand::CanExecute() const
{
	if (!Super::CanExecute())
	{
		return false;
	}

	// Check if target location is valid
	if (TargetLocation.IsZero())
	{
		ErrorMessage = TEXT("Invalid Target Location (zero vector)");
		return false;
	}

	// Check if robot has AI controller
	ARobotAIController* AIController = Cast<ARobotAIController>(OwningRobot->GetController());
	if (!AIController)
	{
		ErrorMessage = TEXT("Robot has no AI Controller");
		return false;
	}

	return true;
}

FString UMoveCommand::GetErrorMessage() const
{
	if (Super::GetErrorMessage().IsEmpty())
	{
		return ErrorMessage;
	}

	return Super::GetErrorMessage();
}

void UMoveCommand::Execute()
{
	if (!CanExecute())
	{
		FailCommand(GetErrorMessage());
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("MoveCommand: Moving to location %s"), *TargetLocation.ToString());

	ARobotAIController* AIController = Cast<ARobotAIController>(OwningRobot->GetController());
	if (AIController)
	{
		// Bind Callback for when movement completes
		AIController->OnMoveComplete.BindUObject(this, &UMoveCommand::OnMovementComplete);

		// Start Movement
		AIController->MoveToSpecificLocation(TargetLocation);
	}
}

void UMoveCommand::Cancel()
{
	Super::Cancel();

	ARobotAIController* AIController = Cast<ARobotAIController>(OwningRobot->GetController());
	if (AIController)
	{
		AIController->StopMovement();
	}
}

FString UMoveCommand::GetDisplayName() const
{
	return FString::Printf(TEXT("Move to %s"), *TargetLocation.ToCompactString());
}

void UMoveCommand::OnMovementComplete()
{
	UE_LOG(LogTemp, Log, TEXT("MoveCommand: Movement complete"));
	CompleteCommand();
}








