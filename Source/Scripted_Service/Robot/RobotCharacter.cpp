// RobotCharacter.cpp

#include "RobotCharacter.h"
#include "RobotAIController.h"
#include "Commands/MoveCommand.h"
#include "Commands/TakeOrderCommand.h"
#include "Commands/PickupCommand.h"
#include "Commands/DeliverCommand.h"
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
    UE_LOG(LogTemp, Warning, TEXT("=== EXECUTE PROGRAM CALLED ==="));
    
    if (CurrentProgram.Num() == 0)
    {
        UE_LOG(LogTemp, Error, TEXT("Robot: No program to execute"));
        return;
    }
    
    if (bIsExecuting && !bIsPaused)
    {
        UE_LOG(LogTemp, Error, TEXT("Robot: Already executing program"));
        return;
    }
    
    // Reset state if starting fresh
    if (!bIsPaused)
    {
        CurrentInstructionIndex = 0;
        CurrentCommandIndex = 0;
        CurrentOrder = FOrderData();
        CarryingDish = nullptr;
        CommandQueue.Empty();
        
        UE_LOG(LogTemp, Warning, TEXT("Creating commands from %d instructions"), CurrentProgram.Num());
        
        // Convert ALL instructions to commands upfront
        for (int32 i = 0; i < CurrentProgram.Num(); i++)
        {
            const FRobotInstruction& Instruction = CurrentProgram[i];
            UE_LOG(LogTemp, Warning, TEXT("  Instruction %d: Type %d"), i, (int32)Instruction.InstructionType);
            
            URobotCommand* Command = CreateCommandFromInstruction(Instruction);
            if (Command)
            {
                UE_LOG(LogTemp, Warning, TEXT("    -> Created command: %s"), *Command->GetDisplayName());
                CommandQueue.Add(Command);
            }
            else
            {
                UE_LOG(LogTemp, Error, TEXT("    -> FAILED to create command!"));
                return;
            }
        }
        
        UE_LOG(LogTemp, Warning, TEXT("Successfully converted %d instructions to %d commands"), 
            CurrentProgram.Num(), CommandQueue.Num());
    }
    
    bIsExecuting = true;
    bIsPaused = false;
    
    UE_LOG(LogTemp, Warning, TEXT("Starting command execution"));
    ExecuteNextCommand();
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
    ExecuteNextCommand();
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
void ARobotCharacter::OnMovementComplete()
{
    UE_LOG(LogTemp, Log, TEXT("Robot: Movement complete"));
}

void ARobotCharacter::LoadDemoProgram()
{
    CurrentProgram.Empty();
    
    // Demo: Take order from Table 1, pick up food, deliver
    FRobotInstruction MoveToTable1;
    MoveToTable1.InstructionType = EInstructionType::MoveToTable;
    MoveToTable1.TargetTableNumber = 1;
    CurrentProgram.Add(MoveToTable1);
    
    FRobotInstruction TakeOrder;
    TakeOrder.InstructionType = EInstructionType::TakeOrder;
    TakeOrder.TargetTableNumber = 1;
    CurrentProgram.Add(TakeOrder);
    
    FRobotInstruction MoveToKitchen;
    MoveToKitchen.InstructionType = EInstructionType::MoveToKitchen;
    CurrentProgram.Add(MoveToKitchen);
    
    FRobotInstruction PickupFood;
    PickupFood.InstructionType = EInstructionType::PickupFood;
    CurrentProgram.Add(PickupFood);
    
    FRobotInstruction MoveBackToTable;
    MoveBackToTable.InstructionType = EInstructionType::MoveToTable;
    MoveBackToTable.TargetTableNumber = 1;
    CurrentProgram.Add(MoveBackToTable);
    
    FRobotInstruction Deliver;
    Deliver.InstructionType = EInstructionType::DeliverOrder;
    CurrentProgram.Add(Deliver);
    
    UE_LOG(LogTemp, Log, TEXT("Demo program loaded: %d instructions"), CurrentProgram.Num());
    
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, 
            FString::Printf(TEXT("Demo Program Loaded! (%d instructions)"), CurrentProgram.Num()));
    }
}

void ARobotCharacter::ClearProgram()
{
    CurrentProgram.Empty();
    CurrentInstructionIndex = 0;
    bIsExecuting = false;
    
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Yellow, TEXT("Program Cleared"));
    }
}


URobotCommand* ARobotCharacter::CreateCommandFromInstruction(const FRobotInstruction& Instruction)
{
    URobotCommand* Command = nullptr;
    
    switch (Instruction.InstructionType)
    {
        case EInstructionType::MoveToTable:
        {
            UMoveCommand* MoveCmd = NewObject<UMoveCommand>(this);
            MoveCmd->InitializeMoveToTable(this, Instruction.TargetTableNumber);
            Command = MoveCmd;
            break;
        }
        
        case EInstructionType::MoveToKitchen:
        {
            UMoveCommand* MoveCmd = NewObject<UMoveCommand>(this);
            MoveCmd->InitializeMoveToKitchen(this);
            Command = MoveCmd;
            break;
        }
        
        case EInstructionType::TakeOrder:
        {
            UTakeOrderCommand* TakeCmd = NewObject<UTakeOrderCommand>(this);
            TakeCmd->InitializeTakeOrder(this, Instruction.TargetTableNumber);
            Command = TakeCmd;
            break;
        }

        case EInstructionType::PickupFood:
        {
            UPickupCommand* PickupCmd = NewObject<UPickupCommand>(this);
            PickupCmd->InitializePickup(this);
            Command = PickupCmd;
            break;
        }

        case EInstructionType::DeliverOrder:
        {
            UDeliverCommand* DeliverCmd = NewObject<UDeliverCommand>(this);
            DeliverCmd->InitializeDeliver(this);
            Command = DeliverCmd;
            break;
        }
        
        case EInstructionType::Wait:
        {
            // TODO: Create WaitCommand class (for now, use old way)
            UE_LOG(LogTemp, Warning, TEXT("Wait command not implemented yet"));
            break;
        }
    }
    
    if (Command)
    {
        // Bind callbacks
        Command->OnComplete.BindUObject(this, &ARobotCharacter::OnCommandComplete);
        Command->OnError.BindUObject(this, &ARobotCharacter::OnCommandError);
    }
    
    return Command;
}

void ARobotCharacter::ExecuteNextCommand()
{
        UE_LOG(LogTemp, Warning, TEXT("=== EXECUTE NEXT COMMAND ==="));
        UE_LOG(LogTemp, Warning, TEXT("Paused: %s"), bIsPaused ? TEXT("YES") : TEXT("NO"));
        UE_LOG(LogTemp, Warning, TEXT("Command Index: %d / %d"), CurrentCommandIndex, CommandQueue.Num());
    
        if (bIsPaused)
        {
            UE_LOG(LogTemp, Log, TEXT("Robot: Execution paused"));
            return;
        }
    
        // Check if we've finished all commands
        if (CurrentCommandIndex >= CommandQueue.Num())
        {
            UE_LOG(LogTemp, Warning, TEXT("=== ALL COMMANDS COMPLETE ==="));
            bIsExecuting = false;
        
            if (GEngine)
            {
                GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, 
                    TEXT("Program Complete!"));
            }
            return;
        }
    
        URobotCommand* Command = CommandQueue[CurrentCommandIndex];
    
        UE_LOG(LogTemp, Warning, TEXT("Executing command: %s"), *Command->GetDisplayName());
    
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Yellow, 
                FString::Printf(TEXT("Executing: %s"), *Command->GetDisplayName()));
        }
    
        // Validate before executing
        UE_LOG(LogTemp, Warning, TEXT("Validating command..."));
        if (!Command->CanExecute())
        {
            FString Error = Command->GetErrorMessage();
            UE_LOG(LogTemp, Error, TEXT("VALIDATION FAILED: %s"), *Error);
            OnCommandError(Error);
            return;
        }
    
        UE_LOG(LogTemp, Warning, TEXT("Validation passed, executing..."));
    
        // Execute the command
        Command->Execute();
    
        UE_LOG(LogTemp, Warning, TEXT("Execute() called, waiting for callback..."));
    }

void ARobotCharacter::OnCommandComplete()
{
    UE_LOG(LogTemp, Log, TEXT("Robot: Command completed successfully"));
    
    // Move to next command
    CurrentCommandIndex++;
    CurrentInstructionIndex = CurrentCommandIndex;
    
    ExecuteNextCommand();
}

void ARobotCharacter::OnCommandError(FString ErrorMessage)
{
    UE_LOG(LogTemp, Error, TEXT("Robot: Command failed - %s"), *ErrorMessage);
    
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, 
            FString::Printf(TEXT("ERROR: %s"), *ErrorMessage));
    }
    
    // Stop execution
    bIsExecuting = false;
}