// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "RobotCommand.h"
#include "MoveCommand.generated.h"

UENUM(BlueprintType)
enum class EMoveTargetType : uint8
{
	Location,      // Move to specific coordinates
	Table,         // Move near a table
	Kitchen        // Move to kitchen counter
};


UCLASS()
class SCRIPTED_SERVICE_API UMoveCommand : public URobotCommand
{
	GENERATED_BODY()

public:
	/**
	 * Initialize move to specific location
	 */
	void InitializeMoveToLocation(ARobotCharacter* InRobot, const FVector& InTargetLocation);
    
	/**
	 * Initialize move to table (calculates safe position near table)
	 */
	void InitializeMoveToTable(ARobotCharacter* InRobot, int32 TableNumber);
    
	/**
	 * Initialize move to kitchen counter
	 */
	void InitializeMoveToKitchen(ARobotCharacter* InRobot);
	
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
	EMoveTargetType MoveType;
	FVector TargetLocation;
	int32 TargetTableNumber;
	
	static constexpr float TABLE_STOP_DISTANCE = 200.0f;
	
	/**
	 * Called when AI Controller completes movement
	 */
	void OnMovementComplete();
	
	FVector CalculateTableTargetLocation() const;
};
