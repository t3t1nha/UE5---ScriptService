
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/BoxComponent.h"
#include "IPickupPoint.h"
#include "KitchenCounter.generated.h"

UCLASS()
class SCRIPTED_SERVICE_API AKitchenCounter : public AActor, public IPickupPoint
{
	GENERATED_BODY()
    
public:    
	AKitchenCounter();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UStaticMeshComponent* CounterMesh;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UBoxComponent* PickupZone;
    
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Counter")
	TArray<TSubclassOf<class ABaseIngredient>> AvailableDishes;
    
	// IPickupPoint Interface Implementation
	virtual bool HasItem(TSubclassOf<class ABaseIngredient> ItemClass) const override;
	virtual bool PickupItem(TSubclassOf<class ABaseIngredient> ItemClass) override;
	virtual FVector GetPickupLocation() const override;
	virtual TArray<TSubclassOf<class ABaseIngredient>> GetAvailableItems() const override;

protected:
	virtual void BeginPlay() override;
    
	UFUNCTION()
	void OnDishPlaced(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep,
		const FHitResult& SweepResult);
};