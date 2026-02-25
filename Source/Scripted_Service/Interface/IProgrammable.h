// IProgrammable.h
#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "StructTypes.h"
#include "IProgrammable.generated.h"

UINTERFACE(MinimalAPI, Blueprintable)
class UProgrammable : public UInterface
{
	GENERATED_BODY()
};

/**
 * Interface for entities that can execute programs
 * Implemented by: RobotCharacter, (future: Drone, ConveyorBelt, etc.)
 */
class IProgrammable
{
	GENERATED_BODY()

public:
	/**
	 * Load a program into this programmable entity
	 * @param Instructions - Array of instructions to execute
	 */
	virtual void LoadProgram(const TArray<FRobotInstruction>& Instructions) = 0;
    
	/**
	 * Start executing the loaded program
	 */
	virtual void ExecuteProgram() = 0;
    
	/**
	 * Stop program execution immediately
	 */
	virtual void StopProgram() = 0;
    
	/**
	 * Pause program execution (can be resumed)
	 */
	virtual void PauseProgram() = 0;
    
	/**
	 * Resume paused program execution
	 */
	virtual void ResumeProgram() = 0;
    
	/**
	 * Check if program is currently running
	 */
	virtual bool IsProgramRunning() const = 0;
    
	/**
	 * Get the current instruction index being executed
	 */
	virtual int32 GetCurrentInstructionIndex() const = 0;
    
	/**
	 * Get the total number of instructions in the loaded program
	 */
	virtual int32 GetProgramLength() const = 0;
};