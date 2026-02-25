// RobotCharacter.cpp

#include "RobotCharacter.h"
#include "RobotAIController.h"
#include "TableManager.h"
#include "TableActor.h"
#include "KitchenCounter.h"
#include "IOrderable.h"
#include "IPickupPoint.h"
#include "EngineUtils.h"
#include "TimerManager.h"

ARobotCharacter::ARobotCharacter()
{
    PrimaryActorTick.bCanEverTick = false;
    
    // AI Controller setup
    AIControllerClass = ARobotAIController::StaticClass();
    AutoPossessAI = EAutoPossessAI::PlacedInWorld;
    
    // Initialize state
    CurrentInstructionIndex = 0;
    bIsExecuting = false;
    bIsPaused = false;
    CarryingDish = nullptr;
}

void ARobotCharacter::BeginPlay()
{
    Super::BeginPlay();
    
    // Find TableManager in the level
    for (TActorIterator<ATableManager> It(GetWorld()); It; ++It)
    {
        TableManager = *It;
        break;
    }
    
    if (!TableManager)
    {
        UE_LOG(LogTemp, Error, TEXT("Robot: No TableManager found in level!"));
    }
    else
    {
        UE_LOG(LogTemp, Log, TEXT("Robot: TableManager found successfully"));
    }
}

// ============================================================================
// IProgrammable INTERFACE IMPLEMENTATION
// ============================================================================

void ARobotCharacter::LoadProgram(const TArray<FRobotInstruction>& Instructions)
{
    if (bIsExecuting)
    {
        UE_LOG(LogTemp, Warning, TEXT("Robot: Cannot load program while executing"));
        return;
    }
    
    CurrentProgram = Instructions;
    CurrentInstructionIndex = 0;
    bIsPaused = false;
    
    UE_LOG(LogTemp, Log, TEXT("Robot: Program loaded with %d instructions"), Instructions.Num());
}

void ARobotCharacter::ExecuteProgram()
{
    if (CurrentProgram.Num() == 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("Robot: No program to execute"));
        return;
    }
    
    if (bIsExecuting && !bIsPaused)
    {
        UE_LOG(LogTemp, Warning, TEXT("Robot: Already executing program"));
        return;
    }
    
    // Reset state if starting fresh
    if (!bIsPaused)
    {
        CurrentInstructionIndex = 0;
        CurrentOrder = FOrderData();
        CarryingDish = nullptr;
        
        UE_LOG(LogTemp, Log, TEXT("Robot: Starting fresh program execution"));
    }
    else
    {
        UE_LOG(LogTemp, Log, TEXT("Robot: Resuming program execution"));
    }
    
    bIsExecuting = true;
    bIsPaused = false;
    
    ExecuteNextInstruction();
}

void ARobotCharacter::StopProgram()
{
    if (!bIsExecuting)
    {
        return;
    }
    
    bIsExecuting = false;
    bIsPaused = false;
    CurrentInstructionIndex = 0;
    
    // Clear any active timers
    GetWorldTimerManager().ClearTimer(WaitTimerHandle);
    
    // Stop any movement
    ARobotAIController* AIController = Cast<ARobotAIController>(GetController());
    if (AIController)
    {
        AIController->StopMovement();
    }
    
    UE_LOG(LogTemp, Log, TEXT("Robot: Program stopped"));
}

void ARobotCharacter::PauseProgram()
{
    if (!bIsExecuting)
    {
        UE_LOG(LogTemp, Warning, TEXT("Robot: No program running to pause"));
        return;
    }
    
    if (bIsPaused)
    {
        UE_LOG(LogTemp, Warning, TEXT("Robot: Program already paused"));
        return;
    }
    
    bIsPaused = true;
    
    // Clear any active timers
    GetWorldTimerManager().ClearTimer(WaitTimerHandle);
    
    // Stop any movement
    ARobotAIController* AIController = Cast<ARobotAIController>(GetController());
    if (AIController)
    {
        AIController->StopMovement();
    }
    
    UE_LOG(LogTemp, Log, TEXT("Robot: Program paused at instruction %d"), CurrentInstructionIndex);
}

void ARobotCharacter::ResumeProgram()
{
    if (!bIsExecuting || !bIsPaused)
    {
        UE_LOG(LogTemp, Warning, TEXT("Robot: No paused program to resume"));
        return;
    }
    
    bIsPaused = false;
    UE_LOG(LogTemp, Log, TEXT("Robot: Program resumed from instruction %d"), CurrentInstructionIndex);
    ExecuteNextInstruction();
}

bool ARobotCharacter::IsProgramRunning() const
{
    return bIsExecuting && !bIsPaused;
}

int32 ARobotCharacter::GetCurrentInstructionIndex() const
{
    return CurrentInstructionIndex;
}

int32 ARobotCharacter::GetProgramLength() const
{
    return CurrentProgram.Num();
}

/**
 *INSTRUCTION EXECUTION
 */
void ARobotCharacter::ExecuteNextInstruction()
{
    // Check if program is paused
    if (bIsPaused)
    {
        UE_LOG(LogTemp, Log, TEXT("Robot: Execution paused"));
        return;
    }
    
    // Check if we've finished the program
    if (CurrentInstructionIndex >= CurrentProgram.Num())
    {
        UE_LOG(LogTemp, Log, TEXT("Robot: Program execution complete!"));
        bIsExecuting = false;
        return;
    }
    
    const FRobotInstruction& Instruction = CurrentProgram[CurrentInstructionIndex];
    
    UE_LOG(LogTemp, Log, TEXT("Robot: Executing instruction %d of %d"), 
        CurrentInstructionIndex + 1, 
        CurrentProgram.Num());
    
    switch (Instruction.InstructionType)
    {
        case EInstructionType::MoveToTable:
        {
            if (!TableManager)
            {
                UE_LOG(LogTemp, Error, TEXT("Robot: Cannot move to table - No TableManager!"));
                bIsExecuting = false;
                return;
            }
            
            ATableActor* Table = TableManager->FindTableByNumber(Instruction.TargetTableNumber);
            if (!Table)
            {
                UE_LOG(LogTemp, Error, TEXT("Robot: Table %d not found!"), Instruction.TargetTableNumber);
                bIsExecuting = false;
                return;
            }
            
            // Use interface to get location
            IOrderable* Orderable = Cast<IOrderable>(Table);
            if (Orderable)
            {
                FVector TargetLocation = Orderable->GetInteractionLocation();
                
                ARobotAIController* AIController = Cast<ARobotAIController>(GetController());
                if (AIController)
                {
                    AIController->MoveToLocation(TargetLocation);
                }
            }
            break;
        }
        
        case EInstructionType::MoveToKitchen:
        {
            if (!TableManager)
            {
                UE_LOG(LogTemp, Error, TEXT("Robot: Cannot move to kitchen - No TableManager!"));
                bIsExecuting = false;
                return;
            }
            
            FVector KitchenLocation = TableManager->GetKitchenLocation();
            
            ARobotAIController* AIController = Cast<ARobotAIController>(GetController());
            if (AIController)
            {
                AIController->MoveToLocation(KitchenLocation);
            }
            break;
        }
        
        case EInstructionType::TakeOrder:
        {
            TakeOrderFromTable(Instruction.TargetTableNumber);
            break;
        }
        
        case EInstructionType::PickupFood:
        {
            PickupDishFromCounter();
            break;
        }
        
        case EInstructionType::DeliverOrder:
        {
            DeliverDishToTable();
            break;
        }
        
        case EInstructionType::Wait:
        {
            WaitForDuration(Instruction.WaitValue);
            break;
        }
        
        default:
        {
            UE_LOG(LogTemp, Warning, TEXT("Robot: Unknown instruction type"));
            CurrentInstructionIndex++;
            ExecuteNextInstruction();
            break;
        }
    }
}

void ARobotCharacter::OnMovementComplete()
{
    UE_LOG(LogTemp, Log, TEXT("Robot: Movement complete"));
    
    // Move to next instruction
    CurrentInstructionIndex++;
    ExecuteNextInstruction();
}

// ============================================================================
// ACTION FUNCTIONS
// ============================================================================

void ARobotCharacter::TakeOrderFromTable(int32 TableNumber)
{
    if (!TableManager)
    {
        UE_LOG(LogTemp, Error, TEXT("Robot: Cannot take order - No TableManager!"));
        return;
    }
    
    // Find the table
    ATableActor* Table = TableManager->FindTableByNumber(TableNumber);
    if (!Table)
    {
        UE_LOG(LogTemp, Error, TEXT("Robot: Table %d not found!"), TableNumber);
        CurrentInstructionIndex++;
        ExecuteNextInstruction();
        return;
    }
    
    // Cast to interface
    IOrderable* Orderable = Cast<IOrderable>(Table);
    if (!Orderable)
    {
        UE_LOG(LogTemp, Error, TEXT("Robot: Table does not implement IOrderable!"));
        CurrentInstructionIndex++;
        ExecuteNextInstruction();
        return;
    }
    
    // Check if robot is close enough to table
    FVector TableLocation = Orderable->GetInteractionLocation();
    float Distance = FVector::Dist(GetActorLocation(), TableLocation);
    
    if (Distance > 300.0f)
    {
        UE_LOG(LogTemp, Warning, TEXT("Robot: Too far from table %d (Distance: %.2f)"), 
            TableNumber, Distance);
        CurrentInstructionIndex++;
        ExecuteNextInstruction();
        return;
    }
    
    // Check if table has an order
    if (!Orderable->HasPendingOrder())
    {
        UE_LOG(LogTemp, Warning, TEXT("Robot: Table %d has no pending order"), TableNumber);
        CurrentInstructionIndex++;
        ExecuteNextInstruction();
        return;
    }
    
    // Take the order
    CurrentOrder = Orderable->GetCurrentOrder();
    CurrentOrder.OrderState = EOrderState::Taken;
    
    UE_LOG(LogTemp, Log, TEXT("Robot: Took order from table %d"), TableNumber);
    
    // Continue to next instruction
    CurrentInstructionIndex++;
    ExecuteNextInstruction();
}

void ARobotCharacter::PickupDishFromCounter()
{
    if (!TableManager)
    {
        UE_LOG(LogTemp, Error, TEXT("Robot: Cannot pickup dish - No TableManager!"));
        return;
    }
    
    AKitchenCounter* Counter = TableManager->GetKitchenCounter();
    if (!Counter)
    {
        UE_LOG(LogTemp, Error, TEXT("Robot: No kitchen counter found!"));
        CurrentInstructionIndex++;
        ExecuteNextInstruction();
        return;
    }
    
    // Cast to interface
    IPickupPoint* PickupPoint = Cast<IPickupPoint>(Counter);
    if (!PickupPoint)
    {
        UE_LOG(LogTemp, Error, TEXT("Robot: Counter does not implement IPickupPoint!"));
        CurrentInstructionIndex++;
        ExecuteNextInstruction();
        return;
    }
    
    // Check if robot is close enough to counter
    FVector CounterLocation = PickupPoint->GetPickupLocation();
    float Distance = FVector::Dist(GetActorLocation(), CounterLocation);
    
    if (Distance > 300.0f)
    {
        UE_LOG(LogTemp, Warning, TEXT("Robot: Too far from counter (Distance: %.2f)"), Distance);
        CurrentInstructionIndex++;
        ExecuteNextInstruction();
        return;
    }
    
    // Check if we have an order to fulfill
    if (!CurrentOrder.RequestedDish)
    {
        UE_LOG(LogTemp, Warning, TEXT("Robot: No order to pickup dish for"));
        CurrentInstructionIndex++;
        ExecuteNextInstruction();
        return;
    }
    
    // Check if the required dish is available
    if (!PickupPoint->HasItem(CurrentOrder.RequestedDish))
    {
        UE_LOG(LogTemp, Warning, TEXT("Robot: Required dish not available on counter"));
        CurrentInstructionIndex++;
        ExecuteNextInstruction();
        return;
    }
    
    // Pickup the dish
    if (PickupPoint->PickupItem(CurrentOrder.RequestedDish))
    {
        CarryingDish = CurrentOrder.RequestedDish;
        CurrentOrder.OrderState = EOrderState::Ready;
        
        UE_LOG(LogTemp, Log, TEXT("Robot: Picked up dish from counter"));
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Robot: Failed to pickup dish"));
    }
    
    // Continue to next instruction
    CurrentInstructionIndex++;
    ExecuteNextInstruction();
}

void ARobotCharacter::DeliverDishToTable()
{
    if (!TableManager)
    {
        UE_LOG(LogTemp, Error, TEXT("Robot: Cannot deliver dish - No TableManager!"));
        return;
    }
    
    // Check if robot is carrying a dish
    if (!CarryingDish)
    {
        UE_LOG(LogTemp, Warning, TEXT("Robot: Not carrying any dish to deliver"));
        CurrentInstructionIndex++;
        ExecuteNextInstruction();
        return;
    }
    
    // Find the table for this order
    ATableActor* Table = TableManager->FindTableByNumber(CurrentOrder.TableNumber);
    if (!Table)
    {
        UE_LOG(LogTemp, Error, TEXT("Robot: Table %d not found!"), CurrentOrder.TableNumber);
        CurrentInstructionIndex++;
        ExecuteNextInstruction();
        return;
    }
    
    // Cast to interface
    IOrderable* Orderable = Cast<IOrderable>(Table);
    if (!Orderable)
    {
        UE_LOG(LogTemp, Error, TEXT("Robot: Table does not implement IOrderable!"));
        CurrentInstructionIndex++;
        ExecuteNextInstruction();
        return;
    }
    
    // Check if robot is close enough to table
    FVector TableLocation = Orderable->GetInteractionLocation();
    float Distance = FVector::Dist(GetActorLocation(), TableLocation);
    
    if (Distance > 300.0f)
    {
        UE_LOG(LogTemp, Warning, TEXT("Robot: Too far from table %d (Distance: %.2f)"), 
            CurrentOrder.TableNumber, Distance);
        CurrentInstructionIndex++;
        ExecuteNextInstruction();
        return;
    }
    
    // Attempt delivery using interface
    bool bDeliverySuccessful = Orderable->DeliverOrder(CarryingDish);
    
    if (bDeliverySuccessful)
    {
        UE_LOG(LogTemp, Log, TEXT("Robot: Successfully delivered correct dish to table %d"), 
            CurrentOrder.TableNumber);
        
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Green, 
                FString::Printf(TEXT("Correct dish delivered to table %d!"), CurrentOrder.TableNumber));
        }
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Robot: Delivered wrong dish to table %d"), 
            CurrentOrder.TableNumber);
        
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Red, 
                FString::Printf(TEXT("Wrong dish delivered to table %d!"), CurrentOrder.TableNumber));
        }
    }
    
    // Clear robot inventory
    CarryingDish = nullptr;
    CurrentOrder = FOrderData();
    
    // Continue to next instruction
    CurrentInstructionIndex++;
    ExecuteNextInstruction();
}

void ARobotCharacter::WaitForDuration(float Seconds)
{
    if (Seconds <= 0.0f)
    {
        UE_LOG(LogTemp, Warning, TEXT("Robot: Invalid wait duration: %.2f"), Seconds);
        CurrentInstructionIndex++;
        ExecuteNextInstruction();
        return;
    }
    
    UE_LOG(LogTemp, Log, TEXT("Robot: Waiting for %.2f seconds"), Seconds);
    
    // Set timer to continue after wait
    FTimerDelegate TimerDelegate;
    TimerDelegate.BindLambda([this]()
    {
        UE_LOG(LogTemp, Log, TEXT("Robot: Wait complete"));
        CurrentInstructionIndex++;
        ExecuteNextInstruction();
    });
    
    GetWorldTimerManager().SetTimer(WaitTimerHandle, TimerDelegate, Seconds, false);
}