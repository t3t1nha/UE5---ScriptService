#include "Commands/DeliverCommand.h"
#include "RobotCharacter.h"
#include "TableManager.h"
#include "TableActor.h"
#include "IOrderable.h"
#include "EngineUtils.h"

void UDeliverCommand::InitializeDeliver(ARobotCharacter* InRobot)
{
    Initialize(InRobot);
}

bool UDeliverCommand::CanExecute() const
{
    if (!Super::CanExecute())
    {
        return false;
    }
    
    // Check if carrying a dish
    if (!OwningRobot->CarryingDish)
    {
        ErrorMessage = TEXT("Not carrying any dish");
        return false;
    }
    
    // Find table
    ATableManager* TableManager = nullptr;
    for (TActorIterator<ATableManager> It(OwningRobot->GetWorld()); It; ++It)
    {
        TableManager = *It;
        break;
    }
    
    if (!TableManager)
    {
        ErrorMessage = TEXT("No TableManager found");
        return false;
    }
    
    ATableActor* Table = TableManager->FindTableByNumber(OwningRobot->CurrentOrder.TableNumber);
    if (!Table)
    {
        ErrorMessage = FString::Printf(TEXT("Table %d not found"), OwningRobot->CurrentOrder.TableNumber);
        return false;
    }
    
    IOrderable* Orderable = Cast<IOrderable>(Table);
    if (!Orderable)
    {
        ErrorMessage = TEXT("Table does not implement IOrderable");
        return false;
    }
    
    // Check distance
    FVector TableCenter = Orderable->GetInteractionLocation();
    float Distance = FVector::Dist(OwningRobot->GetActorLocation(), TableCenter);
    
    if (Distance > INTERACTION_RANGE)
    {
        ErrorMessage = FString::Printf(TEXT("Too far from table (%.1f cm)"), Distance);
        return false;
    }
    
    return true;
}

FString UDeliverCommand::GetErrorMessage() const
{
    if (!ErrorMessage.IsEmpty())
    {
        return ErrorMessage;
    }
    return Super::GetErrorMessage();
}

void UDeliverCommand::Execute()
{
    if (!CanExecute())
    {
        FailCommand(GetErrorMessage());
        return;
    }
    
    // Find table
    ATableManager* TableManager = nullptr;
    for (TActorIterator<ATableManager> It(OwningRobot->GetWorld()); It; ++It)
    {
        TableManager = *It;
        break;
    }
    
    ATableActor* Table = TableManager->FindTableByNumber(OwningRobot->CurrentOrder.TableNumber);
    IOrderable* Orderable = Cast<IOrderable>(Table);
    
    // Attempt delivery
    bool bSuccess = Orderable->DeliverOrder(OwningRobot->CarryingDish);
    
    if (bSuccess)
    {
        UE_LOG(LogTemp, Log, TEXT("DeliverCommand: Successfully delivered to table %d"), 
            OwningRobot->CurrentOrder.TableNumber);
        
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Green, 
                FString::Printf(TEXT("Correct dish delivered to Table %d!"), 
                    OwningRobot->CurrentOrder.TableNumber));
        }
        
        // Clear inventory
        OwningRobot->CarryingDish = nullptr;
        OwningRobot->CurrentOrder = FOrderData();
        
        CompleteCommand();
    }
    else
    {
        
        FailCommand(TEXT("Wrong dish delivered"));
    }
}

FString UDeliverCommand::GetDisplayName() const
{
    return FString::Printf(TEXT("Deliver to Table %d"), OwningRobot->CurrentOrder.TableNumber);
}