#pragma once

#include "CoreMinimal.h"
#include "RobotCommand.h"
#include "DeliverCommand.generated.h"

UCLASS()
class SCRIPTED_SERVICE_API UDeliverCommand : public URobotCommand
{
	GENERATED_BODY()

public:
	void InitializeDeliver(ARobotCharacter* InRobot);
    
	virtual bool CanExecute() const override;
	virtual FString GetErrorMessage() const override;
	virtual void Execute() override;
	virtual FString GetDisplayName() const override;

private:
	static constexpr float INTERACTION_RANGE = 350.0f;
};