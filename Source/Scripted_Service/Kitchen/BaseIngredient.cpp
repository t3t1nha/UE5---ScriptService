// Fill out your copyright notice in the Description page of Project Settings.


#include "BaseIngredient.h"

#include "CookingSystemTypes.h"

// Sets default values
ABaseIngredient::ABaseIngredient()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	ItemMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ItemMesh"));
	SetRootComponent(ItemMeshComponent);

	ItemMeshComponent->SetSimulatePhysics(true);
	ItemMeshComponent->SetCollisionProfileName(TEXT("PhysicsActor"));
}

// Called when the game starts or when spawned
void ABaseIngredient::BeginPlay()
{
	Super::BeginPlay();
}

void ABaseIngredient::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	UpdateVisuals();
}

// Called every frame
void ABaseIngredient::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ABaseIngredient::UpdateVisuals()
{
	if (ItemID == NAME_None) return;
	if (!ItemBaseDataTable)
	{
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("Data Table Not Set On ingredient"));
		}
		return;
	}

	FItemBaseData* Data = ItemBaseDataTable->FindRow<FItemBaseData>(ItemID, TEXT("IngredientVisualUpdate"));

	if (Data && Data->Mesh)
	{
		ItemMeshComponent->SetStaticMesh(Data->Mesh);
	}
}

