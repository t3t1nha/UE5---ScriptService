// Fill out your copyright notice in the Description page of Project Settings.


#include "ApparatusActor.h"
#include "BaseIngredient.h"
#include "Components/AudioComponent.h"
#include "kismet/GameplayStatics.h"
#include "Particles/ParticleSystem.h"
#include "Particles/ParticleSystemComponent.h"


// Sets default values
AApparatusActor::AApparatusActor()
{
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

}

void AApparatusActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AApparatusActor::Interact_Implementation()
{
	IInteractInterface::Interact_Implementation();
	StartCookingProcess();
}

void AApparatusActor::CheckForRecipe()
{
    CurrentRecipeData = FRecipeData();

	if (!RecipeDataTable)
	{
		return;
	}
    TArray<FName> RowNames = RecipeDataTable->GetRowNames();

    for (const FName& RowName : RowNames)
    {
        if (FRecipeData* Recipe = RecipeDataTable->FindRow<FRecipeData>(RowName, TEXT("Looking up Recipe Data")))
        {
        	if (!Recipe->bIsUnlocked)
        	{
        		return;
        	}
        	
            if (Recipe->RequiredApparatus != ApparatusType)
            {
                continue;
            }

            if (Recipe->RequiredIngredients.Num() != CurrentIngredients.Num())
            {
                continue;
            }
        	
            bool bIsMatch = true;

            for (const auto& RequiredElem : Recipe->RequiredIngredients)
            {
                const TSubclassOf<ABaseIngredient> RequiredIngredient = RequiredElem.Key;
                const int32 RequiredQuantity = RequiredElem.Value;

                if (!CurrentIngredients.Contains(RequiredIngredient))
                {
                    bIsMatch = false;
                    break;
                }

                if (RequiredQuantity != CurrentIngredients[RequiredIngredient])
                {
                    bIsMatch = false;
                    break;
                }
            }

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
                break;
            }
        }
    }
}

void AApparatusActor::AddIngredient(ABaseIngredient* Ingredient)
{
	TSubclassOf<ABaseIngredient> IngredientClass = Ingredient->GetClass();
	
	if (CurrentIngredients.Contains(IngredientClass))
	{
		CurrentIngredients[IngredientClass]++;
	}
	else
	{
		CurrentIngredients.Add(IngredientClass, 1);
	}
	
	CurrentIngredientActors.Add(Ingredient);
	CheckForRecipe();

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, TEXT("Added Ingredient: " + Ingredient->GetName()));
	}
}

void AApparatusActor::RemoveIngredient(ABaseIngredient* Ingredient)
{
	TSubclassOf<ABaseIngredient> IngredientClass = Ingredient->GetClass();
	
	if (CurrentIngredients.Contains(IngredientClass))
	{
		CurrentIngredients[IngredientClass]--;

		if (CurrentIngredients[IngredientClass] <= 0)
		{
			CurrentIngredients.Remove(IngredientClass);
		}
	}
	
	CurrentIngredientActors.Remove(Ingredient);
	CheckForRecipe();

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, TEXT("Removed Ingredient: " + Ingredient->GetName()));
	}
}

void AApparatusActor::StartCookingProcess()
{
	if (GetWorldTimerManager().GetTimerRemaining(CookingTimerHandle) > 0)
	{
		return;
	}

	OnStartCooking();
	
	if (CurrentRecipeData.OutputItemSubclass != nullptr && CurrentRecipeData.BaseCookTime > 0.0f)
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
	} else {
		GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, TEXT("Cooking not started or in Progress"));
	}
}

void AApparatusActor::SnapIngredient(ABaseIngredient* ToSnapIngredient)
{
	if (!ToSnapIngredient)
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, TEXT("No ingredient"));
		return;
	}
	
	UPrimitiveComponent* Root = Cast<UPrimitiveComponent>(ToSnapIngredient->GetRootComponent());
	if (Root)
	{
		Root->SetSimulatePhysics(false);
	}

	const FVector& DropZoneLocation = DropZoneComponent->GetComponentLocation();
	ToSnapIngredient->SetActorLocation(DropZoneLocation);
	
	AddIngredient(ToSnapIngredient);
	
	GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, TEXT("Snapping Ingredient"));
}

void AApparatusActor::FinishCooking()
{
	OnFinishCooking();
	GetWorldTimerManager().ClearTimer(CookingTimerHandle);
	
	if (ActiveLoopSound)
	{
		if (CookingFinishSound)
		{
			UGameplayStatics::PlaySoundAtLocation(this, CookingFinishSound, GetActorLocation());
		}
		
		ActiveLoopSound->Stop();
		ActiveLoopSound = nullptr;
	}
	
	if (CurrentRecipeData.OutputItemSubclass != nullptr)
	{
		FVector SpawnLocation;
		
		if (CurrentIngredientActors.Num() == 1)
		{
			SpawnLocation = CurrentIngredientActors[0]->GetActorLocation();
		} else
		{
			SpawnLocation = DropZoneComponent->GetComponentLocation();
		}
		
		GetWorld()->SpawnActor<ABaseIngredient>(CurrentRecipeData.OutputItemSubclass,SpawnLocation, FRotator::ZeroRotator);

		for (ABaseIngredient* IngredientActor : CurrentIngredientActors)
		{
			IngredientActor->Destroy();			
		}

		CurrentIngredientActors.Empty();
		CurrentIngredients.Empty();
		CurrentRecipeData = FRecipeData();
	}

	if (ActiveLoopParticles)
	{
		ActiveLoopParticles->DeactivateSystem();
		ActiveLoopParticles = nullptr;
	}
}

float AApparatusActor::GetCookingProgress() const
{
	if (!GetWorldTimerManager().IsTimerActive(CookingTimerHandle))
	{
		return 0.0f;
	}
	
	const float Elapsed   = GetWorldTimerManager().GetTimerElapsed(CookingTimerHandle);
	const float Remaining = GetWorldTimerManager().GetTimerRemaining(CookingTimerHandle);
	const float Total     = Elapsed + Remaining;
		
	if (Total <= 0.0f)
	{
		return 0.0f;
	}
	
	return FMath::Clamp(Elapsed / Total, 0.0f, 1.0f);
}