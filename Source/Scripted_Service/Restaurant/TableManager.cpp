// Fill out your copyright notice in the Description page of Project Settings.


#include "TableManager.h"
#include "EngineUtils.h"
#include "TableActor.h"

// Sets default values
ATableManager::ATableManager()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

void ATableManager::RegisterTable(ATableActor* Table)
{
	if (Table && !AllTables.Contains(Table))
	{
		AllTables.Add(Table);
		Table->TableNumber = AllTables.Num();
		GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Yellow, 
		FString::Printf(TEXT("Registered Table %d"), Table->TableNumber));
	}
}

ATableActor* ATableManager::FindTableByNumber(int32 TableNumber)
{
	for (ATableActor* Table : AllTables)
	{
		if (Table && Table->TableNumber == TableNumber)
		{
			return Table;
		}
	}

	GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Red, 
	FString::Printf(TEXT("Table %d not found!"), TableNumber));
    
	return nullptr;
}

AKitchenCounter* ATableManager::GetKitchenCounter() const
{
	return KitchenCounter;
}

// Called when the game starts or when spawned
void ATableManager::BeginPlay()
{
	Super::BeginPlay();

	FindAllTablesInLevel();
}

void ATableManager::FindAllTablesInLevel()
{
	AllTables.Empty();
    
	// Find all tables
	for (TActorIterator<ATableActor> It(GetWorld()); It; ++It)
	{
		ATableActor* Table = *It;
		RegisterTable(Table);
	}
    
	// Find kitchen counter
	for (TActorIterator<AKitchenCounter> It(GetWorld()); It; ++It)
	{
		KitchenCounter = *It;
        
		GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, TEXT("Found Kitchen Counter!"));
		
		break;
	}
	GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, 
	FString::Printf(TEXT("Found %d tables in level"), AllTables.Num()));
	
}

// Called every frame
void ATableManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

