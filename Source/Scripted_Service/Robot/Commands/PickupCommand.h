// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "RobotCommand.h"
#include "PickupCommand.generated.h"

/**
 * 
 */
UCLASS()
class SCRIPTED_SERVICE_API UPickupCommand : public URobotCommand
{
	GENERATED_BODY()
public:
	void InitializePickup(ARobotCharacter* InRobot);
    
	virtual bool CanExecute() const override;
	virtual FString GetErrorMessage() const override;
	virtual void Execute() override;
	virtual FString GetDisplayName() const override;

private:
	static constexpr float INTERACTION_RANGE = 300.0f;
};
