// Fill out your copyright notice in the Description page of Project Settings.


#include "BaseIngredient.h"


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
}

// Called every frame
void ABaseIngredient::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}