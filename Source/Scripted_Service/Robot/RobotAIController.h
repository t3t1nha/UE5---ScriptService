// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "RobotAIController.generated.h"

// Delegate for movement completion
DECLARE_DELEGATE(FOnMoveCompleted);

UCLASS()
class SCRIPTED_SERVICE_API ARobotAIController : public AAIController
{
	GENERATED_BODY()
	
public:
	ARobotAIController();

	/***
	 * Move AI to target location
	 */
	UFUNCTION(Blueprintable, Category = "AI")
	void MoveToSpecificLocation(FVector TargetLocation);

	/**
	 * Delegate called when movement completes
	 */
	FOnMoveCompleted OnMoveComplete;
protected:
	virtual void OnMoveCompleted(FAIRequestID RequestID, const FPathFollowingResult& Result) override;
};
