// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "RobotAIController.h"
#include "GameFramework/Character.h"
#include "StructTypes.h"
#include "RobotCharacter.generated.h"

class ARobotAIController;

UCLASS()
class SCRIPTED_SERVICE_API ARobotCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ARobotCharacter();

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Program)
	TArray<FRobotInstruction> CurrentProgram;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Program)
	int32 CurrentInstructionIndex = 0;
	
	UFUNCTION(BlueprintCallable, Category = "Order")
	void TakeOrderFromTable(int32 TableNumber);
    
	UFUNCTION(BlueprintCallable, Category = "Order")
	void PickupDishFromCounter();
    
	UFUNCTION(BlueprintCallable, Category = "Order")
	void DeliverDishToTable();
	
	UFUNCTION(BlueprintCallable, CallInEditor, Category = Program)
	void ExecuteProgram();

	UFUNCTION(BlueprintCallable, Category = Program)
	void ExecuteNextInstruction();
	
protected:
	virtual void BeginPlay() override;

	UPROPERTY()
	ARobotAIController* RobotController;

	UPROPERTY(BlueprintReadOnly, Category = "Order")
	FOrderData CurrentOrder;

	UPROPERTY(BlueprintReadOnly, Category = "Order")
	TSubclassOf<ABaseIngredient> CarryingDish;

	UPROPERTY(BlueprintReadOnly, Category = "Order")
	bool bIsCarryingDish = false;

	UFUNCTION(BlueprintCallable, Category = "Location")
	bool IsNearActor(AActor* TargetActor, float Distance = 200.0f);
};
