// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BaseIngredient.h"
#include "Components/BoxComponent.h"
#include "KitchenCounter.generated.h"

UCLASS()
class SCRIPTED_SERVICE_API AKitchenCounter : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AKitchenCounter();

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UStaticMeshComponent* CounterMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UBoxComponent* PickupZone;
	
	UPROPERTY(EditAnywhere, Category = "Dishes")
	TArray<ABaseIngredient*> AvailableDishes;

	UFUNCTION(BlueprintCallable, Category = "Pickup")
	ABaseIngredient* PickupDish(TSubclassOf<ABaseIngredient> DishClass);

	UFUNCTION(BlueprintCallable, Category = "Pickup")
	bool HasDish(TSubclassOf<ABaseIngredient> DishClass);
	
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UFUNCTION()
	void OnPickupZoneBeginOverlap(UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult);

	UFUNCTION()
	void OnPickupZoneEndOverlap(UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex);
};
