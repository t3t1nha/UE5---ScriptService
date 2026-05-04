// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/ProgressBar.h"
#include "CookingProgressWidget.generated.h"

class AApparatusActor;

UCLASS()
class SCRIPTED_SERVICE_API UCookingProgressWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadWrite)
	AApparatusActor* OwningApparatus = nullptr;

	/**
	 * Cooking progress value pushed by AApparatusActor every ~0.05s.
	 * Range: 0.0 (just started) to 1.0 (done).
	 * Bind your progress bar's Percent property to this in Blueprint.
	 */
	UPROPERTY(BlueprintReadWrite, Category = "Cooking")
	float CookingProgress = 0.0f;

	/**
	 * Called by AApparatusActor to push the latest progress value.
	 * Also fires the BlueprintImplementableEvent so Blueprint can
	 * react (e.g. colour shifts, pulse animations).
	 *
	 * @param NewProgress  Normalised progress in [0, 1].
	 */
	UFUNCTION(BlueprintCallable, Category = "Cooking")
	void UpdateProgress(float NewProgress);

	/**
	 * Override in Blueprint to react to progress changes —
	 * e.g. change bar colour when nearly done, play a pulse, etc.
	 */
	UFUNCTION(BlueprintImplementableEvent, Category = "Cooking")
	void OnProgressUpdated(float NewProgress);
};
