// IPickupPoint.h
#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "IPickupPoint.generated.h"

UINTERFACE(MinimalAPI, Blueprintable)
class UPickupPoint : public UInterface
{
	GENERATED_BODY()
};

/**
 * Interface for locations where robot can pickup items
 * Implemented by: KitchenCounter, (future: ServingStation, WarmerBox, etc.)
 */
class IPickupPoint
{
	GENERATED_BODY()

public:
	/**
	 * Check if a specific item is available for pickup
	 * @param ItemClass - The class of item to check for
	 */
	virtual bool HasItem(TSubclassOf<class ABaseIngredient> ItemClass) const = 0;
    
	/**
	 * Attempt to pickup an item
	 * @param ItemClass - The class of item to pickup
	 * @return true if pickup successful, false otherwise
	 */
	virtual bool PickupItem(TSubclassOf<class ABaseIngredient> ItemClass) = 0;
    
	/**
	 * Get the world location where robot should stand to pickup
	 */
	virtual FVector GetPickupLocation() const = 0;
    
	/**
	 * Get list of all available items at this pickup point
	 */
	virtual TArray<TSubclassOf<class ABaseIngredient>> GetAvailableItems() const = 0;
};