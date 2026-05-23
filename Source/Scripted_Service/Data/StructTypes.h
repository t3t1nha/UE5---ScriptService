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
struct FBlockData
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EInstructionType InstructionType;
    
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText DisplayName;
    
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FLinearColor BlockColor = FLinearColor::White;
    
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Category = "Movement"; // "Movement", "Actions", "Logic"
    
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bHasTableParameter = false;
    
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bHasWaitParameter = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsContainerBlock = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bHasLoopCountParameter = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bCanSaveToSlot = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bCanReadTableFromSlot = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bHasSlotIndexParameter = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bHasSlotValueParameter = false;
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

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 LoopCount = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bSaveToSlot = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 SaveToSlotIndex = 0;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bReadTableFromSlot = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 ReadFromSlotIndex = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 SlotIndex = 0;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 SlotValue = 0;
};

USTRUCT(BlueprintType)
struct FRobotMemorySlot
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName SlotName = NAME_None;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	int32 IntValue = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	bool bIsSet = false;
};
