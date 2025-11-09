// Fill out your copyright notice in the Description page of Project Settings.


#include "AApparatusActor.h"

// Sets default values
AAApparatusActor::AAApparatusActor()
{
 	PrimaryActorTick.bCanEverTick = false;
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
				GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Blue, TEXT("Recipe Found"));
			}
		}
	}

	GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Blue, TEXT("No Recipe Found"));
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

void AAApparatusActor::StartCookingProcess()
{
	
}

void AAApparatusActor::FinishCooking()
{
}

