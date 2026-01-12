// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BaseIngredient.h"
#include "StructTypes.h"
#include "GameFramework/Actor.h"
#include "TableActor.generated.h"

UCLASS()
class SCRIPTED_SERVICE_API ATableActor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ATableActor();

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Table")
	int32 TableNumber = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Table")
	FOrderData CurrentOrder;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Table")
	bool bHasOrder = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UStaticMeshComponent* TableMesh;

	UFUNCTION(BlueprintCallable, Category = "Order")
	bool HasWaitingOrder() const;

	UFUNCTION(BlueprintCallable, Category = "Order")
	FOrderData GetOrder() const;

	UFUNCTION(BlueprintCallable, CallInEditor, Category = "Order")
	void PlaceOrder(TSubclassOf<ABaseIngredient> DishClass);

	UFUNCTION(BlueprintCallable, Category = "Order")
	bool DeliverDish(TSubclassOf<ABaseIngredient> DeliveredDish);
	
protected:
	virtual void BeginPlay() override;


};
