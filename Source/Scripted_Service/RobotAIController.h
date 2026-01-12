// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "TableManager.h"
#include "Navigation/PathFollowingComponent.h"
#include "RobotAIController.generated.h"

UCLASS()
class SCRIPTED_SERVICE_API ARobotAIController : public AAIController
{
	GENERATED_BODY()
	
public:
	ARobotAIController();

	UFUNCTION(BlueprintCallable, Category = Movement)
	void MoveToTableLocation(int32 TableNumber);

	UFUNCTION(BlueprintCallable, Category = Movement)
	void MoveToKitchenLocation();
	
protected:
	virtual void OnMoveCompleted(FAIRequestID RequestID, const FPathFollowingResult& Result) override;

	UPROPERTY()
	ATableManager* TableManager;
    
	void FindTableManager();
};
