// Fill out your copyright notice in the Description page of Project Settings.

#include "RobotCharacter.h"

#include "Commands/MoveCommand.h"
#include "Commands/TakeOrderCommand.h"
#include "Commands/PickupCommand.h"
#include "Commands/DeliverCommand.h"
#include "Commands/WaitCommand.h"

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

    // Spawn with an AI controller so navigation works without extra Blueprint setup
    AIControllerClass  = ARobotAIController::StaticClass();
    AutoPossessAI      = EAutoPossessAI::PlacedInWorld;

    // Initialise execution state
    InstructionPointer = 0;
    bIsExecuting       = false;
    bIsPaused          = false;
    CarryingDish       = nullptr;
    CurrentCommand     = nullptr;

    TableManager       = nullptr;
}

void ARobotCharacter::BeginPlay()
{
    Super::BeginPlay();

    // Cache the TableManager so every command doesn't need to search the world
    for (TActorIterator<ATableManager> It(GetWorld()); It; ++It)
    {
        TableManager = *It;
        break;
    }

    if (!TableManager)
    {
        UE_LOG(LogTemp, Error,
            TEXT("ARobotCharacter '%s': No ATableManager found in level! "
                 "Place one in the level before running."),
            *GetName());
    }
    else
    {
        UE_LOG(LogTemp, Log,
            TEXT("ARobotCharacter '%s': TableManager found — '%s'"),
            *GetName(), *TableManager->GetName());
    }
}

void ARobotCharacter::LoadProgram(const TArray<FRobotInstruction>& Instructions)
{
    if (bIsExecuting && !bIsPaused)
    {
        UE_LOG(LogTemp, Warning,
            TEXT("ARobotCharacter: Cannot load a new program while one is running. "
                 "Call StopProgram() first."));
        return;
    }

    CurrentProgram     = Instructions;
    InstructionPointer = 0;
    bIsPaused          = false;

    UE_LOG(LogTemp, Log,
        TEXT("ARobotCharacter: Program loaded — %d instruction(s)"),
        Instructions.Num());
}

void ARobotCharacter::ExecuteProgram()
{
    UE_LOG(LogTemp, Warning, TEXT("=== ARobotCharacter::ExecuteProgram ==="));

    if (CurrentProgram.Num() == 0)
    {
        UE_LOG(LogTemp, Error,
            TEXT("ARobotCharacter: ExecuteProgram called but no program is loaded."));
        return;
    }

    if (bIsExecuting && !bIsPaused)
    {
        UE_LOG(LogTemp, Warning,
            TEXT("ARobotCharacter: ExecuteProgram called while already running."));
        return;
    }

    if (!bIsPaused)
    {
        // Fresh start — reset ALL interpreter state so a re-run is clean
        InstructionPointer = 0;
        ExecutionStack.Empty();
        CurrentOrder       = FOrderData();
        CarryingDish       = nullptr;
        CurrentCommand     = nullptr;
    }

    bIsExecuting = true;
    bIsPaused    = false;

    UE_LOG(LogTemp, Log,
        TEXT("ARobotCharacter: Executing program — %d instruction(s), starting at IP=%d"),
        CurrentProgram.Num(), InstructionPointer);

    ExecuteCurrentInstruction();
}

void ARobotCharacter::StopProgram()
{
    if (!bIsExecuting)
    {
        return;
    }

    // Cancel any async command that may be in flight
    if (CurrentCommand)
    {
        CurrentCommand->Cancel();
        CurrentCommand = nullptr;
    }

    // Stop AI navigation
    if (ARobotAIController* AI = Cast<ARobotAIController>(GetController()))
    {
        AI->StopMovement();
    }

    // Clear any pending timer (e.g. from a Wait command)
    GetWorldTimerManager().ClearTimer(WaitTimerHandle);

    bIsExecuting       = false;
    bIsPaused          = false;
    InstructionPointer = 0;
    ExecutionStack.Empty();

    UE_LOG(LogTemp, Log, TEXT("ARobotCharacter: Program stopped."));
}

void ARobotCharacter::PauseProgram()
{
    if (!bIsExecuting)
    {
        UE_LOG(LogTemp, Warning,
            TEXT("ARobotCharacter: PauseProgram called but nothing is running."));
        return;
    }

    if (bIsPaused)
    {
        UE_LOG(LogTemp, Warning, TEXT("ARobotCharacter: Program is already paused."));
        return;
    }

    // Cancel the current async command WITHOUT advancing the IP.
    // When ResumeProgram() is called it will re-execute the same instruction.
    if (CurrentCommand)
    {
        CurrentCommand->Cancel();
        CurrentCommand = nullptr;
    }

    if (ARobotAIController* AI = Cast<ARobotAIController>(GetController()))
    {
        AI->StopMovement();
    }

    GetWorldTimerManager().ClearTimer(WaitTimerHandle);

    bIsPaused = true;

    UE_LOG(LogTemp, Log,
        TEXT("ARobotCharacter: Program paused at IP=%d."), InstructionPointer);
}

void ARobotCharacter::ResumeProgram()
{
    if (!bIsExecuting || !bIsPaused)
    {
        UE_LOG(LogTemp, Warning,
            TEXT("ARobotCharacter: ResumeProgram called but there is no paused program."));
        return;
    }

    bIsPaused      = false;
    CurrentCommand = nullptr;

    UE_LOG(LogTemp, Log,
        TEXT("ARobotCharacter: Resuming from IP=%d."), InstructionPointer);

    ExecuteCurrentInstruction();
}

bool ARobotCharacter::IsProgramRunning() const
{
    return bIsExecuting && !bIsPaused;
}

int32 ARobotCharacter::GetCurrentInstructionIndex() const
{
    return InstructionPointer;
}

int32 ARobotCharacter::GetProgramLength() const
{
    return CurrentProgram.Num();
}

void ARobotCharacter::ExecuteCurrentInstruction()
{
    // Guard: do nothing while paused
    if (bIsPaused)
    {
        return;
    }

    if (InstructionPointer >= CurrentProgram.Num())
    {
        bIsExecuting = false;

        UE_LOG(LogTemp, Warning, TEXT("=== PROGRAM COMPLETE ==="));

        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(
                -1, 5.0f, FColor::Green, TEXT("✔  Program Complete!"));
        }
        return;
    }

    const FRobotInstruction& Instr = CurrentProgram[InstructionPointer];

    UE_LOG(LogTemp, Verbose,
        TEXT("ARobotCharacter: IP=%d  InstructionType=%d"),
        InstructionPointer, static_cast<int32>(Instr.InstructionType));

    switch (Instr.InstructionType)
    {

    case EInstructionType::IfTableHasOrder:
    {
        bool bCondition = false;

        if (TableManager)
        {
            if (ATableActor* Table = TableManager->FindTableByNumber(Instr.TargetTableNumber))
            {
                if (IOrderable* Ord = Cast<IOrderable>(Table))
                {
                    bCondition = Ord->HasPendingOrder();
                }
            }
        }
        else
        {
            UE_LOG(LogTemp, Warning,
                TEXT("ARobotCharacter: IfTableHasOrder — no TableManager, condition is FALSE."));
        }

        UE_LOG(LogTemp, Log,
            TEXT("ARobotCharacter: IfTableHasOrder(Table %d) → %s"),
            Instr.TargetTableNumber, bCondition ? TEXT("TRUE") : TEXT("FALSE"));

        if (bCondition)
        {
            // Step into the body
            InstructionPointer++;
        }
        else
        {
            // Skip to the instruction after the matching EndBlock
            InstructionPointer = FindMatchingEndBlock(InstructionPointer) + 1;
        }

        ExecuteCurrentInstruction();
        return;
    }
        
    case EInstructionType::IfCarryingDish:
    {
        bool bCarrying = (CarryingDish != nullptr);

        UE_LOG(LogTemp, Log,
            TEXT("ARobotCharacter: IfCarryingDish → %s"),
            bCarrying ? TEXT("TRUE") : TEXT("FALSE"));

        if (bCarrying) InstructionPointer++;
        else           InstructionPointer = FindMatchingEndBlock(InstructionPointer) + 1;

        ExecuteCurrentInstruction();
        return;
    }

    case EInstructionType::IfNotCarryingDish:
    {
        bool bEmpty = (CarryingDish == nullptr);

        UE_LOG(LogTemp, Log,
            TEXT("ARobotCharacter: IfNotCarryingDish → %s"),
            bEmpty ? TEXT("TRUE") : TEXT("FALSE"));

        if (bEmpty) InstructionPointer++;
        else        InstructionPointer = FindMatchingEndBlock(InstructionPointer) + 1;

        ExecuteCurrentInstruction();
        return;
    }

    case EInstructionType::RepeatLoop:
    {
        // Clamp to at least 1 so the body always runs at least once
        const int32 Count = FMath::Max(1, Instr.LoopCount);

        UE_LOG(LogTemp, Log,
            TEXT("ARobotCharacter: RepeatLoop ×%d — pushing frame"), Count);

        FExecFrame Frame;
        Frame.LoopStartIndex      = InstructionPointer; // Points at RepeatLoop itself
        Frame.RemainingIterations = Count;
        ExecutionStack.Push(Frame);

        InstructionPointer++; // Step into body
        ExecuteCurrentInstruction();
        return;
    }

    case EInstructionType::LoopForever:
    {
        UE_LOG(LogTemp, Log,
            TEXT("ARobotCharacter: LoopForever — pushing frame"));

        FExecFrame Frame;
        Frame.LoopStartIndex      = InstructionPointer;
        Frame.RemainingIterations = -1; // Sentinel: run forever
        ExecutionStack.Push(Frame);

        InstructionPointer++;
        ExecuteCurrentInstruction();
        return;
    }

    case EInstructionType::EndBlock:
    {
        if (ExecutionStack.Num() > 0)
        {
            FExecFrame& Frame = ExecutionStack.Top();
            const bool bForever = (Frame.RemainingIterations == -1);

            if (bForever)
            {
                // Jump back to first instruction inside the loop body (IP of
                // LoopForever is Frame.LoopStartIndex, so body starts at +1)
                InstructionPointer = Frame.LoopStartIndex + 1;

                UE_LOG(LogTemp, Verbose,
                    TEXT("ARobotCharacter: EndBlock — looping forever, jumping to IP=%d"),
                    InstructionPointer);
            }
            else
            {
                Frame.RemainingIterations--;

                if (Frame.RemainingIterations > 0)
                {
                    // More iterations remain — jump back to body start
                    InstructionPointer = Frame.LoopStartIndex + 1;

                    UE_LOG(LogTemp, Verbose,
                        TEXT("ARobotCharacter: EndBlock — %d iteration(s) left, "
                             "jumping to IP=%d"),
                        Frame.RemainingIterations, InstructionPointer);
                }
                else
                {
                    // All iterations done — pop the frame and continue
                    ExecutionStack.Pop();
                    InstructionPointer++;

                    UE_LOG(LogTemp, Log,
                        TEXT("ARobotCharacter: EndBlock — loop complete, "
                             "advancing to IP=%d"),
                        InstructionPointer);
                }
            }
        }
        else
        {
            // Unmatched EndBlock (malformed program) — skip it and keep going
            UE_LOG(LogTemp, Warning,
                TEXT("ARobotCharacter: Unmatched EndBlock at IP=%d — skipping."),
                InstructionPointer);
            InstructionPointer++;
        }

        ExecuteCurrentInstruction();
        return;
    }

    default:
        break; // Fall through to action command dispatch below
    }


    URobotCommand* Command = CreateCommandFromInstruction(Instr);

    if (!Command)
    {
        // CreateCommandFromInstruction returns nullptr for unknown types
        UE_LOG(LogTemp, Error,
            TEXT("ARobotCharacter: No command handler for InstructionType=%d at IP=%d. "
                 "Skipping."),
            static_cast<int32>(Instr.InstructionType), InstructionPointer);

        // Skip unknown instruction rather than halting entirely, to be robust
        InstructionPointer++;
        ExecuteCurrentInstruction();
        return;
    }

    // Pre-flight validation
    if (!Command->CanExecute())
    {
        OnCommandError(Command->GetErrorMessage());
        return;
    }

    // Store reference so PauseProgram / StopProgram can cancel it
    CurrentCommand = Command;

    // Bind delegate callbacks
    Command->OnComplete.BindUObject(this, &ARobotCharacter::OnCommandComplete);
    Command->OnError.BindUObject(this, &ARobotCharacter::OnCommandError);

    UE_LOG(LogTemp, Log,
        TEXT("ARobotCharacter: Executing '%s' at IP=%d"),
        *Command->GetDisplayName(), InstructionPointer);

    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Yellow,
            FString::Printf(TEXT("▶  %s"), *Command->GetDisplayName()));
    }

    Command->Execute();
    // ── async gap — next step driven by OnCommandComplete / OnCommandError ────
}

void ARobotCharacter::OnCommandComplete()
{
    CurrentCommand = nullptr;

    // Advance past the completed action instruction
    InstructionPointer++;

    UE_LOG(LogTemp, Log,
        TEXT("ARobotCharacter: Command complete — advancing to IP=%d"),
        InstructionPointer);

    ExecuteCurrentInstruction();
}

void ARobotCharacter::OnCommandError(FString ErrorMessage)
{
    UE_LOG(LogTemp, Error,
        TEXT("ARobotCharacter: Command failed at IP=%d — %s"),
        InstructionPointer, *ErrorMessage);

    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red,
            FString::Printf(TEXT("❌  ERROR: %s"), *ErrorMessage));
    }

    CurrentCommand = nullptr;
    bIsExecuting   = false;
}

int32 ARobotCharacter::FindMatchingEndBlock(int32 StartIndex) const
{
    // Walk forward from StartIndex + 1.
    // Keep a depth counter: increment for each block-opening instruction,
    // decrement for each EndBlock.  The first EndBlock at depth 0 is the match.
    int32 Depth = 0;

    for (int32 i = StartIndex + 1; i < CurrentProgram.Num(); i++)
    {
        const EInstructionType Type = CurrentProgram[i].InstructionType;

        const bool bOpener =
            Type == EInstructionType::IfTableHasOrder   ||
            Type == EInstructionType::IfCarryingDish    ||
            Type == EInstructionType::IfNotCarryingDish ||
            Type == EInstructionType::RepeatLoop        ||
            Type == EInstructionType::LoopForever;

        if (bOpener)
        {
            Depth++;
        }
        else if (Type == EInstructionType::EndBlock)
        {
            if (Depth == 0)
            {
                // Found the matching EndBlock
                return i;
            }
            Depth--;
        }
    }

    // Malformed program — no matching EndBlock exists.
    // Return the last valid index so the interpreter jumps to the end of
    // the program rather than going out of bounds.
    UE_LOG(LogTemp, Warning,
        TEXT("ARobotCharacter: FindMatchingEndBlock — no EndBlock found for "
             "block at IP=%d. Program may be malformed."),
        StartIndex);

    return CurrentProgram.Num() - 1;
}

URobotCommand* ARobotCharacter::CreateCommandFromInstruction(
    const FRobotInstruction& Instruction)
{
    switch (Instruction.InstructionType)
    {

    case EInstructionType::MoveToTable:
    {
        UMoveCommand* Cmd = NewObject<UMoveCommand>(this);
        Cmd->InitializeMoveToTable(this, Instruction.TargetTableNumber);
        return Cmd;
    }

    case EInstructionType::MoveToKitchen:
    {
        UMoveCommand* Cmd = NewObject<UMoveCommand>(this);
        Cmd->InitializeMoveToKitchen(this);
        return Cmd;
    }

    case EInstructionType::TakeOrder:
    {
        UTakeOrderCommand* Cmd = NewObject<UTakeOrderCommand>(this);
        Cmd->InitializeTakeOrder(this, Instruction.TargetTableNumber);
        return Cmd;
    }

    case EInstructionType::PickupFood:
    {
        UPickupCommand* Cmd = NewObject<UPickupCommand>(this);
        Cmd->InitializePickup(this);
        return Cmd;
    }

    case EInstructionType::DeliverOrder:
    {
        UDeliverCommand* Cmd = NewObject<UDeliverCommand>(this);
        Cmd->InitializeDeliver(this);
        return Cmd;
    }

    case EInstructionType::Wait:
    {
        UWaitCommand* Cmd = NewObject<UWaitCommand>(this);
        Cmd->InitializeWait(this, Instruction.WaitValue);
        return Cmd;
    }

    // Control-flow instructions are NOT converted to commands —
    // they are handled directly in ExecuteCurrentInstruction().
    case EInstructionType::IfTableHasOrder:
    case EInstructionType::IfCarryingDish:
    case EInstructionType::IfNotCarryingDish:
    case EInstructionType::RepeatLoop:
    case EInstructionType::LoopForever:
    case EInstructionType::EndBlock:
        return nullptr;

    default:
        UE_LOG(LogTemp, Warning,
            TEXT("ARobotCharacter: CreateCommandFromInstruction — "
                 "unhandled InstructionType=%d"),
            static_cast<int32>(Instruction.InstructionType));
        return nullptr;
    }
}

void ARobotCharacter::LoadDemoProgram()
{

    CurrentProgram.Empty();

    auto Make = [](EInstructionType Type, int32 Table = 0,
                   float Wait = 0.0f, int32 Loop = 1) -> FRobotInstruction
    {
        FRobotInstruction I;
        I.InstructionType  = Type;
        I.TargetTableNumber = Table;
        I.WaitValue        = Wait;
        I.LoopCount        = Loop;
        return I;
    };

    // Outer loop — runs forever
    CurrentProgram.Add(Make(EInstructionType::LoopForever));

        // Inner condition — only serve if Table 1 has an order
        CurrentProgram.Add(Make(EInstructionType::IfTableHasOrder, /*Table=*/1));

            CurrentProgram.Add(Make(EInstructionType::MoveToTable,   /*Table=*/1));
            CurrentProgram.Add(Make(EInstructionType::TakeOrder,     /*Table=*/1));
            CurrentProgram.Add(Make(EInstructionType::MoveToKitchen));
            CurrentProgram.Add(Make(EInstructionType::PickupFood));
            CurrentProgram.Add(Make(EInstructionType::MoveToTable,   /*Table=*/1));
            CurrentProgram.Add(Make(EInstructionType::DeliverOrder));

        CurrentProgram.Add(Make(EInstructionType::EndBlock)); // closes IfTableHasOrder

        // Pause between checks so the robot doesn't busy-loop
        CurrentProgram.Add(Make(EInstructionType::Wait, 0, /*Wait=*/2.0f));

    CurrentProgram.Add(Make(EInstructionType::EndBlock)); // closes LoopForever

    UE_LOG(LogTemp, Log,
        TEXT("ARobotCharacter: Demo program loaded — %d instruction(s)"),
        CurrentProgram.Num());

    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green,
            FString::Printf(TEXT("Demo Program Loaded! (%d instructions)"),
                CurrentProgram.Num()));
    }
}

void ARobotCharacter::ClearProgram()
{
    if (bIsExecuting)
    {
        StopProgram();
    }

    CurrentProgram.Empty();
    InstructionPointer = 0;
    ExecutionStack.Empty();

    UE_LOG(LogTemp, Log, TEXT("ARobotCharacter: Program cleared."));

    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::White, TEXT("Program Cleared"));
    }
}

void ARobotCharacter::OnMovementComplete()
{
    // The new command-based path handles completion via MoveCommand's delegate.
    // This method is kept so ARobotAIController::OnMoveCompleted still compiles
    // when it calls Robot->OnMovementComplete().
    UE_LOG(LogTemp, Verbose, TEXT("ARobotCharacter: OnMovementComplete (legacy callback)"));
}