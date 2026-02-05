// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "KitchenCounter.h"
#include "TableActor.h"
#include "TableManager.generated.h"

UCLASS()
class SCRIPTED_SERVICE_API ATableManager : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ATableManager();

	UPROPERTY(BlueprintReadOnly, Category = "Tables")
	TArray<ATableActor*> AllTables;
    
	UFUNCTION(BlueprintCallable, Category = "Tables")
	void RegisterTable(ATableActor* Table);
    
	UFUNCTION(BlueprintCallable, Category = "Tables")
	ATableActor* FindTableByNumber(int32 TableNumber);
    
	UFUNCTION(BlueprintCallable, Category = "Tables")
	FVector GetKitchenLocation() const { return KitchenLocation; }

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kitchen")
	FVector KitchenLocation = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kitchen")
	AKitchenCounter* KitchenCounter;
	
	UFUNCTION(BlueprintCallable, Category = "Kitchen")
	AKitchenCounter* GetKitchenCounter() const;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	void FindAllTablesInLevel();

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
