// Fill out your copyright notice in the Description page of Project Settings.


#include "RobotCommand.h"


URobotCommand::URobotCommand()
{
	OwningRobot = nullptr;
}

void URobotCommand::Initialize(ARobotCharacter* InRobot)
{
	OwningRobot = InRobot;
}

bool URobotCommand::CanExecute() const
{
	if (!OwningRobot)
	{
		return false;
	}
	
	return true;
}

FString URobotCommand::GetErrorMessage() const
{
	if (!OwningRobot)
	{
		return TEXT("No Robot Assigned to command");
	}

	return ErrorMessage;
}

void URobotCommand::Execute()
{
	UE_LOG(LogTemp, Warning, TEXT("Base URobotCommand::Execute() called - should be overriden"))
	CompleteCommand();
}

void URobotCommand::Cancel()
{
	UE_LOG(LogTemp, Log, TEXT("%s: Command cancelled"), *GetDisplayName());
}

FString URobotCommand::GetDisplayName() const
{
	return TEXT("Base Command");
}

void URobotCommand::CompleteCommand()
{
	if (OnComplete.IsBound())
	{
		OnComplete.Execute();
	}
}

void URobotCommand::FailCommand(const FString& Error)
{
	UE_LOG(LogTemp, Error, TEXT("%s: Failed - %s"), *GetDisplayName(), *Error);

	if (OnError.IsBound())
	{
		OnError.Execute(Error);
	}
}












