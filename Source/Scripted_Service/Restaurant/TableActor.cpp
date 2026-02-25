// Fill out your copyright notice in the Description page of Project Settings.


#include "TableActor.h"

ATableActor::ATableActor()
{
	PrimaryActorTick.bCanEverTick = false;
	TableNumber = 0;

	TableMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("TableMesh"));
	RootComponent = TableMesh;
}

void ATableActor::BeginPlay()
{
	Super::BeginPlay();
    
	// Initialize order state
	CurrentOrder.OrderState = EOrderState::Waiting;
	CurrentOrder.RequestedDish = nullptr;
	CurrentOrder.TableNumber = TableNumber;
	CurrentOrder.TimeWaiting = 0.0f;
}

/**
 *IOrderable INTERFACE IMPLEMENTATION
 */
bool ATableActor::HasPendingOrder() const
{
	return CurrentOrder.OrderState == EOrderState::Waiting;
}

FOrderData ATableActor::GetCurrentOrder() const
{
	return CurrentOrder;
}

void ATableActor::PlaceOrder(TSubclassOf<ABaseIngredient> Dish)
{
	if (!Dish)
	{
		UE_LOG(LogTemp, Warning, TEXT("Table %d: Attempted to place order with null dish"), TableNumber);
		return;
	}
    
	CurrentOrder.RequestedDish = Dish;
	CurrentOrder.OrderState = EOrderState::Waiting;
	CurrentOrder.TableNumber = TableNumber;
	CurrentOrder.TimeWaiting = 0.0f;
    
	UE_LOG(LogTemp, Log, TEXT("Table %d: Order placed for %s"), 
		TableNumber, 
		*Dish->GetName());
}

bool ATableActor::DeliverOrder(TSubclassOf<ABaseIngredient> Dish)
{
	// Check if delivered dish matches order
	if (Dish == CurrentOrder.RequestedDish)
	{
		CurrentOrder.OrderState = EOrderState::Delivered;
		UE_LOG(LogTemp, Log, TEXT("Table %d: Correct dish delivered!"), TableNumber);
		return true;
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Table %d: Wrong dish delivered!"), TableNumber);
		return false;
	}
}

FVector ATableActor::GetInteractionLocation() const
{
	return GetActorLocation();
}

int32 ATableActor::GetOrderableID() const
{
	return TableNumber;
}