// Fill out your copyright notice in the Description page of Project Settings.


#include "Commands/PickupCommand.h"
#include "RobotCharacter.h"
#include "TableManager.h"
#include "KitchenCounter.h"
#include "IPickupPoint.h"
#include "EngineUtils.h"

void UPickupCommand::InitializePickup(ARobotCharacter* InRobot)
{
	Initialize(InRobot);
}

bool UPickupCommand::CanExecute() const
{
	if (!Super::CanExecute())
	{
		return false;
	}
    
	// Find TableManager
	ATableManager* TableManager = nullptr;
	for (TActorIterator<ATableManager> It(OwningRobot->GetWorld()); It; ++It)
	{
		TableManager = *It;
		break;
	}
    
	if (!TableManager)
	{
		ErrorMessage = TEXT("No TableManager found");
		return false;
	}
    
	AKitchenCounter* Counter = TableManager->GetKitchenCounter();
	if (!Counter)
	{
		ErrorMessage = TEXT("No kitchen counter found");
		return false;
	}
    
	IPickupPoint* PickupPoint = Cast<IPickupPoint>(Counter);
	if (!PickupPoint)
	{
		ErrorMessage = TEXT("Counter does not implement IPickupPoint");
		return false;
	}
    
	// Check distance
	FVector CounterLocation = PickupPoint->GetPickupLocation();
	float Distance = FVector::Dist(OwningRobot->GetActorLocation(), CounterLocation);
    
	if (Distance > INTERACTION_RANGE)
	{
		ErrorMessage = FString::Printf(TEXT("Too far from counter (%.1f cm)"), Distance);
		return false;
	}
    
	// Check if we have an order
	if (!OwningRobot->CurrentOrder.RequestedDish)
	{
		ErrorMessage = TEXT("No order to pickup dish for");
		return false;
	}
    
	// Check if dish is available
	if (!PickupPoint->HasItem(OwningRobot->CurrentOrder.RequestedDish))
	{
		ErrorMessage = TEXT("Required dish not available on counter");
		return false;
	}
    
	return true;
}

FString UPickupCommand::GetErrorMessage() const
{
	if (!ErrorMessage.IsEmpty())
	{
		return ErrorMessage;
	}
	return Super::GetErrorMessage();
}

void UPickupCommand::Execute()
{
	if (!CanExecute())
	{
		FailCommand(GetErrorMessage());
		return;
	}
    
	// Find counter
	ATableManager* TableManager = nullptr;
	for (TActorIterator<ATableManager> It(OwningRobot->GetWorld()); It; ++It)
	{
		TableManager = *It;
		break;
	}
    
	AKitchenCounter* Counter = TableManager->GetKitchenCounter();
	IPickupPoint* PickupPoint = Cast<IPickupPoint>(Counter);
    
	// Pickup the dish
	if (PickupPoint->PickupItem(OwningRobot->CurrentOrder.RequestedDish))
	{
		OwningRobot->CarryingDish = OwningRobot->CurrentOrder.RequestedDish;
		OwningRobot->CurrentOrder.OrderState = EOrderState::Ready;
        
		UE_LOG(LogTemp, Log, TEXT("PickupCommand: Picked up dish"));
        
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Green, TEXT("Dish picked up!"));
		}
        
		CompleteCommand();
	}
	else
	{
		FailCommand(TEXT("Failed to pickup dish from counter"));
	}
}

FString UPickupCommand::GetDisplayName() const
{
	return TEXT("Pickup Food from Counter");
}
