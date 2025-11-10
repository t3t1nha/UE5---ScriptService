// Fill out your copyright notice in the Description page of Project Settings.


#include "AApparatusActor.h"

// Sets default values
AAApparatusActor::AAApparatusActor()
{
 	PrimaryActorTick.bCanEverTick = false;

	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>("StaticMesh");
	MeshComponent->SetupAttachment(RootComponent);
	
	
	TriggerBoxComponent = CreateDefaultSubobject<UBoxComponent>(FName("TriggerBoxComponent"));
	TriggerBoxComponent->SetupAttachment(MeshComponent);
	TriggerBoxComponent->OnComponentBeginOverlap.AddDynamic(this, &AAApparatusActor::OnTriggerBoxComponentOverlap);
}

// Called when the game starts or when spawned
void AAApparatusActor::BeginPlay()
{
	Super::BeginPlay();
	
}

void AAApparatusActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AAApparatusActor::CheckForRecipe()
{
	CurrentRecipeData = FRecipeData();

	TArray<FName> RowNames = RecipeDataTable->GetRowNames();

	for (const FName& RowName : RowNames)
	{
		FRecipeData* Recipe = RecipeDataTable->FindRow<FRecipeData>(RowName, TEXT("Looking up Recipe Data"));

		if (Recipe)
		{
			// Skip not intended recipes for this Apparatus
			if (Recipe->RequiredApparatus != ApparatusType)
			{
				continue;
			}

			// Skip if the number of required ingredients doesn't match the current number
			if (Recipe->RequiredIngredients.Num() > CurrentIngredients.Num())
			{
				continue;
			}

			bool bisMatch = true;

			for (const auto& RequiredElem : Recipe->RequiredIngredients)
			{
				const FName& RequiredItemID = RequiredElem.Key;
				const int32 RequiredQuantity = RequiredElem.Value;

				if (!CurrentIngredients.Contains(RequiredItemID))
				{
					bisMatch = false;
					break; // Missing an ingredient
				}

				if (CurrentIngredients[RequiredItemID] != RequiredQuantity)
				{
					bisMatch = false;
					break; // Quantity Mismatch
				}
			}

			// Check if the CurrentIngredients map contains any extra items
			for (const auto& CurrentElem : CurrentIngredients)
			{
				if (!Recipe->RequiredIngredients.Contains(CurrentElem.Key))
				{
					bisMatch = false;
					break; // Extra ingredient found
				}
			}

			if (bisMatch)
			{
				CurrentRecipeData = *Recipe;
				if (GEngine)
				{
					GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Blue, TEXT("Recipe Found"));
				}
			}
		}
	}

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Blue, TEXT("No Recipe Found"));
	}
}

void AAApparatusActor::AddIngredient(FName ItemID)
{
	if (CurrentIngredients.Contains(ItemID))
	{
		CurrentIngredients[ItemID]++;
	}
	else
	{
		CurrentIngredients.Add(ItemID);
	}

	CheckForRecipe();

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, TEXT("Added Ingredient: " + ItemID.ToString()));
	}
}

void AAApparatusActor::RemoveIngredient(FName ItemID)
{
	if (CurrentIngredients.Contains(ItemID))
	{
		CurrentIngredients[ItemID]--;
	}
	else
	{
		CurrentIngredients.Remove(ItemID);
	}

	CheckForRecipe();

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, TEXT("Removed Ingredient: " + ItemID.ToString()));
	}
}

void AAApparatusActor::StartCookingProcess()
{
	if (CurrentRecipeData.OutputItemID != NAME_None && CurrentRecipeData.BaseCookTime > 0.0f)
	{
		GetWorldTimerManager().ClearTimer(CookingTimerHandle);

		GetWorldTimerManager().SetTimer(
			CookingTimerHandle,
			this,
			&AAApparatusActor::FinishCooking,
			CurrentRecipeData.BaseCookTime,
			false
		);

		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Blue, TEXT("Cooking Started"));
		}
	}
}

void AAApparatusActor::FinishCooking()
{
	GetWorldTimerManager().ClearTimer(CookingTimerHandle);

	if (CurrentRecipeData.OutputItemID != NAME_None)
	{
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Blue, TEXT("Cooking Finished"));

			// TODO: Spawning Logic
		}

		CurrentIngredients.Empty();
		CurrentRecipeData = FRecipeData();
	}
}

void AAApparatusActor::OnTriggerBoxComponentOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Blue, TEXT("OnTriggerBoxComponentOverlap" + OtherActor->GetName()));
	}
}
