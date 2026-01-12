// Fill out your copyright notice in the Description page of Project Settings.


#include "TableActor.h"

// Sets default values
ATableActor::ATableActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	TableMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("TableMesh"));
	SetRootComponent(TableMesh);
}

bool ATableActor::HasWaitingOrder() const
{
	return bHasOrder && CurrentOrder.OrderState == EOrderState::Waiting;
}

FOrderData ATableActor::GetOrder() const
{
	return CurrentOrder;
}

void ATableActor::PlaceOrder(TSubclassOf<ABaseIngredient> DishClass)
{
	if (!DishClass)
	{
		GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Red, TEXT("Invalid dish class!"));
		return;
	}

	CurrentOrder.RequestedDish = DishClass;
	CurrentOrder.OrderState = EOrderState::Waiting;
	CurrentOrder.TableNumber = TableNumber;
	CurrentOrder.TimeWaiting = 0.0f;
	bHasOrder = true;

	GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Green, 
		   FString::Printf(TEXT("Table %d ordered: %s"), TableNumber, *DishClass->GetName()));
}

bool ATableActor::DeliverDish(TSubclassOf<ABaseIngredient> DeliveredDish)
{
	if (!bHasOrder)
	{
		GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Red, TEXT("Table has no order!"));
		return false;
	}

	bool bCorrectDish = (DeliveredDish == CurrentOrder.RequestedDish);

	if (bCorrectDish)
	{
		GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Green, 
				FString::Printf(TEXT("Table %d: Correct dish delivered!"), TableNumber));

		CurrentOrder.OrderState = EOrderState::Delivered;
		bHasOrder = false;

		// TODO: Calculate Profit based on TimeWaiting
		// Fast = More profit, slow = less profit
		
		return true;
	}
	else
	{
		GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Red, 
				FString::Printf(TEXT("Table %d: WRONG dish! Customer unhappy!"), TableNumber));

		CurrentOrder.OrderState = EOrderState::Delivered;
		bHasOrder = false;

		// Apply profit penalty
		
		return false;
	}
}

// Called when the game starts or when spawned
void ATableActor::BeginPlay()
{
	Super::BeginPlay();
}

// Called every frame
void ATableActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bHasOrder && CurrentOrder.OrderState == EOrderState::Waiting)
	{
		CurrentOrder.TimeWaiting += DeltaTime;
	}

}

