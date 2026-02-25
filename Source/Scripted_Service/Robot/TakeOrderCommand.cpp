// Fill out your copyright notice in the Description page of Project Settings.

#include "TakeOrderCommand.h"
#include "RobotCharacter.h"
#include "TableManager.h"
#include "TableActor.h"
#include "IOrderable.h"
#include "StructTypes.h"
#include "EngineUtils.h"

void UTakeOrderCommand::InitializeTakeOrder(ARobotCharacter* InRobot, int32 InTableNumber)
{
	Initialize(InRobot);
	TableNumber = InTableNumber;
}

bool UTakeOrderCommand::CanExecute() const
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
		ErrorMessage = TEXT("No TableManager found in level");
		return false;
	}
    
	// Find table
	ATableActor* Table = TableManager->FindTableByNumber(TableNumber);
	if (!Table)
	{
		ErrorMessage = FString::Printf(TEXT("Table %d not found"), TableNumber);
		return false;
	}
    
	// Cast to interface
	IOrderable* Orderable = Cast<IOrderable>(Table);
	if (!Orderable)
	{
		ErrorMessage = FString::Printf(TEXT("Table %d does not implement IOrderable"), TableNumber);
		return false;
	}
    
	// Check if robot is close enough
	FVector TableLocation = Orderable->GetInteractionLocation();
	float Distance = FVector::Dist(OwningRobot->GetActorLocation(), TableLocation);
    
	if (Distance > INTERACTION_RANGE)
	{
		ErrorMessage = FString::Printf(TEXT("Too far from table %d (Distance: %.2f)"), TableNumber, Distance);
		return false;
	}
    
	// Check if table has an order
	if (!Orderable->HasPendingOrder())
	{
		ErrorMessage = FString::Printf(TEXT("Table %d has no pending order"), TableNumber);
		return false;
	}
    
	return true;
}

FString UTakeOrderCommand::GetErrorMessage() const
{
	if (!ErrorMessage.IsEmpty())
	{
		return ErrorMessage;
	}
    
	return Super::GetErrorMessage();
}

void UTakeOrderCommand::Execute()
{
	if (!CanExecute())
	{
		FailCommand(GetErrorMessage());
		return;
	}
    
	// Find TableManager
	ATableManager* TableManager = nullptr;
	for (TActorIterator<ATableManager> It(OwningRobot->GetWorld()); It; ++It)
	{
		TableManager = *It;
		break;
	}
    
	ATableActor* Table = TableManager->FindTableByNumber(TableNumber);
	IOrderable* Orderable = Cast<IOrderable>(Table);
    
	// Take the order
	FOrderData Order = Orderable->GetCurrentOrder();
	Order.OrderState = EOrderState::Taken;
    
	// Store in robot (access via public member)
	OwningRobot->CurrentOrder = Order;
    
	UE_LOG(LogTemp, Log, TEXT("TakeOrderCommand: Took order from table %d"), TableNumber);
    
	// Complete immediately (no async operation)
	CompleteCommand();
}

FString UTakeOrderCommand::GetDisplayName() const
{
	return FString::Printf(TEXT("Take Order from Table %d"), TableNumber);
}
