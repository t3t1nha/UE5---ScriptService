// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TimerManager.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/DataTable.h"
#include "Interface/InteractInterface.h"
#include "CookingSystemTypes.h"
#include "AApparatusActor.generated.h"

class UStaticMesh;
class UBoxComponent;
class UDataTable;

UCLASS()
class SCRIPTED_SERVICE_API AAApparatusActor : public AActor, public IInteractInterface
{
	GENERATED_BODY()
	
public:	
	AAApparatusActor();

protected:
	virtual void BeginPlay() override;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UStaticMeshComponent* ApparatusMeshComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UBoxComponent* DropZoneComponent;
	
public:
	virtual void Tick(float DeltaTime) override;

	virtual void Interact_Implementation() override;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
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

	UFUNCTION()
	virtual void OnDropZoneOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	virtual void OnDropZoneOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);
};
