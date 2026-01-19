// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "StructTypes.h"
#include "Components/TextBlock.h"
#include "Components/Border.h"
#include "Components/EditableTextBox.h"
#include "BlockWidget.generated.h"

UCLASS()
class SCRIPTED_SERVICE_API UBlockWidget : public UUserWidget
{
	GENERATED_BODY()

	public:
	UPROPERTY(BlueprintReadWrite, Category = "Block")
	FRobotInstruction Instruction;

	UPROPERTY(BlueprintReadWrite, Category = "Block")
	FBlockData BlockData;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* BlockNameText;
    
	UPROPERTY(meta = (BindWidget))
	UBorder* BlockBorder;
    
	UPROPERTY(meta = (BindWidget), BlueprintReadWrite)
	UEditableTextBox* TableNumberInput;
    
	UPROPERTY(meta = (BindWidget), BlueprintReadWrite)
	UEditableTextBox* WaitValueInput;

	UFUNCTION(BlueprintCallable, Category = "Block")
	void InitializeBlock(FBlockData InBlockData);
	
	UFUNCTION(BlueprintCallable, Category = "Block")
	FRobotInstruction GetInstruction();

	UFUNCTION(BlueprintCallable, Category = "Block")
	void UpdateVisuals();

	protected:
	virtual void NativeConstruct() override;
};
