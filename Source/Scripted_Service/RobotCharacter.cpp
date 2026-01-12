// Fill out your copyright notice in the Description page of Project Settings.


#include "RobotCharacter.h"
#include "EngineUtils.h"

#include <string>

// Sets default values
ARobotCharacter::ARobotCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void ARobotCharacter::BeginPlay()
{
	Super::BeginPlay();
	RobotController = Cast<ARobotAIController>(GetController());
}

bool ARobotCharacter::IsNearActor(AActor* TargetActor, float Distance)
{
	if (!TargetActor) return false;
    
	float Dist = FVector::Dist(GetActorLocation(), TargetActor->GetActorLocation());
	return Dist <= Distance;
}

// Called every frame
void ARobotCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void ARobotCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

void ARobotCharacter::TakeOrderFromTable(int32 TableNumber)
{
	if (!RobotController)
    {
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Red, TEXT("No AI Controller!"));
        }
        ExecuteNextInstruction();
        return;
    }
    
    // Get table manager
    ATableManager* TableMgr = nullptr;
    for (TActorIterator<ATableManager> It(GetWorld()); It; ++It)
    {
        TableMgr = *It;
        break;
    }
    
    if (!TableMgr)
    {
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Red, TEXT("No Table Manager!"));
        }
        ExecuteNextInstruction();
        return;
    }
    
    // Find the table
    ATableActor* Table = TableMgr->FindTableByNumber(TableNumber);
    
    if (!Table)
    {
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Red, 
                FString::Printf(TEXT("Table %d not found!"), TableNumber));
        }
        ExecuteNextInstruction();
        return;
    }
    
    // CHECK IF ROBOT IS NEAR THE TABLE
    if (!IsNearActor(Table, 300.0f))
    {
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Red, 
                TEXT("Not close enough to table! Move there first!"));
        }
        ExecuteNextInstruction();
        return;
    }
    
    // Check if table has order
    if (!Table->HasWaitingOrder())
    {
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Red, 
                FString::Printf(TEXT("Table %d has no order!"), TableNumber));
        }
        ExecuteNextInstruction();
        return;
    }
    
    // Take the order
    CurrentOrder = Table->GetOrder();
    CurrentOrder.OrderState = EOrderState::Taken;
    
    // Store table number
    if (CurrentOrder.TableNumber == 0)
    {
        CurrentOrder.TableNumber = TableNumber;
    }
    
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Green, 
            FString::Printf(TEXT("Order taken from Table %d: %s"), 
            TableNumber, *CurrentOrder.RequestedDish->GetName()));
    }
    
    // Continue to next instruction
    ExecuteNextInstruction();
}

void ARobotCharacter::PickupDishFromCounter()
{
	if (bIsCarryingDish)
    {
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Red, TEXT("Already carrying a dish!"));
        }
        ExecuteNextInstruction();
        return;
    }
    
    if (!CurrentOrder.RequestedDish)
    {
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Red, TEXT("No order taken yet!"));
        }
        ExecuteNextInstruction();
        return;
    }
    
    // Get table manager to find counter
    ATableManager* TableMgr = nullptr;
    for (TActorIterator<ATableManager> It(GetWorld()); It; ++It)
    {
        TableMgr = *It;
        break;
    }
    
    if (!TableMgr || !TableMgr->GetKitchenCounter())
    {
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Red, TEXT("No Kitchen Counter!"));
        }
        ExecuteNextInstruction();
        return;
    }
    
    AKitchenCounter* Counter = TableMgr->GetKitchenCounter();
    
    // CHECK IF ROBOT IS NEAR THE COUNTER
    if (!IsNearActor(Counter, 300.0f))
    {
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Red, 
                TEXT("Not close enough to counter! Move to kitchen first!"));
        }
        ExecuteNextInstruction();
        return;
    }
    
    // Check if the dish we need is on the counter
    if (!Counter->HasDish(CurrentOrder.RequestedDish))
    {
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Red, 
                TEXT("Required dish not on counter! Player needs to cook it."));
        }
        ExecuteNextInstruction();
        return;
    }
    
    // Pick up the dish
    ABaseIngredient* Dish = Counter->PickupDish(CurrentOrder.RequestedDish);
    
    if (Dish)
    {
        CarryingDish = Dish->GetClass();
        bIsCarryingDish = true;
        
        // Destroy the physical dish
        Dish->Destroy();
        
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Green, 
                FString::Printf(TEXT("Picked up: %s"), *CarryingDish->GetName()));
        }
    }
    
    ExecuteNextInstruction();
}

void ARobotCharacter::DeliverDishToTable()
{
	if (!bIsCarryingDish)
	{
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Red, TEXT("Not carrying any dish!"));
		}
		ExecuteNextInstruction();
		return;
	}
    
	// Get table manager
	ATableManager* TableMgr = nullptr;
	for (TActorIterator<ATableManager> It(GetWorld()); It; ++It)
	{
		TableMgr = *It;
		break;
	}
    
	if (!TableMgr)
	{
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Red, TEXT("No Table Manager!"));
		}
		ExecuteNextInstruction();
		return;
	}
    
	// Find the table from our order
	ATableActor* Table = TableMgr->FindTableByNumber(CurrentOrder.TableNumber);
    
	if (!Table)
	{
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Red, TEXT("Table not found!"));
		}
		ExecuteNextInstruction();
		return;
	}
    
	// CHECK IF ROBOT IS NEAR THE TABLE
	if (!IsNearActor(Table, 300.0f))
	{
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Red, 
				TEXT("Not close enough to table! Move there first!"));
		}
		ExecuteNextInstruction();
		return;
	}
    
	// Deliver the dish
	bool bCorrectDish = Table->DeliverDish(CarryingDish);
    
	// Clear robot's inventory
	CarryingDish = nullptr;
	bIsCarryingDish = false;
	CurrentOrder = FOrderData();
    
	if (bCorrectDish)
	{
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Green, TEXT("Delivery SUCCESS!"));
		}
	}
	else
	{
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Orange, TEXT("Wrong dish delivered - Customer unhappy!"));
		}
	}
    
	ExecuteNextInstruction();
}


void ARobotCharacter::ExecuteProgram()
{
	CurrentInstructionIndex = 0;
	bIsCarryingDish = false;
	CarryingDish = nullptr;
	CurrentOrder = FOrderData();
	
	ExecuteNextInstruction();
}

void ARobotCharacter::ExecuteNextInstruction()
{
	if (CurrentInstructionIndex >= CurrentProgram.Num())
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, "Program Complete"); return;
	}

	FRobotInstruction CurrentInstruction = CurrentProgram[CurrentInstructionIndex];

	CurrentInstructionIndex++;

	switch (CurrentInstruction.InstructionType)
	{
	case EInstructionType::MoveToTable:
		if (RobotController)
		{
			RobotController->MoveToTableLocation(CurrentInstruction.TargetTableNumber);
		}
		else
		{
			GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, "Not Found");
		}
		break;

	case EInstructionType::MoveToKitchen:
		if (RobotController)
		{
			RobotController->MoveToKitchenLocation();
		}
		break;

	case EInstructionType::TakeOrder:
		TakeOrderFromTable(CurrentInstruction.TargetTableNumber);
		break;
            
	case EInstructionType::DeliverOrder:
		DeliverDishToTable();
		break;

	case EInstructionType::Wait:
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, "Waiting");
		ExecuteNextInstruction();
		break;

	case EInstructionType::PickupFood:
		PickupDishFromCounter();
		break;
		
	default:
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, "Unhandled Instruction");
		ExecuteNextInstruction();
		break;
	}
}

