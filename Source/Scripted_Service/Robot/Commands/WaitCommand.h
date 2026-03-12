// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "RobotCommand.h"
#include "WaitCommand.generated.h"


UCLASS()
class SCRIPTED_SERVICE_API UWaitCommand : public URobotCommand
{
	GENERATED_BODY()

public:
	void InitializeWait(ARobotCharacter* InRobot, float InDuration);

	virtual bool CanExecute() const override;
	virtual void Execute() override;
	virtual void Cancel() override;
	virtual FString GetDisplayName() const override;

private:
	float Duration = 1.0f;

	FTimerHandle TimerHandle;

	UFUNCTION()
	void OnWaitComplete();
};
