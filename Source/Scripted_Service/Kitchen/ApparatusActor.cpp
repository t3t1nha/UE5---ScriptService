// Fill out your copyright notice in the Description page of Project Settings.


#include "ApparatusActor.h"
#include "BaseIngredient.h"


// Sets default values
AApparatusActor::AApparatusActor()
{
 	PrimaryActorTick.bCanEverTick = false;

	ApparatusMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>("ApparatusMesh");
	SetRootComponent(ApparatusMeshComponent);
	
	DropZoneComponent = CreateDefaultSubobject<UBoxComponent>(FName("DropZone"));
	DropZoneComponent->SetupAttachment(ApparatusMeshComponent);
	DropZoneComponent->SetCollisionProfileName(FName("OverlapAll"));
}

// Called when the game starts or when spawned
void AApparatusActor::BeginPlay()
{
	Super::BeginPlay();

	if (DropZoneComponent)
	{
		DropZoneComponent->OnComponentBeginOverlap.AddDynamic(this, &AApparatusActor::OnDropZoneOverlapBegin);
		DropZoneComponent->OnComponentEndOverlap.AddDynamic(this, &AApparatusActor::OnDropZoneOverlapEnd);
	}
}

void AApparatusActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AApparatusActor::Interact_Implementation()
{
	IInteractInterface::Interact_Implementation();
	StartCookingProcess();
	GEngine->AddOnScreenDebugMessage(-1, 5, FColor::Red, TEXT("Apparatus Interact"));
}

void AApparatusActor::CheckForRecipe()
{
    CurrentRecipeData = FRecipeData();
    bool bRecipeFound = false;

	if (!RecipeDataTable)
	{
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, TEXT("RecipeDataTable not set on Apparatus"));
		}
		return;
	}
    TArray<FName> RowNames = RecipeDataTable->GetRowNames();

    for (const FName& RowName : RowNames)
    {
        if (FRecipeData* Recipe = RecipeDataTable->FindRow<FRecipeData>(RowName, TEXT("Looking up Recipe Data")))
        {
            // Skip not intended recipes for this Apparatus
            if (Recipe->RequiredApparatus != ApparatusType)
            {
                continue;
            }

            // Skip if the number of required ingredients doesn't match the current number
            if (Recipe->RequiredIngredients.Num() != CurrentIngredients.Num()) // Changed > to !=
            {
                continue;
            }

            bool bIsMatch = true;

            // Check if all required ingredients are present with correct quantities
            for (const auto& RequiredElem : Recipe->RequiredIngredients)
            {
                const FName& RequiredItemID = RequiredElem.Key;
                const int32 RequiredQuantity = RequiredElem.Value;

                if (!CurrentIngredients.Contains(RequiredItemID))
                {
                    bIsMatch = false;
                    break;
                }

                if (RequiredQuantity != CurrentIngredients[RequiredItemID])
                {
                    bIsMatch = false;
                    break;
                }
            }

            // Check if the CurrentIngredients map contains any extra items
            if (bIsMatch)
            {
                for (const auto& CurrentElem : CurrentIngredients)
                {
                    if (!Recipe->RequiredIngredients.Contains(CurrentElem.Key))
                    {
                        bIsMatch = false;
                        break;
                    }
                }
            }

            if (bIsMatch)
            {
                CurrentRecipeData = *Recipe;
                bRecipeFound = true;
                if (GEngine)
                {
                    GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Blue, TEXT("Recipe Found: ") + RowName.ToString());
                }
            	
                break; // Exit loop if recipe was found
            }
        }
    }

    if (!bRecipeFound && GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow, TEXT("No Recipe Found"));
    }
}

void AApparatusActor::AddIngredient(ABaseIngredient* Ingredient)
{
	FName ItemID = Ingredient->ItemID;
	if (CurrentIngredients.Contains(ItemID))
	{
		CurrentIngredients[ItemID]++;
	}
	else
	{
		CurrentIngredients.Add(ItemID, 1);
	}
	
	CurrentIngredientActors.Add(Ingredient);
	CheckForRecipe();

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, TEXT("Added Ingredient: " + ItemID.ToString()));
	}
}

void AApparatusActor::RemoveIngredient(ABaseIngredient* Ingredient)
{
	FName ItemID = Ingredient->ItemID;
	
	if (CurrentIngredients.Contains(ItemID))
	{
		CurrentIngredients[ItemID]--;
	}
	else
	{
		CurrentIngredients.Remove(ItemID);
	}
	
	CurrentIngredientActors.Remove(Ingredient);
	CheckForRecipe();

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, TEXT("Removed Ingredient: " + ItemID.ToString()));
	}
}

void AApparatusActor::StartCookingProcess()
{
	if (CurrentRecipeData.OutputItemID != NAME_None && CurrentRecipeData.BaseCookTime > 0.0f)
	{
		GetWorldTimerManager().ClearTimer(CookingTimerHandle);

		GetWorldTimerManager().SetTimer(
			CookingTimerHandle,
			this,
			&AApparatusActor::FinishCooking,
			CurrentRecipeData.BaseCookTime,
			false
		);

		const FVector ParticleLocation = DropZoneComponent->GetComponentLocation();
		
		if (CookingLoopParticles)
		{
			ActiveLoopParticles = UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), CookingLoopParticles, ParticleLocation);
		}
		
		if (CookingStartSound)
		{
			UGameplayStatics::PlaySoundAtLocation(this, CookingStartSound, GetActorLocation());
		}

		if (CookingLoopSound)
		{
			ActiveLoopSound = UGameplayStatics::SpawnSoundAtLocation(this, CookingLoopSound, GetActorLocation());
		}
		
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Blue, TEXT("Cooking Started"));
		}
	} else if (GetWorldTimerManager().IsTimerActive(CookingTimerHandle))
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Blue, TEXT("Cooking already in progress"));
	}
	else
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, TEXT("Cooking Not Started"));
	}
}

void AApparatusActor::FinishCooking()
{
	GetWorldTimerManager().ClearTimer(CookingTimerHandle);

	if (ActiveLoopParticles)
	{
		ActiveLoopParticles->DeactivateSystem();
		ActiveLoopParticles = nullptr;
	}
	
	if (ActiveLoopSound)
	{
		if (CookingFinishSound)
		{
			UGameplayStatics::PlaySoundAtLocation(this, CookingFinishSound, GetActorLocation());
		}
		
		ActiveLoopSound->Stop();
		ActiveLoopSound = nullptr;
	}
	
	if (CurrentRecipeData.OutputItemID != NAME_None)
	{
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Blue, TEXT("Cooking Finished"));

		}
		
		if (!ItemBaseDataTable) return;

		FItemBaseData* ItemData = ItemBaseDataTable->FindRow<FItemBaseData>(CurrentRecipeData.OutputItemID, TEXT("Spawning Output"));

		if (ItemData && IngredientBPClass)
		{
			FVector SpawnLocation = DropZoneComponent->GetComponentLocation() + FVector(0.0f, 0.0f, 20.0f);

			// Spawn generic object
			ABaseIngredient* NewIngredient = GetWorld()->SpawnActor<ABaseIngredient>(IngredientBPClass, SpawnLocation, FRotator::ZeroRotator);

			if (NewIngredient)
			{
				// Make correct ingredient
				NewIngredient->ItemID = CurrentRecipeData.OutputItemID;

				NewIngredient->UpdateVisuals();
			}
		}

		for (ABaseIngredient* IngredientActor : CurrentIngredientActors)
		{
			IngredientActor->Destroy();			
		}

		CurrentIngredientActors.Empty();
		CurrentIngredients.Empty();
		CurrentRecipeData = FRecipeData();
	}
}

void AApparatusActor::OnDropZoneOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor)
	{
		ABaseIngredient* IngredientActor = Cast<ABaseIngredient>(OtherActor);

		if (IngredientActor)
		{
			FName ItemID = IngredientActor->ItemID;

			if (ItemID == NAME_None)
			{
				if (GEngine)
				{
					GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Blue, TEXT("Actor Does not have valid Item ID" + ItemID.ToString()));
				}
			} else
			{
				AddIngredient(IngredientActor);
			}
		}
	}
	else
	{
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Black, TEXT("Actor is not an ingredient"));
		}
	}
}

void AApparatusActor::OnDropZoneOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (OtherActor)
	{
		ABaseIngredient* IngredientActor = Cast<ABaseIngredient>(OtherActor);

		if (IngredientActor)
		{
			FName ItemID = IngredientActor->ItemID;

			if (ItemID == NAME_None)
			{
				GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Blue, TEXT("Actor Does not have valid Item ID" + ItemID.ToString()));
			}
			else
			{
				RemoveIngredient(IngredientActor);
			}
		}
	}
}