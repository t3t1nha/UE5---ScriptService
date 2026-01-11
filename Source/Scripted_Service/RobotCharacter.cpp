// Fill out your copyright notice in the Description page of Project Settings.


#include "RobotCharacter.h"

#include <string>

// Sets default values
ARobotCharacter::ARobotCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void ARobotCharacter::BeginPlay()
{
	Super::BeginPlay();
	RobotController = Cast<ARobotAIController>(GetController());
}

// Called every frame
void ARobotCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void ARobotCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}


void ARobotCharacter::ExecuteProgram()
{
	CurrentInstructionIndex = 0;
	ExecuteNextInstruction();
}

void ARobotCharacter::ExecuteNextInstruction()
{
	if (CurrentInstructionIndex >= CurrentProgram.Num())
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, "Program Complete"); return;
	}

	FRobotInstruction CurrentInstruction = CurrentProgram[CurrentInstructionIndex];

	CurrentInstructionIndex++;

	switch (CurrentInstruction.InstructionType)
	{
	case EInstructionType::MoveToTable:
		if (RobotController)
		{
			RobotController->MoveToTableLocation(CurrentInstruction.TargetTableNumber);
		}
		else
		{
			GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, "Not Found");
		}
		break;

	case EInstructionType::MoveToKitchen:
		if (RobotController)
		{
			RobotController->MoveToKitchenLocation();
		}
		break;

	case EInstructionType::Wait:
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, "Waiting");
		ExecuteNextInstruction();
		break;

	default:
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, "Unhandled Instruction");
		ExecuteNextInstruction();
		break;
	}
}

