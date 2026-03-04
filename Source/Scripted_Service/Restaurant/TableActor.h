// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BaseIngredient.h"
#include "StructTypes.h"
#include "IOrderable.h"
#include "GameFramework/Actor.h"
#include "TableActor.generated.h"

/**
 * Broadcast when this table generates a new random order.
 * @param TableNumber  - Which table placed the order
 * @param OrderData    - Full data for the new order (dish class, state, etc.)
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
	FOnOrderPlaced,
	int32, TableNumber,
	FOrderData, OrderData
);

/**
 * Broadcast when an order that was never taken expires (timed out).
 * @param TableNumber  - Which table's order timed out
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
	FOnOrderExpired,
	int32, TableNumber
);

/**
 * Broadcast when an order is successfully delivered to this table.
 * @param TableNumber  - Which table received the delivery
 * @param bCorrect     - true if the right dish was delivered
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
	FOnOrderDelivered,
	int32, TableNumber,
	bool, bCorrect
);

UCLASS()
class SCRIPTED_SERVICE_API ATableActor : public AActor, public IOrderable
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ATableActor();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UStaticMeshComponent* TableMesh;
	
	// Table identification
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Table")
	int32 TableNumber;
    
	// Current order at this table
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Table")
	FOrderData CurrentOrder;

	/**
	 * Pool of dishes this table can randomly order.
	 * If left empty, no orders will be generated automatically.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Table|Order Generation")
	TArray<TSubclassOf<ABaseIngredient>> PossibleDishes;

	/**
	 * Minimum seconds before a new random order is generated after the
	 * previous one was delivered (or after BeginPlay on first order).
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Table|Order Generation",
		meta = (ClampMin = "1.0", UIMin = "1.0"))
	float MinOrderInterval = 10.0f;

	/**
	 * Maximum seconds before a new random order is generated.
	 * Must be >= MinOrderInterval.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Table|Order Generation",
		meta = (ClampMin = "1.0", UIMin = "1.0"))
	float MaxOrderInterval = 30.0f;

	/**
	 * How long (seconds) a Waiting order stays valid before it expires and
	 * the table gives up waiting.  Set to 0 to disable timeouts.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Table|Order Generation",
		meta = (ClampMin = "0.0", UIMin = "0.0"))
	float OrderTimeoutDuration = 60.0f;

	/**
	 * If true, a new order will be generated on BeginPlay automatically.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Table|Order Generation")
	bool bGenerateOrderOnBeginPlay = true;

	/** Fires every time this table generates a new randomised order */
	UPROPERTY(BlueprintAssignable, Category = "Table|Events")
	FOnOrderPlaced OnOrderPlaced;

	/** Fires when a Waiting order times out before the robot takes it */
	UPROPERTY(BlueprintAssignable, Category = "Table|Events")
	FOnOrderExpired OnOrderExpired;

	/** Fires when an order is delivered (right or wrong dish) */
	UPROPERTY(BlueprintAssignable, Category = "Table|Events")
	FOnOrderDelivered OnOrderDelivered;

    
	// IOrderable Interface Implementation
	virtual bool HasPendingOrder() const override;
	virtual FOrderData GetCurrentOrder() const override;
	UFUNCTION(BlueprintCallable)
	virtual void PlaceOrder(TSubclassOf<class ABaseIngredient> Dish) override;
	virtual bool DeliverOrder(TSubclassOf<class ABaseIngredient> Dish) override;
	virtual FVector GetInteractionLocation() const override;
	virtual int32 GetOrderableID() const override;

	/**
	 * Immediately pick a random dish from PossibleDishes and place an order.
	 * Safe to call from Blueprint at any time.
	 */
	UFUNCTION(BlueprintCallable, Category = "Table|Order")
	void GenerateRandomOrder();

	/**
	 * Cancel any active order and timer — useful for resetting the table
	 * from a game-mode or manager script.
	 */
	UFUNCTION(BlueprintCallable, Category = "Table|Order")
	void CancelCurrentOrder();

protected:
	virtual void BeginPlay() override;

private:
	/** Fires GenerateRandomOrder() after a random delay */
	FTimerHandle OrderGenerationTimerHandle;

	/** Fires OnOrderTimedOut() if the order was never taken in time */
	FTimerHandle OrderTimeoutTimerHandle;
	
	/**
	 * Schedule the next random order using a random delay in
	 * [MinOrderInterval, MaxOrderInterval].
	 */
	void ScheduleNextOrder();

	/**
	 * Called by the timeout timer when an order was never taken.
	 * Clears the order and re-schedules a new one.
	 */
	void OnOrderTimedOut();

	/**
	 * Clear the order timeout timer without firing the timeout callback.
	 */
	void ClearTimeoutTimer();
	
};
