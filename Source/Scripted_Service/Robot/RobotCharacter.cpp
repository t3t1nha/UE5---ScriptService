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
#include "Scripted_Service/Player_Character.h"

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

    static const FName DefaultSlotNames[] = { TEXT("A"), TEXT("B"), TEXT("C"), TEXT("D") };
    MemorySlots.SetNum(4);
    for (int32 i = 0; i < 4; i++)
    {
        MemorySlots[i].SlotName    = DefaultSlotNames[i];
        MemorySlots[i].IntValue    = 0;
        MemorySlots[i].bIsSet      = false;
    }
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

void ARobotCharacter::Interact_Implementation()
{
   // APlayerController* PC = Cast<APlayerController>(GetController());
   // if (!PC){ GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Emerald, TEXT("NO PC")); return;
   // }

    APlayer_Character* PlayerCharacter = nullptr;
    for (TActorIterator<APlayer_Character> ActorIterator(GetWorld()); ActorIterator; ++ActorIterator)
    {
        PlayerCharacter = *ActorIterator;
    }
    if (!PlayerCharacter){ GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Emerald, TEXT("NoPlayerChar")); return; }

    PlayerCharacter->OpenRobotOS(this);
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
        InstructionPointer = 0;
        ExecutionStack.Empty();
        CurrentOrder       = FOrderData();
        CarryingDish       = nullptr;
        CurrentCommand     = nullptr;

        ClearAllSlots();
    }

    bIsExecuting = true;
    bIsPaused    = false;

    UE_LOG(LogTemp, Log,
        TEXT("ARobotCharacter: Executing program — %d instruction(s)"),
        CurrentProgram.Num());

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
            const int32 ResolvedTable = ResolveTableNumber(Instr);
            if (ATableActor* Table = TableManager->FindTableByNumber(ResolvedTable))
            {
                if (IOrderable* Ord = Cast<IOrderable>(Table))
                {
                    bCondition = Ord->HasPendingOrder();
                }
            }

            UE_LOG(LogTemp, Log,
            TEXT("ARobotCharacter: IfTableHasOrder(Table %d) → %s"),
            ResolveTableNumber(Instr), bCondition ? TEXT("TRUE") : TEXT("FALSE"));

            if (bCondition) InstructionPointer++;
            else            InstructionPointer = FindMatchingEndBlock(InstructionPointer) + 1;

            ExecuteCurrentInstruction();
            return;
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
        Frame.RemainingIterations = -1;
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
                    InstructionPointer = Frame.LoopStartIndex + 1;

                    UE_LOG(LogTemp, Verbose,
                        TEXT("ARobotCharacter: EndBlock — %d iteration(s) left, "
                             "jumping to IP=%d"),
                        Frame.RemainingIterations, InstructionPointer);
                }
                else
                {
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
            UE_LOG(LogTemp, Warning,
                TEXT("ARobotCharacter: Unmatched EndBlock at IP=%d — skipping."),
                InstructionPointer);
            InstructionPointer++;
        }

        ExecuteCurrentInstruction();
        return;
    }

        case EInstructionType::SetSlot:
    {
        SetSlotValue(Instr.SlotIndex, Instr.SlotValue);
        InstructionPointer++;
        ExecuteCurrentInstruction();
        return;
    }
    
    case EInstructionType::IncrementSlot:
    {
        // slot[N] += 1.  Useful for counting served tables, loop iterations, etc.
        const int32 NewValue = GetSlotValue(Instr.SlotIndex) + 1;
        SetSlotValue(Instr.SlotIndex, NewValue);
        InstructionPointer++;
        ExecuteCurrentInstruction();
        return;
    }
    
    case EInstructionType::DecrementSlot:
    {
        // slot[N] -= 1.  Pair with IfSlotGreaterThan 0 to make a countdown loop.
        const int32 NewValue = GetSlotValue(Instr.SlotIndex) - 1;
        SetSlotValue(Instr.SlotIndex, NewValue);
        InstructionPointer++;
        ExecuteCurrentInstruction();
        return;
    }
    
    case EInstructionType::IfSlotEquals:
    {
        const int32  SlotVal  = GetSlotValue(Instr.SlotIndex);
        const bool   bCondition = (SlotVal == Instr.SlotValue);
    
        UE_LOG(LogTemp, Log,
            TEXT("ARobotCharacter: IfSlotEquals — Slot[%d]=%d == %d → %s"),
            Instr.SlotIndex, SlotVal, Instr.SlotValue,
            bCondition ? TEXT("TRUE") : TEXT("FALSE"));
    
        if (bCondition) InstructionPointer++;
        else            InstructionPointer = FindMatchingEndBlock(InstructionPointer) + 1;
    
        ExecuteCurrentInstruction();
        return;
    }
    
    case EInstructionType::IfSlotGreaterThan:
    {
        const int32 SlotVal    = GetSlotValue(Instr.SlotIndex);
        const bool  bCondition = (SlotVal > Instr.SlotValue);
    
        UE_LOG(LogTemp, Log,
            TEXT("ARobotCharacter: IfSlotGreaterThan — Slot[%d]=%d > %d → %s"),
            Instr.SlotIndex, SlotVal, Instr.SlotValue,
            bCondition ? TEXT("TRUE") : TEXT("FALSE"));
    
        if (bCondition) InstructionPointer++;
        else            InstructionPointer = FindMatchingEndBlock(InstructionPointer) + 1;
    
        ExecuteCurrentInstruction();
        return;
    }
    
    case EInstructionType::IfSlotLessThan:
    {
        const int32 SlotVal    = GetSlotValue(Instr.SlotIndex);
        const bool  bCondition = (SlotVal < Instr.SlotValue);
    
        UE_LOG(LogTemp, Log,
            TEXT("ARobotCharacter: IfSlotLessThan — Slot[%d]=%d < %d → %s"),
            Instr.SlotIndex, SlotVal, Instr.SlotValue,
            bCondition ? TEXT("TRUE") : TEXT("FALSE"));
    
        if (bCondition) InstructionPointer++;
        else            InstructionPointer = FindMatchingEndBlock(InstructionPointer) + 1;
    
        ExecuteCurrentInstruction();
        return;
    }
        
    case EInstructionType::IfKitchenHasOrder:
    {
        bool bCondition = false;
    
        if (TableManager)
        {
            if (AKitchenCounter* Counter = TableManager->GetKitchenCounter())
            {
                bCondition = Counter->GetAvailableItems().Num() > 0;
            }
        }
        else
        {
            UE_LOG(LogTemp, Warning,
                TEXT("ARobotCharacter: IfKitchenHasOrder — no TableManager, condition is FALSE."));
        }
    
        UE_LOG(LogTemp, Log,
            TEXT("ARobotCharacter: IfKitchenHasOrder → %s"),
            bCondition ? TEXT("TRUE") : TEXT("FALSE"));
    
        if (bCondition) InstructionPointer++;
        else            InstructionPointer = FindMatchingEndBlock(InstructionPointer) + 1;
    
        ExecuteCurrentInstruction();
        return;
    }
    
    case EInstructionType::IfTableWaitingTooLong:
    {
        bool bCondition = false;
        const int32 ResolvedTable = ResolveTableNumber(Instr);
    
        if (TableManager)
        {
            if (ATableActor* Table = TableManager->FindTableByNumber(ResolvedTable))
            {
                // Only meaningful if there's actually a pending order
                if (Table->HasPendingOrder())
                {
                    bCondition = Table->CurrentOrder.TimeWaiting
                                 >= static_cast<float>(Instr.SlotValue);
                }
            }
        }
    
        UE_LOG(LogTemp, Log,
            TEXT("ARobotCharacter: IfTableWaitingTooLong(Table %d, threshold %ds) → %s"),
            ResolvedTable, Instr.SlotValue, bCondition ? TEXT("TRUE") : TEXT("FALSE"));
    
        if (bCondition) InstructionPointer++;
        else            InstructionPointer = FindMatchingEndBlock(InstructionPointer) + 1;
    
        ExecuteCurrentInstruction();
        return;
        }
        
    default:
        break;
    }


    const FRobotInstruction ResolvedInstr = ResolveInstruction(Instr);
    URobotCommand* Command = CreateCommandFromInstruction(ResolvedInstr);

    if (!Command)
    {
        UE_LOG(LogTemp, Error,
            TEXT("ARobotCharacter: No command handler for InstructionType=%d at IP=%d. "
                 "Skipping."),
            static_cast<int32>(Instr.InstructionType), InstructionPointer);

        InstructionPointer++;
        ExecuteCurrentInstruction();
        return;
    }

    if (!Command->CanExecute())
    {
        OnCommandError(Command->GetErrorMessage());
        return;
    }

    CurrentCommand = Command;

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
}

void ARobotCharacter::OnCommandComplete()
{
    if (CurrentProgram.IsValidIndex(InstructionPointer))
    {
        const FRobotInstruction& CompletedInstr = CurrentProgram[InstructionPointer];

        if (CompletedInstr.bSaveToSlot)
        {
            switch (CompletedInstr.InstructionType)
            {
            case EInstructionType::TakeOrder:
                SetSlotValue(CompletedInstr.SaveToSlotIndex, CurrentOrder.TableNumber);
                break;

            case EInstructionType::MoveToTable:
                SetSlotValue(CompletedInstr.SaveToSlotIndex,
                    ResolveTableNumber(CompletedInstr));
                break;

            default:
                UE_LOG(LogTemp, Warning,
                    TEXT("ARobotCharacter: bSaveToSlot set on instruction type %d "
                         "which has no save handler — ignored."),
                    static_cast<int32>(CompletedInstr.InstructionType));
                break;
            }
        }
    }
    
    CurrentCommand = nullptr;

    InstructionPointer++;

    UE_LOG(LogTemp, Log,
        TEXT("ARobotCharacter: Command complete — advancing to IP=%d"),
        InstructionPointer);

    ExecuteCurrentInstruction();
}

void ARobotCharacter::OnCommandError(FString ErrorMessage)
{
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red,
            FString::Printf(TEXT("ERROR: "), *ErrorMessage));
    }

    OnCommandErro.Broadcast(ErrorMessage);
    
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
            Type == EInstructionType::IfTableHasOrder           ||
            Type == EInstructionType::IfCarryingDish            ||
            Type == EInstructionType::IfNotCarryingDish         ||
            Type == EInstructionType::RepeatLoop                ||
            Type == EInstructionType::LoopForever               ||
                
            Type == EInstructionType::IfSlotEquals              ||
            Type == EInstructionType::IfSlotGreaterThan         ||
            Type == EInstructionType::IfSlotLessThan            ||
            Type == EInstructionType::IfKitchenHasOrder         ||
            Type == EInstructionType::IfTableWaitingTooLong;

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

    case EInstructionType::IfTableHasOrder:
    case EInstructionType::IfCarryingDish:
    case EInstructionType::IfNotCarryingDish:
    case EInstructionType::RepeatLoop:
    case EInstructionType::LoopForever:
    case EInstructionType::EndBlock:
        
    case EInstructionType::SetSlot:
    case EInstructionType::IncrementSlot:
    case EInstructionType::DecrementSlot:
    case EInstructionType::IfSlotEquals:
    case EInstructionType::IfSlotGreaterThan:
    case EInstructionType::IfSlotLessThan:
    case EInstructionType::IfKitchenHasOrder:
    case EInstructionType::IfTableWaitingTooLong:
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

int32 ARobotCharacter::GetSlotValue(int32 SlotIndex) const
{
    if (!MemorySlots.IsValidIndex(SlotIndex))
    {
        UE_LOG(LogTemp, Warning,
            TEXT("ARobotCharacter::GetSlotValue — index %d is out of range (max %d). Returning 0."),
            SlotIndex, MemorySlots.Num() - 1);
        return 0;
    }

    const FRobotMemorySlot& Slot = MemorySlots[SlotIndex];

    if (!Slot.bIsSet)
    {
        UE_LOG(LogTemp, Warning,
            TEXT("ARobotCharacter::GetSlotValue — Slot[%d] ('%s') has never been written. Returning 0."),
            SlotIndex, *Slot.SlotName.ToString());
    }

    return Slot.IntValue;
}

void ARobotCharacter::SetSlotValue(int32 SlotIndex, int32 Value)
{
    if (!MemorySlots.IsValidIndex(SlotIndex))
    {
        UE_LOG(LogTemp, Warning,
            TEXT("ARobotCharacter::SetSlotValue — index %d is out of range."), SlotIndex);
        return;
    }

    MemorySlots[SlotIndex].IntValue = Value;
    MemorySlots[SlotIndex].bIsSet   = true;

    UE_LOG(LogTemp, Log,
        TEXT("ARobotCharacter: Slot[%d] ('%s') ← %d"),
        SlotIndex, *MemorySlots[SlotIndex].SlotName.ToString(), Value);

    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Cyan,
            FString::Printf(TEXT("Slot %s = %d"),
                *MemorySlots[SlotIndex].SlotName.ToString(), Value));
    }
}

void ARobotCharacter::ClearSlot(int32 SlotIndex)
{
    if (MemorySlots.IsValidIndex(SlotIndex))
    {
        MemorySlots[SlotIndex].IntValue = 0;
        MemorySlots[SlotIndex].bIsSet   = false;
    }
}

void ARobotCharacter::ClearAllSlots()
{
    for (FRobotMemorySlot& Slot : MemorySlots)
    {
        Slot.IntValue = 0;
        Slot.bIsSet   = false;
    }
    UE_LOG(LogTemp, Log, TEXT("ARobotCharacter: All memory slots cleared."));
}

int32 ARobotCharacter::ResolveTableNumber(const FRobotInstruction& Instr) const
{
    if (!Instr.bReadTableFromSlot)
    {
        return Instr.TargetTableNumber;
    }

    if (!MemorySlots.IsValidIndex(Instr.ReadFromSlotIndex))
    {
        UE_LOG(LogTemp, Warning,
            TEXT("ARobotCharacter::ResolveTableNumber — slot index %d out of range. "
                 "Falling back to literal %d."),
            Instr.ReadFromSlotIndex, Instr.TargetTableNumber);
        return Instr.TargetTableNumber;
    }

    const FRobotMemorySlot& Slot = MemorySlots[Instr.ReadFromSlotIndex];

    if (!Slot.bIsSet)
    {
        UE_LOG(LogTemp, Warning,
            TEXT("ARobotCharacter::ResolveTableNumber — Slot[%d] ('%s') unset. "
                 "Returning 0."),
            Instr.ReadFromSlotIndex, *Slot.SlotName.ToString());
    }

    return Slot.IntValue;
}

FRobotInstruction ARobotCharacter::ResolveInstruction(const FRobotInstruction& Instr) const
{
    // Make a mutable copy so the caller always gets plain literals.
    FRobotInstruction Resolved = Instr;

    if (Instr.bReadTableFromSlot)
    {
        Resolved.TargetTableNumber  = ResolveTableNumber(Instr);
        Resolved.bReadTableFromSlot = false; // Already baked in — no double-resolve
    }

    return Resolved;
}