// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "RobotCommand.h"
#include "TakeOrderCommand.generated.h"

/**
 * 
 */
UCLASS()
class SCRIPTED_SERVICE_API UTakeOrderCommand : public URobotCommand
{
	GENERATED_BODY()

public:
	void InitializeTakeOrder(ARobotCharacter* InRobot, int32 InTableNumber);

	virtual bool CanExecute() const override;
	virtual FString GetErrorMessage() const override;
	virtual void Execute() override;
	virtual FString GetDisplayName() const override;

private:
	int32 TableNumber;

	static constexpr float INTERACTION_RANGE = 300.0f;
};
