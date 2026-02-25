// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "StructTypes.h"
#include "IOrderable.generated.h"

/**
 * 
 */
UINTERFACE(MinimalAPI, Blueprintable)
class UOrderable : public UInterface
{
	GENERATED_BODY()
};

/**
 * Interface for actors that can receive and fulfill orders
 * Implemented by: TableActor, (future: TakeoutCounter, DriveThruWindow, etc.)
 */
class IOrderable
{
	GENERATED_BODY()

public:
	/**
	 * Check if this orderable has a pending order waiting to be taken
	 */
	virtual bool HasPendingOrder() const = 0;
	
	/**
	 * Get the current order data
	 */
	virtual FOrderData GetCurrentOrder() const = 0;
    
	/**
	 * Place a new order at this location
	 * @param Dish - The dish class being ordered
	 */
	virtual void PlaceOrder(TSubclassOf<class ABaseIngredient> Dish) = 0;
	
	/**
	 * Attempt to deliver a dish to fulfill the order
	 * @param Dish - The dish being delivered
	 * @return true if delivery was successful (correct dish), false otherwise
	 */
	virtual bool DeliverOrder(TSubclassOf<class ABaseIngredient> Dish) = 0;
    
	/**
	 * Get the world location where the robot should stand to interact
	 */
	virtual FVector GetInteractionLocation() const = 0;
    
	/**
	 * Get the identifier for this orderable (e.g., table number)
	 */
	virtual int32 GetOrderableID() const = 0;
};
