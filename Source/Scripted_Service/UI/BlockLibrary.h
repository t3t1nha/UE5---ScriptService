#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "StructTypes.h"
#include "BlockLibrary.generated.h"

UCLASS(BlueprintType)
class SCRIPTED_SERVICE_API UBlockLibrary : public UObject
{
	GENERATED_BODY()

public:
	// All available blocks
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blocks")
	TArray<FBlockData> AvailableBlocks;
    
	UFUNCTION(BlueprintCallable, Category = "Blocks")
	static TArray<FBlockData> GetDefaultBlocks();
};