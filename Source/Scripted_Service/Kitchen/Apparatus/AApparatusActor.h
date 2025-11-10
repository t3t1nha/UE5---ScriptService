// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "TimerManager.h"
#include "Components/BoxComponent.h"
#include "components/MeshComponent.h"
#include "Engine/DataTable.h"
#include "CookingSystemTypes.h"
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "AApparatusActor.generated.h"

UCLASS()
class SCRIPTED_SERVICE_API AAApparatusActor : public AActor
{
	GENERATED_BODY()
	
public:	
	AAApparatusActor();

protected:
	virtual void BeginPlay() override;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UMeshComponent* MeshComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UBoxComponent* TriggerBoxComponent;

public:
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EApparatusType ApparatusType;

	UPROPERTY(BlueprintReadWrite)
	TMap<FName, int32> CurrentIngredients;

	UPROPERTY(EditAnywhere, Category = "DataTable")
	UDataTable* RecipeDataTable;
	
	UPROPERTY(BlueprintReadWrite)
	FRecipeData CurrentRecipeData;

	UPROPERTY(BlueprintReadWrite)
	FTimerHandle CookingTimerHandle;

	UFUNCTION(BlueprintCallable)
	void CheckForRecipe();

	UFUNCTION(BlueprintCallable)
	void AddIngredient(FName ItemID);

	UFUNCTION(BlueprintCallable)
	void RemoveIngredient(FName ItemID);
	
	UFUNCTION(BlueprintCallable)
	void StartCookingProcess();

	UFUNCTION()
	void FinishCooking();

	UFUNCTION(BlueprintCallable)
	void OnTriggerBoxComponentOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
		bool bFromSweep, const FHitResult& SweepResult);
};
