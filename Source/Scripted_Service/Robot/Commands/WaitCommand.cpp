// Fill out your copyright notice in the Description page of Project Settings.


#include "Commands/WaitCommand.h"
#include "RobotCharacter.h"

void UWaitCommand::InitializeWait(ARobotCharacter* InRobot, float InDuration)
{
	Initialize(InRobot);

	Duration = FMath::Max(0.05f, InDuration);
}

bool UWaitCommand::CanExecute() const
{
	return Super::CanExecute();
}

void UWaitCommand::Execute()
{
	if (!CanExecute())
	{
		FailCommand(GetErrorMessage());
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("Waiting for %.2f seconds"), Duration);

	FTimerDelegate Delegate;
	Delegate.BindUObject(this, &UWaitCommand::OnWaitComplete);
	OwningRobot->GetWorldTimerManager().SetTimer(TimerHandle, Delegate, Duration, false);
}

void UWaitCommand::Cancel()
{
	Super::Cancel();

	if (OwningRobot)
	{
		OwningRobot->GetWorldTimerManager().ClearTimer(TimerHandle);
	}
}

FString UWaitCommand::GetDisplayName() const
{
	return FString::Printf(TEXT("Wait %.1f s"), Duration);
}

void UWaitCommand::OnWaitComplete()
{
	CompleteCommand();
}
