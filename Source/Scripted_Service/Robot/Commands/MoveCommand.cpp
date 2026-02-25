// MoveCommand.cpp
#include "MoveCommand.h"
#include "RobotCharacter.h"
#include "RobotAIController.h"
#include "TableManager.h"
#include "TableActor.h"
#include "IOrderable.h"
#include "EngineUtils.h"

void UMoveCommand::InitializeMoveToLocation(ARobotCharacter* InRobot, const FVector& InTargetLocation)
{
    Initialize(InRobot);
    MoveType = EMoveTargetType::Location;
    TargetLocation = InTargetLocation;
}

void UMoveCommand::InitializeMoveToTable(ARobotCharacter* InRobot, int32 TableNumber)
{
    Initialize(InRobot);
    MoveType = EMoveTargetType::Table;
    TargetTableNumber = TableNumber;
}

void UMoveCommand::InitializeMoveToKitchen(ARobotCharacter* InRobot)
{
    Initialize(InRobot);
    MoveType = EMoveTargetType::Kitchen;
}

bool UMoveCommand::CanExecute() const
{
    if (!Super::CanExecute())
    {
        return false;
    }
    
    // Check if robot has AI controller
    ARobotAIController* AIController = Cast<ARobotAIController>(OwningRobot->GetController());
    if (!AIController)
    {
        ErrorMessage = TEXT("Robot has no AI controller");
        return false;
    }
    
    // Validate based on move type
    if (MoveType == EMoveTargetType::Table)
    {
        // Find table manager
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
        
        // Check if table exists
        ATableActor* Table = TableManager->FindTableByNumber(TargetTableNumber);
        if (!Table)
        {
            ErrorMessage = FString::Printf(TEXT("Table %d not found"), TargetTableNumber);
            return false;
        }
    }
    else if (MoveType == EMoveTargetType::Kitchen)
    {
        // Find table manager for kitchen location
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
    }
    
    return true;
}

FString UMoveCommand::GetErrorMessage() const
{
    if (!ErrorMessage.IsEmpty())
    {
        return ErrorMessage;
    }
    
    return Super::GetErrorMessage();
}

FVector UMoveCommand::CalculateTableTargetLocation() const
{
    // Find table
    ATableManager* TableManager = nullptr;
    for (TActorIterator<ATableManager> It(OwningRobot->GetWorld()); It; ++It)
    {
        TableManager = *It;
        break;
    }
    
    if (!TableManager)
    {
        return FVector::ZeroVector;
    }
    
    ATableActor* Table = TableManager->FindTableByNumber(TargetTableNumber);
    if (!Table)
    {
        return FVector::ZeroVector;
    }
    
    IOrderable* Orderable = Cast<IOrderable>(Table);
    if (!Orderable)
    {
        return FVector::ZeroVector;
    }
    
    // Get table center
    FVector TableCenter = Orderable->GetInteractionLocation();
    FVector RobotLocation = OwningRobot->GetActorLocation();
    
    // Calculate direction from robot to table
    FVector ToTable = TableCenter - RobotLocation;
    ToTable.Z = 0;  // Horizontal plane only
    
    float DistanceToTable = ToTable.Size();
    
    // If already very close, return current position
    if (DistanceToTable < TABLE_STOP_DISTANCE + 50.0f)
    {
        return RobotLocation;
    }
    
    ToTable.Normalize();
    
    // Calculate target: Stop TABLE_STOP_DISTANCE away from center
    FVector Target = TableCenter - (ToTable * TABLE_STOP_DISTANCE);
    Target.Z = RobotLocation.Z;  // Same height as robot
    
    return Target;
}

void UMoveCommand::Execute()
{
    UE_LOG(LogTemp, Warning, TEXT("=== MOVECOMMAND EXECUTE ==="));
    
    if (!CanExecute())
    {
        UE_LOG(LogTemp, Error, TEXT("MoveCommand: CanExecute failed"));
        FailCommand(GetErrorMessage());
        return;
    }
    
    // Calculate actual target location based on move type
    FVector FinalTarget;
    
    switch (MoveType)
    {
        case EMoveTargetType::Location:
            FinalTarget = TargetLocation;
            UE_LOG(LogTemp, Warning, TEXT("Move Type: Location -> %s"), *FinalTarget.ToString());
            break;
            
        case EMoveTargetType::Table:
            FinalTarget = CalculateTableTargetLocation();
            UE_LOG(LogTemp, Warning, TEXT("Move Type: Table %d"), TargetTableNumber);
            UE_LOG(LogTemp, Warning, TEXT("Calculated target: %s"), *FinalTarget.ToString());
            if (FinalTarget.IsZero())
            {
                UE_LOG(LogTemp, Error, TEXT("Target is ZERO - calculation failed!"));
                FailCommand(TEXT("Failed to calculate table target location"));
                return;
            }
            break;
            
        case EMoveTargetType::Kitchen:
        {
            ATableManager* TableManager = nullptr;
            for (TActorIterator<ATableManager> It(OwningRobot->GetWorld()); It; ++It)
            {
                TableManager = *It;
                break;
            }
            
            if (TableManager)
            {
                FinalTarget = TableManager->GetKitchenLocation();
                UE_LOG(LogTemp, Warning, TEXT("Move Type: Kitchen -> %s"), *FinalTarget.ToString());
            }
            else
            {
                UE_LOG(LogTemp, Error, TEXT("No TableManager found!"));
                FailCommand(TEXT("No TableManager found"));
                return;
            }
            break;
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Final target location: %s"), *FinalTarget.ToString());
    UE_LOG(LogTemp, Warning, TEXT("Robot current location: %s"), *OwningRobot->GetActorLocation().ToString());
    
    // Debug visualization
    if (GEngine)
    {
        DrawDebugSphere(OwningRobot->GetWorld(), FinalTarget, 50.0f, 12, FColor::Yellow, false, 5.0f);
        DrawDebugLine(OwningRobot->GetWorld(), 
            OwningRobot->GetActorLocation(), 
            FinalTarget, 
            FColor::Blue, false, 5.0f, 0, 3.0f);
    }
    
    ARobotAIController* AIController = Cast<ARobotAIController>(OwningRobot->GetController());
    if (!AIController)
    {
        UE_LOG(LogTemp, Error, TEXT("No AI Controller!"));
        FailCommand(TEXT("No AI Controller"));
        return;
    }
    
    UE_LOG(LogTemp, Warning, TEXT("AI Controller found, binding callback..."));
    
    // Bind callback for when movement completes
    AIController->OnMoveComplete.BindUObject(this, &UMoveCommand::OnMovementComplete);
    
    UE_LOG(LogTemp, Warning, TEXT("Callback bound, starting movement..."));
    
    // Start movement
    AIController->MoveToLocation(FinalTarget);
    
    UE_LOG(LogTemp, Warning, TEXT("MoveToLocation called, movement started!"));
}

void UMoveCommand::OnMovementComplete()
{
    UE_LOG(LogTemp, Warning, TEXT("=== MOVECOMMAND CALLBACK FIRED ==="));
    UE_LOG(LogTemp, Warning, TEXT("Movement complete, calling CompleteCommand"));
    CompleteCommand();
}   

void UMoveCommand::Cancel()
{
    Super::Cancel();
    
    ARobotAIController* AIController = Cast<ARobotAIController>(OwningRobot->GetController());
    if (AIController)
    {
        AIController->StopMovement();
    }
}

FString UMoveCommand::GetDisplayName() const
{
    switch (MoveType)
    {
        case EMoveTargetType::Location:
            return FString::Printf(TEXT("Move to %s"), *TargetLocation.ToCompactString());
        case EMoveTargetType::Table:
            return FString::Printf(TEXT("Move to Table %d"), TargetTableNumber);
        case EMoveTargetType::Kitchen:
            return TEXT("Move to Kitchen");
    }
    
    return TEXT("Move");
}