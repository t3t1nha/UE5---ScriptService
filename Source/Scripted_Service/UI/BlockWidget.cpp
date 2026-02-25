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

FReply UBlockWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	if (InMouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
	{
		return FReply::Handled().DetectDrag(TakeWidget(), EKeys::LeftMouseButton);
	}

	return FReply::Unhandled();
}

void UBlockWidget::NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent,
	UDragDropOperation*& OutOperation)
{
	Super::NativeOnDragDetected(InGeometry, InMouseEvent, OutOperation);

	UDragDropOperation* DragOp = NewObject<UDragDropOperation>();
	
	UBlockWidget* DragVisual = CreateWidget<UBlockWidget>(GetWorld(), StaticClass());
	DragVisual->InitializeBlock(BlockData);

	DragOp->DefaultDragVisual = DragVisual;
	DragOp->Pivot = EDragPivot::MouseDown;
	DragOp->Payload = this;

	OutOperation = DragOp; 
	
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Green, FString::Printf(TEXT("Dragging block: %s"), *BlockData.DisplayName.ToString()));
	}
}

void UBlockWidget::NativeConstruct()
{
	Super::NativeConstruct();
}
