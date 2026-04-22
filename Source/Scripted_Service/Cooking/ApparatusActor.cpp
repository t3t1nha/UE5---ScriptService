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
 	PrimaryActorTick.bCanEverTick = false;

	ApparatusMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>("ApparatusMesh");
	SetRootComponent(ApparatusMeshComponent);
	
	DropZoneComponent = CreateDefaultSubobject<UBoxComponent>(FName("DropZone"));
	DropZoneComponent->SetupAttachment(ApparatusMeshComponent);
	DropZoneComponent->SetCollisionProfileName(FName("OverlapAll"));

	CookingProgressComponent = CreateDefaultSubobject<UWidgetComponent>(TEXT("CookingProgressComponent"));
	CookingProgressComponent->SetupAttachment(ApparatusMeshComponent);
	CookingProgressComponent->SetWidgetSpace(EWidgetSpace::Screen);
	CookingProgressComponent->SetDrawSize(FVector2D(200.f, 40.f));
	CookingProgressComponent->SetVisibility(true);
	CookingProgressComponent->SetRelativeLocation(FVector(0.f, 0.f, 120.f));
}

// Called when the game starts or when spawned
void AApparatusActor::BeginPlay()
{
	Super::BeginPlay();

	if (DropZoneComponent)
	{
		// DropZoneComponent->OnComponentBeginOverlap.AddDynamic(this, &AApparatusActor::OnDropZoneOverlapBegin);
		DropZoneComponent->OnComponentEndOverlap.AddDynamic(this, &AApparatusActor::OnDropZoneOverlapEnd);
	}

	if (CookingProgressComponent && CookingProgressWidgetClass)
	{
		CookingProgressComponent->SetWidgetClass(CookingProgressWidgetClass);
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

	if (!RecipeDataTable)
	{
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
            if (Recipe->RequiredIngredients.Num() != CurrentIngredients.Num())
            {
                continue;
            }

            bool bIsMatch = true;

            // Check if all required ingredients are present with correct quantities
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
                break; // Exit loop if recipe was found
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

		if (CookingProgressComponent && CookingProgressWidgetClass)
		{
			CookingProgressComponent->SetVisibility(true);
			
			UCookingProgressWidget* ProgressWidget = Cast<UCookingProgressWidget>(
				CookingProgressComponent->GetUserWidgetObject());

			if (ProgressWidget)
			{
				ProgressWidget->OwningApparatus = this;
			}
		}
		
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
	ToSnapIngredient->SetActorRotation(FRotator(0.0f, 0.0f, 0.0f));
	
	AddIngredient(ToSnapIngredient);
	
	GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, TEXT("Snapping Ingredient"));
}

void AApparatusActor::FinishCooking()
{
	GetWorldTimerManager().ClearTimer(CookingTimerHandle);

	if (CookingProgressComponent)
	{
		CookingProgressComponent->SetVisibility(false);
	}
	
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
	
	if (CurrentRecipeData.OutputItemSubclass != nullptr)
	{
		FVector SpawnLocation, Offset = FVector(0.0f, 0.0f, 20.0f);
		
		if (CurrentIngredientActors.Num() == 1)
		{
			SpawnLocation = CurrentIngredientActors[0]->GetActorLocation() + Offset;
		} else
		{
			SpawnLocation = DropZoneComponent->GetComponentLocation() + Offset;
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

void AApparatusActor::OnDropZoneOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
                                             UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor)
	{
		ABaseIngredient* IngredientActor = Cast<ABaseIngredient>(OtherActor);

		if (IngredientActor)
		{
			AddIngredient(IngredientActor);
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
				RemoveIngredient(IngredientActor);
		}
	}
}