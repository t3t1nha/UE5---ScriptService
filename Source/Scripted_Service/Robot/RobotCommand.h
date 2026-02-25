// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "RobotCommand.generated.h"

class ARobotCharacter;

/**
 * Delegate called when command execution completes
 */
DECLARE_DELEGATE(FOnCommandComplete);

/**
 * Delegate called when command execution fails
 */
DECLARE_DELEGATE_OneParam(FOnCommandError, FString);

/**
 * Base Class for all robot commands
 */
UCLASS()
class SCRIPTED_SERVICE_API URobotCommand : public UObject
{
	GENERATED_BODY()

public:
	URobotCommand();

	/**
	 * Initialize the command with owning robot
	 */
	virtual void Initialize(ARobotCharacter* InRobot);

	/**
	 * Check if this command can execute
	 * @return true if command can execute, false otherwise
	 */
	virtual bool CanExecute() const;

	/**
	 * Get Error message if CanExecute() returns false
	 */
	virtual FString GetErrorMessage() const;

	/**
	 * Execute the command
	 * Call OnComplete when Finished
	 */
	virtual void Execute();

	/**
	 * Cancel the command mid-execution
	 */
	virtual void Cancel();

	/**
	 * Get DisplayName for this command
	 */
	virtual FString GetDisplayName() const;

	FOnCommandComplete OnComplete;
	FOnCommandError OnError;

protected:
	/***
	 * The robot executing the command
	 */
	UPROPERTY()
	ARobotCharacter* OwningRobot;

	/**
	 * Error Message set by CanExecute()
	 */
	mutable FString ErrorMessage;

	/**
	 * Helper to complete command successfully
	 */
	void CompleteCommand();
	
	/**
	 * Helper to fail command with error
	 */
	void FailCommand(const FString& Error);
};
