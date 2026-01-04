// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"
#include "CoreMinimal.h"
#include "EnumTypes.generated.h"

UENUM(BlueprintType)
enum class ECookState : uint8
{
	Raw			UMETA(DisplayName = "Raw"),
	Chopped		UMETA(DisplayName = "Chopped"),
	Sliced		UMETA(DisplayName = "Sliced"),
	Cooked		UMETA(DisplayName = "Cooked"),
	Boiled		UMETA(DisplayName = "Boiled"),
	Fried		UMETA(DisplayName = "Fried"),
	Burnt		UMETA(DisplayName = "Burnt")
};

UENUM(BlueprintType)
enum class EApparatusType : uint8
{
	Grill			UMETA(DisplayName = "Grill"),
	Stove			UMETA(DisplayName = "Stove"),
	Oven			UMETA(DisplayName = "Oven"),
	CuttingBoard	UMETA(DisplayName = "Cutting Board"),
	Plate			UMETA(DisplayName = "Plate"),
	WaterPot		UMETA(DisplayName = "Water Pot")
};

UENUM(BlueprintType)
enum class EIngredientType : uint8
{
	Vegetable		UMETA(DisplayName = "Vegetable"),
	Meat			UMETA(DisplayName = "Meat"),
	Condiment		UMETA(DisplayName = "Condiment"),
	Liquid			UMETA(DisplayName = "Liquid"),
	CookedDish		UMETA(DisplayName = "Cooked Dish"),
	Apparatus		UMETA(DisplayName = "Apparatus")
};
