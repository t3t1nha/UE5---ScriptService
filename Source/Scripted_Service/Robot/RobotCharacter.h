#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
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
    virtual void ExecuteProgram() override;
    virtual void StopProgram() override;
    virtual void PauseProgram() override;
    virtual void ResumeProgram() override;
    virtual bool IsProgramRunning() const override;
    virtual int32 GetCurrentInstructionIndex() const override;
    virtual int32 GetProgramLength() const override;
    
    // Instruction execution (keep existing)
    void ExecuteNextInstruction();
    void OnMovementComplete();
    
    // Action functions (keep existing)
    void TakeOrderFromTable(int32 TableNumber);
    void PickupDishFromCounter();
    void DeliverDishToTable();
    void WaitForDuration(float Seconds);

protected:
    virtual void BeginPlay() override;
    
private:
    class ATableManager* TableManager;
    FTimerHandle WaitTimerHandle;
};