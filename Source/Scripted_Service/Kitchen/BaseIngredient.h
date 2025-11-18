// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Interface/GrabableInterface.h"
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BaseIngredient.generated.h"

UCLASS()
class SCRIPTED_SERVICE_API ABaseIngredient : public AActor, public IGrabableInterface
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ABaseIngredient();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	virtual void OnConstruction(const FTransform& Transform) override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UStaticMeshComponent* ItemMeshComponent;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
	FName ItemID = NAME_None;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Config")
	UDataTable* ItemBaseDataTable;

	UFUNCTION(BlueprintCallable, Category = "Visuals")
	void UpdateVisuals();
};
