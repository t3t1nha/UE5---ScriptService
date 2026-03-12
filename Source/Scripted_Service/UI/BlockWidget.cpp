// Fill out your copyright notice in the Description page of Project Settings.


#include "BlockWidget.h"

void UBlockWidget::InitializeBlock(FBlockData InBlockData)
{
	// Store the definition for use during drag and in GetInstruction()
	BlockData = InBlockData;

	// Pre-populate instruction fields with sensible defaults
	Instruction.InstructionType    = InBlockData.InstructionType;
	Instruction.TargetTableNumber  = 1;    // Default table: 1
	Instruction.WaitValue          = 1.0f; // Default wait: 1 second

	UpdateVisuals();
}

FRobotInstruction UBlockWidget::GetInstruction()
{
	if (TableNumberInput &&
		TableNumberInput->GetVisibility() == ESlateVisibility::Visible)
	{
		const FString TableText = TableNumberInput->GetText().ToString();
		Instruction.TargetTableNumber = FCString::Atoi(*TableText);
	}

	if (WaitValueInput &&
		WaitValueInput->GetVisibility() == ESlateVisibility::Visible)
	{
		const FString WaitText = WaitValueInput->GetText().ToString();
		Instruction.WaitValue = FCString::Atof(*WaitText);
	}

	return Instruction;
}

void UBlockWidget::UpdateVisuals()
{
	// Set the readable label on the text block
	if (BlockNameText)
	{
		BlockNameText->SetText(BlockData.DisplayName);
	}

	// Tint the block border with the category colour
	if (BlockBorder)
	{
		BlockBorder->SetBrushColor(BlockData.BlockColor);
	}

	// Show or hide the table number input based on the block's parameter flags
	if (TableNumberInput)
	{
		TableNumberInput->SetVisibility(
			BlockData.bHasTableParameter
				? ESlateVisibility::Visible
				: ESlateVisibility::Collapsed);
	}

	// Show or hide the wait duration input
	if (WaitValueInput)
	{
		WaitValueInput->SetVisibility(
			BlockData.bHasWaitParameter
				? ESlateVisibility::Visible
				: ESlateVisibility::Collapsed);
	}
}

void UBlockWidget::RequestRemove()
{
	if (OnRemoveRequested.IsBound())
	{
		OnRemoveRequested.Broadcast(this);

		UE_LOG(LogTemp, Log,
			TEXT("BlockWidget: RequestRemove broadcast for '%s'"),
			*BlockData.DisplayName.ToString());
	}
}


FReply UBlockWidget::NativeOnMouseButtonDown(
	const FGeometry& InGeometry,
	const FPointerEvent& InMouseEvent)
{
	if (InMouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
	{
		return FReply::Handled().DetectDrag(TakeWidget(), EKeys::LeftMouseButton);
	}

	return FReply::Unhandled();
}

void UBlockWidget::NativeOnDragDetected(
	const FGeometry& InGeometry,
	const FPointerEvent& InMouseEvent,
	UDragDropOperation*& OutOperation)
{
	Super::NativeOnDragDetected(InGeometry, InMouseEvent, OutOperation);

	// Create the operation object that UMG tracks until mouse-up
	UDragDropOperation* DragOp = NewObject<UDragDropOperation>();

	UBlockWidget* DragVisual = CreateWidget<UBlockWidget>(GetWorld(), StaticClass());
	if (DragVisual)
	{
		DragVisual->InitializeBlock(BlockData);
	}

	DragOp->DefaultDragVisual = DragVisual;
	DragOp->Pivot             = EDragPivot::MouseDown; // Visual anchors to click point
	
	DragOp->Payload = this;

	OutOperation = DragOp;

	UE_LOG(LogTemp, Log,
		TEXT("BlockWidget: Drag started for '%s'"),
		*BlockData.DisplayName.ToString());

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(
			-1, 1.5f, FColor::White,
			FString::Printf(TEXT("Dragging: %s"), *BlockData.DisplayName.ToString()));
	}
}

TArray<FRobotInstruction> UBlockWidget::GetInstructions()
{
	TArray<FRobotInstruction> Result;
	Result.Add(GetInstruction());
	return Result;
}

void UBlockWidget::NativeConstruct()
{
	Super::NativeConstruct();
}
