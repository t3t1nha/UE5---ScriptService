// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "RobotCommand.h"
#include "MoveCommand.generated.h"

/**
 * Move Robot to a specific location
 */
UCLASS()
class SCRIPTED_SERVICE_API UMoveCommand : public URobotCommand
{
	GENERATED_BODY()

public:
	/**
	 * Initialize move command with target location
	 */
	void InitializeMoveCommand(ARobotCharacter* InRobot, const FVector& InTargetLocation);

	virtual bool CanExecute() const override;
	virtual FString GetErrorMessage() const override;
	virtual void Execute() override;
	virtual void Cancel() override;
	virtual FString GetDisplayName() const override;

private:
	FVector TargetLocation;

	/**
	 * Called when AI Controller completes movement
	 */
	void OnMovementComplete();
};
