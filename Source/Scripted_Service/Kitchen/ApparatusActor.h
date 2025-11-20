// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BaseIngredient.h"
#include "GameFramework/Actor.h"
#include "TimerManager.h"
#include "Sound/SoundBase.h"
#include "Components/AudioComponent.h"
#include "kismet/GameplayStatics.h"
#include "Particles/ParticleSystem.h"
#include "Particles/ParticleSystemComponent.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/DataTable.h"
#include "Interface/InteractInterface.h"
#include "CookingSystemTypes.h"
#include "ApparatusActor.generated.h"

class UStaticMesh;
class UBoxComponent;
class UDataTable;

UCLASS()
class SCRIPTED_SERVICE_API AApparatusActor : public AActor, public IInteractInterface
{
	GENERATED_BODY()
	
public:	
	AApparatusActor();
	virtual void Tick(float DeltaTime) override;
	virtual void Interact_Implementation() override;

	UFUNCTION(BlueprintCallable, Category = "Cooking")
	void CheckForRecipe();

	UFUNCTION(BlueprintCallable, Category = "Cooking")
	void StartCookingProcess();
	
protected:
	virtual void BeginPlay() override;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UStaticMeshComponent> ApparatusMeshComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UBoxComponent> DropZoneComponent;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
	EApparatusType ApparatusType = EApparatusType::Grill;
	
	UPROPERTY(BlueprintReadOnly, Category = "State", meta = (AllowPrivateAccess = "true"))
	TMap<FName, int32> CurrentIngredients;

	UPROPERTY(BlueprintReadonly, Category = "State", meta = (AllowPrivateAccess = "true"))
	TArray<ABaseIngredient*> CurrentIngredientActors;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Data")
	UDataTable* RecipeDataTable;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Data")
	UDataTable* ItemBaseDataTable;

	// VFX
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Effects")
	UParticleSystem* CookingLoopParticles;

	UPROPERTY(BlueprintReadOnly, Category = "Effects")
	UParticleSystemComponent* ActiveLoopParticles;

	
	
	// SFX
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Effects")
	USoundBase* CookingStartSound;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Effects")
	USoundBase* CookingLoopSound;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Effects")
	USoundBase* CookingFinishSound;

	UPROPERTY(BlueprintReadOnly, Category = "Effects")
	UAudioComponent* ActiveLoopSound;	


	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Data")
	TSubclassOf<class ABaseIngredient> IngredientBPClass;
	
	UPROPERTY(BlueprintReadOnly, Category = "State", meta = (AllowPrivateAccess = "true"))
	FRecipeData CurrentRecipeData;

	FTimerHandle CookingTimerHandle;

	UFUNCTION(BlueprintCallable, Category = "Cooking")
	void AddIngredient(ABaseIngredient* Ingredient);

	UFUNCTION(BlueprintCallable, Category = "Cooking")
	void RemoveIngredient(ABaseIngredient* Ingredient);

	UFUNCTION()
	void FinishCooking();

	UFUNCTION()
	virtual void OnDropZoneOverlapBegin(UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult);

	UFUNCTION()
	virtual void OnDropZoneOverlapEnd(UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex);
};
