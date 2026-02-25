// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BaseIngredient.h"
#include "StructTypes.h"
#include "IOrderable.h"
#include "GameFramework/Actor.h"
#include "TableActor.generated.h"

UCLASS()
class SCRIPTED_SERVICE_API ATableActor : public AActor, public IOrderable
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ATableActor();

	// Table identification
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Table")
	int32 TableNumber;
    
	// Current order at this table
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Table")
	FOrderData CurrentOrder;
    
	// IOrderable Interface Implementation
	virtual bool HasPendingOrder() const override;
	virtual FOrderData GetCurrentOrder() const override;
	virtual void PlaceOrder(TSubclassOf<class ABaseIngredient> Dish) override;
	virtual bool DeliverOrder(TSubclassOf<class ABaseIngredient> Dish) override;
	virtual FVector GetInteractionLocation() const override;
	virtual int32 GetOrderableID() const override;

protected:
	virtual void BeginPlay() override;


};
