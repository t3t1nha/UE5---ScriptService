// Fill out your copyright notice in the Description page of Project Settings.


#include "BlockWidget.h"

void UBlockWidget::InitializeBlock(FBlockData InBlockData)
{
	BlockData = InBlockData;

	Instruction.InstructionType = InBlockData.InstructionType;

	Instruction.TargetTableNumber = 1;
	Instruction.WaitValue = 1.0f;

	UpdateVisuals();
}

FRobotInstruction UBlockWidget::GetInstruction()
{
	if (TableNumberInput && TableNumberInput->GetVisibility() == ESlateVisibility::Visible)
	{
		FString TableText = TableNumberInput->GetText().ToString();
		Instruction.TargetTableNumber = FCString::Atoi(*TableText);
	}

	if (WaitValueInput && WaitValueInput->GetVisibility() == ESlateVisibility::Visible)
	{
		FString WaitText = WaitValueInput->GetText().ToString();
		Instruction.WaitValue = FCString::Atof(*WaitText);
	}
	
	return Instruction;
}

void UBlockWidget::UpdateVisuals()
{
	if (BlockNameText)
	{
		BlockNameText->SetText(BlockData.DisplayName);
	}
    
	if (BlockBorder)
	{
		BlockBorder->SetBrushColor(BlockData.BlockColor);
	}
    
	if (TableNumberInput)
	{
		TableNumberInput->SetVisibility(BlockData.bHasTableParameter ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
	}
    
	if (WaitValueInput)
	{
		WaitValueInput->SetVisibility(BlockData.bHasWaitParameter ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
	}
}

void UBlockWidget::NativeConstruct()
{
	Super::NativeConstruct();
}
