// Fill out your copyright notice in the Description page of Project Settings.


#include "KitchenCounter.h"

// Sets default values
AKitchenCounter::AKitchenCounter()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	CounterMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CounterMesh"));
	SetRootComponent(CounterMesh);

	PickupZone = CreateDefaultSubobject<UBoxComponent>(TEXT("PickupZone"));
	PickupZone->SetupAttachment(CounterMesh);
	PickupZone->SetCollisionProfileName(FName("OverlapAll"));
}

ABaseIngredient* AKitchenCounter::PickupDish(TSubclassOf<ABaseIngredient> DishClass)
{
	for (ABaseIngredient* Dish : AvailableDishes)
	{
		if (Dish && Dish->IsA(DishClass))
		{
			AvailableDishes.Remove(Dish);
            
			if (GEngine)
			{
				GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Yellow, 
					FString::Printf(TEXT("Robot picked up: %s"), *Dish->GetClass()->GetName()));
			}
            
			return Dish;
		}
	}
    
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Red, TEXT("Dish not found on counter!"));
	}
    
	return nullptr;
}

bool AKitchenCounter::HasDish(TSubclassOf<ABaseIngredient> DishClass)
{
	for (ABaseIngredient* Dish : AvailableDishes)
	{
		if (Dish && Dish->IsA(DishClass))
		{
			return true;
		}
	}
	return false;
}

// Called when the game starts or when spawned
void AKitchenCounter::BeginPlay()
{
	Super::BeginPlay();

	if (PickupZone)
	{
		PickupZone->OnComponentBeginOverlap.AddDynamic(this, &AKitchenCounter::OnPickupZoneBeginOverlap);
		PickupZone->OnComponentEndOverlap.AddDynamic(this, &AKitchenCounter::OnPickupZoneEndOverlap);
	}
}

void AKitchenCounter::OnPickupZoneBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (ABaseIngredient* Dish = Cast<ABaseIngredient>(OtherActor))
	{
		// Only track finished dishes (CookedDish type)
		if (Dish->IngredientType == EIngredientType::CookedDish)
		{
			if (!AvailableDishes.Contains(Dish))
			{
				AvailableDishes.Add(Dish);
                
				if (GEngine)
				{
					GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Cyan, 
						FString::Printf(TEXT("Dish added to counter: %s"), *Dish->GetClass()->GetName()));
				}
			}
		}
	}
}

void AKitchenCounter::OnPickupZoneEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (ABaseIngredient* Dish = Cast<ABaseIngredient>(OtherActor))
	{
		AvailableDishes.Remove(Dish);
	}
}

// Called every frame
void AKitchenCounter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

