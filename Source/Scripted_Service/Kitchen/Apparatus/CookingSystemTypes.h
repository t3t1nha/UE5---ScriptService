#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h" 
#include "CookingSystemTypes.generated.h"

// --- ENUM ---

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
enum class EItemType : uint8
{
	Vegetable		UMETA(DisplayName = "Vegetable"),
	Meat			UMETA(DisplayName = "Meat"),
	Condiment		UMETA(DisplayName = "Condiment"),
	Liquid			UMETA(DisplayName = "Liquid"),
	CookedDish		UMETA(DisplayName = "Cooked Dish"),
	Apparatus		UMETA(DisplayName = "Apparatus")
};

// --- STRUCTS ---

USTRUCT(BlueprintType)
struct FItemBaseData : public FTableRowBase
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Data")
	EItemType ItemType = EItemType::Vegetable;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Data")
	FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Data")
	FText Description;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Data")
	class UStaticMesh* Mesh = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Data")
	ECookState DefaultCookState = ECookState::Raw;
};

USTRUCT(BlueprintType)
struct FRecipeData : public FTableRowBase
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recipe Data")
	EApparatusType RequiredApparatus = EApparatusType::Stove;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recipe Data")
	TMap<FName, int32> RequiredIngredients;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recipe Data")
	TMap<FName, ECookState> RequiredCookState;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recipe Data")
	FName OutputItemID = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recipe Data")
	float BaseCookTime = 5.0f;
};