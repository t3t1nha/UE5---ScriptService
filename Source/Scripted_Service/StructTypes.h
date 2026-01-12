// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Engine/DataTable.h"
#include "Modules/ModuleManager.h"
#include "CoreMinimal.h"
#include "EnumTypes.h"
#include "BaseIngredient.h"
#include "StructTypes.generated.h"

USTRUCT(BlueprintType)
struct FRecipeData : public FTableRowBase
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recipe Data")
	EApparatusType RequiredApparatus = EApparatusType::Stove;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recipe Data")
	TMap<TSubclassOf<ABaseIngredient> , int32> RequiredIngredients;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recipe Data")
	TMap<TSubclassOf<ABaseIngredient> , ECookState> RequiredCookState;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recipe Data")
	TSubclassOf<ABaseIngredient> OutputItemSubclass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recipe Data")
	float BaseCookTime = 5.0f;
};

USTRUCT(BlueprintType)
struct FOrderData
{
	GENERATED_BODY()
    
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<ABaseIngredient> RequestedDish;
    
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EOrderState OrderState = EOrderState::Waiting;
    
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 TableNumber = 0;
    
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TimeWaiting = 0.0f;
};

USTRUCT(BlueprintType)
struct FRobotInstruction
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EInstructionType InstructionType;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 TargetTableNumber = 0;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float WaitValue = 0.0f;
};