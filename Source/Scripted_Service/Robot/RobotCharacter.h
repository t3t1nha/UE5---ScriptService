#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "RobotCommand.h"
#include "EnumTypes.h"
#include "StructTypes.h"
#include "IProgrammable.h"
#include "RobotCharacter.generated.h"

UCLASS()
class SCRIPTED_SERVICE_API ARobotCharacter : public ACharacter, public IProgrammable
{
    GENERATED_BODY()

public:
    ARobotCharacter();

    // Program to execute
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Robot")
    TArray<FRobotInstruction> CurrentProgram;
    
    // Current state
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Robot")
    int32 CurrentInstructionIndex;
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Robot")
    bool bIsExecuting;
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Robot")
    bool bIsPaused;
    
    // Current inventory
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Robot")
    FOrderData CurrentOrder;
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Robot")
    TSubclassOf<class ABaseIngredient> CarryingDish;
    
    // IProgrammable Interface Implementation
    virtual void LoadProgram(const TArray<FRobotInstruction>& Instructions) override;
    UFUNCTION(BlueprintCallable, Category = "Robot|Demo")
    virtual void ExecuteProgram() override;
    virtual void StopProgram() override;
    virtual void PauseProgram() override;
    virtual void ResumeProgram() override;
    virtual bool IsProgramRunning() const override;
    virtual int32 GetCurrentInstructionIndex() const override;
    virtual int32 GetProgramLength() const override;
    
    void OnMovementComplete();
    
    UFUNCTION(BlueprintCallable, Category = "Robot|Demo")
    void LoadDemoProgram();

    UFUNCTION(BlueprintCallable, Category = "Robot|Demo")
    void ClearProgram();
    
protected:
    virtual void BeginPlay() override;
    
private:
    class ATableManager* TableManager;
    FTimerHandle WaitTimerHandle;

    // Command execution
    TArray<URobotCommand*> CommandQueue;
    int32 CurrentCommandIndex;

    // Helper to create command from instruction
    URobotCommand* CreateCommandFromInstruction(const FRobotInstruction& Instruction);
    
    // Execute next command in queue
    void ExecuteNextCommand();
    
    // Called when a command completes
    void OnCommandComplete();
    
    // Called when a command fails
    void OnCommandError(FString ErrorMessage);
};