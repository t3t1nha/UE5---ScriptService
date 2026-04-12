#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "RobotCommand.h"
#include "EnumTypes.h"
#include "InteractInterface.h"
#include "StructTypes.h"
#include "IProgrammable.h"
#include "RobotCharacter.generated.h"

/**
 */
UCLASS()
class SCRIPTED_SERVICE_API ARobotCharacter : public ACharacter, public IInteractInterface, public IProgrammable
{
    GENERATED_BODY()

public:
    ARobotCharacter();

    /** The flat bytecode array loaded from the Programming Menu */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Robot|Program")
    TArray<FRobotInstruction> CurrentProgram;

    /**
     * Index of the instruction currently being evaluated.
     * Updated after every action completes or every control-flow step.
     */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Robot|Interpreter")
    int32 InstructionPointer;

    /** True while the program is loaded and running (including paused) */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Robot|Interpreter")
    bool bIsExecuting;

    /** True when execution has been suspended with PauseProgram() */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Robot|Interpreter")
    bool bIsPaused;

    /** The order the robot is currently carrying (taken from a table) */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Robot|Inventory")
    FOrderData CurrentOrder;

    /**
     * The dish class the robot is physically carrying.
     * Set by PickupCommand, cleared by DeliverCommand.
     */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Robot|Inventory")
    TSubclassOf<class ABaseIngredient> CarryingDish;

    virtual void Interact_Implementation() override;
    
    virtual void LoadProgram(const TArray<FRobotInstruction>& Instructions) override;

    UFUNCTION(BlueprintCallable, Category = "Robot|Program")
    virtual void ExecuteProgram() override;

    virtual void StopProgram()   override;
    virtual void PauseProgram()  override;
    virtual void ResumeProgram() override;

    virtual bool  IsProgramRunning()         const override;
    virtual int32 GetCurrentInstructionIndex() const override;
    virtual int32 GetProgramLength()           const override;

    UFUNCTION(BlueprintCallable, Category = "Robot|Debug")
    void LoadDemoProgram();

    /** Empties CurrentProgram and resets all execution state */
    UFUNCTION(BlueprintCallable, Category = "Robot|Debug")
    void ClearProgram();

    void OnMovementComplete();

protected:
    virtual void BeginPlay() override;

private:
    UPROPERTY()
    class ATableManager* TableManager;

    FTimerHandle WaitTimerHandle;
    
    struct FExecFrame
    {
        /** IP of the RepeatLoop / LoopForever instruction (body starts at IP+1) */
        int32 LoopStartIndex = 0;

        /**
         * How many more times to run the body.
         * -1 is a sentinel meaning "loop forever".
         */
        int32 RemainingIterations = 1;
    };

    /** Stack of active loop frames; empty when no loops are in progress */
    TArray<FExecFrame> ExecutionStack;

    /**
     * The URobotCommand that is currently executing asynchronously.
     * nullptr when the interpreter is on a control-flow step or idle.
     * Kept as a UPROPERTY so the GC doesn't collect it mid-execution.
     */
    UPROPERTY()
    URobotCommand* CurrentCommand;

    /**
     * Evaluate and dispatch the instruction at InstructionPointer.
     *
     * Control-flow instructions advance IP and call themselves recursively
     * (the recursion depth equals nesting depth, which is bounded to 1 in the
     * UI, so stack overflow is not a concern in practice).
     *
     * Action instructions create a URobotCommand, validate it, and call
     * Execute().  The callback chain (OnCommandComplete / OnCommandError)
     * drives the next call to ExecuteCurrentInstruction().
     */
    void ExecuteCurrentInstruction();

    /**
     * Scans forward from StartIndex to find the index of the EndBlock that
     * closes the block opened at StartIndex.  Correctly handles nested blocks
     * by tracking depth.
     *
     * @param StartIndex  IP of the block-opening instruction (If*, RepeatLoop…)
     * @return            IP of the matching EndBlock, or the last valid IP if
     *                    the program is malformed (no matching EndBlock found).
     */
    int32 FindMatchingEndBlock(int32 StartIndex) const;

    /**
     * Creates the appropriate URobotCommand for a given action instruction.
     * Returns nullptr for control-flow instructions — they are not commands.
     *
     * @param Instruction  The instruction to convert.
     * @return             New URobotCommand (outer-owned), or nullptr.
     */
    URobotCommand* CreateCommandFromInstruction(const FRobotInstruction& Instruction);

    /**
     * Bound to URobotCommand::OnComplete.
     * Advances IP by one and calls ExecuteCurrentInstruction().
     */
    void OnCommandComplete();

    /**
     * Bound to URobotCommand::OnError.
     * Logs the error, shows it on-screen, and halts execution.
     *
     * @param ErrorMessage  Human-readable description of the failure.
     */
    void OnCommandError(FString ErrorMessage);
};