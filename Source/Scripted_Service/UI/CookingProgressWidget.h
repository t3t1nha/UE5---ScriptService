// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CookingProgressWidget.generated.h"

class AApparatusActor;

UCLASS()
class SCRIPTED_SERVICE_API UCookingProgressWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadWrite)
	AApparatusActor* OwningApparatus = nullptr;
};
