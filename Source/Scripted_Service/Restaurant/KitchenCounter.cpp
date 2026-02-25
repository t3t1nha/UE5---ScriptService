// KitchenCounter.cpp
#include "KitchenCounter.h"
#include "BaseIngredient.h"
#include "EnumTypes.h"

AKitchenCounter::AKitchenCounter()
{
    PrimaryActorTick.bCanEverTick = false;
    
    PickupZone = CreateDefaultSubobject<UBoxComponent>(TEXT("PickupZone"));
    RootComponent = PickupZone;
    PickupZone->SetBoxExtent(FVector(100.0f, 100.0f, 50.0f));
}

void AKitchenCounter::BeginPlay()
{
    Super::BeginPlay();
    
    PickupZone->OnComponentBeginOverlap.AddDynamic(this, &AKitchenCounter::OnDishPlaced);
}

void AKitchenCounter::OnDishPlaced(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep,
    const FHitResult& SweepResult)
{
    ABaseIngredient* Ingredient = Cast<ABaseIngredient>(OtherActor);
    if (Ingredient && Ingredient->IngredientType == EIngredientType::CookedDish)
    {
        TSubclassOf<ABaseIngredient> DishClass = Ingredient->GetClass();
        if (!AvailableDishes.Contains(DishClass))
        {
            AvailableDishes.Add(DishClass);
            UE_LOG(LogTemp, Log, TEXT("Kitchen Counter: Dish placed - %s"), *DishClass->GetName());
        }
    }
}

/**
 * IPickupPoint INTERFACE IMPLEMENTATION
 */
bool AKitchenCounter::HasItem(TSubclassOf<ABaseIngredient> ItemClass) const
{
    return AvailableDishes.Contains(ItemClass);
}

bool AKitchenCounter::PickupItem(TSubclassOf<ABaseIngredient> ItemClass)
{
    if (!HasItem(ItemClass))
    {
        UE_LOG(LogTemp, Warning, TEXT("Kitchen Counter: Item %s not available"), 
            ItemClass ? *ItemClass->GetName() : TEXT("NULL"));
        return false;
    }
    
    // Remove from available list
    AvailableDishes.Remove(ItemClass);
    
    // Find and destroy the physical actor
    TArray<AActor*> OverlappingActors;
    PickupZone->GetOverlappingActors(OverlappingActors, ABaseIngredient::StaticClass());
    
    for (AActor* Actor : OverlappingActors)
    {
        if (Actor->GetClass() == ItemClass)
        {
            Actor->Destroy();
            UE_LOG(LogTemp, Log, TEXT("Kitchen Counter: Picked up %s"), *ItemClass->GetName());
            return true;
        }
    }
    
    return false;
}

FVector AKitchenCounter::GetPickupLocation() const
{
    // Return location in front of counter
    FVector Forward = GetActorForwardVector();
    return GetActorLocation() + (Forward * 150.0f);
}

TArray<TSubclassOf<ABaseIngredient>> AKitchenCounter::GetAvailableItems() const
{
    return AvailableDishes;
}