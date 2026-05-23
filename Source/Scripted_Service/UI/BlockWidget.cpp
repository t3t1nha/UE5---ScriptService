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
	if (SaveToSlotInput && SaveToSlotInput->GetVisibility() == ESlateVisibility::Visible)
	{
		const FString Text  = SaveToSlotInput->GetText().ToString().TrimStartAndEnd();
		const int32   Index = FCString::Atoi(*Text);

		if (!Text.IsEmpty() && Index >= 0 && Index <= 3)
		{
			Instruction.bSaveToSlot      = true;
			Instruction.SaveToSlotIndex  = Index;
		}
		else
		{
			Instruction.bSaveToSlot = false;
		}
	}

	if (ReadFromSlotInput && ReadFromSlotInput->GetVisibility() == ESlateVisibility::Visible)
	{
		const FString Text  = ReadFromSlotInput->GetText().ToString().TrimStartAndEnd();
		const int32   Index = FCString::Atoi(*Text);

		if (!Text.IsEmpty() && Index >= 0 && Index <= 3)
		{
			Instruction.bReadTableFromSlot = true;
			Instruction.ReadFromSlotIndex  = Index;
		}
		else
		{
			Instruction.bReadTableFromSlot = false;
		}
	}

	if (SlotIndexInput && SlotIndexInput->GetVisibility() == ESlateVisibility::Visible)
	{
		Instruction.SlotIndex = FMath::Clamp(
			FCString::Atoi(*SlotIndexInput->GetText().ToString()), 0, 3);
	}

	if (SlotValueInput && SlotValueInput->GetVisibility() == ESlateVisibility::Visible)
	{
		Instruction.SlotValue = FCString::Atoi(*SlotValueInput->GetText().ToString());
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
	
	if (SaveToSlotInput)
	{
		SaveToSlotInput->SetVisibility(
			BlockData.bCanSaveToSlot
				? ESlateVisibility::Visible
				: ESlateVisibility::Collapsed);
	}

	if (ReadFromSlotInput)
	{
		ReadFromSlotInput->SetVisibility(
			BlockData.bCanReadTableFromSlot
				? ESlateVisibility::Visible
				: ESlateVisibility::Collapsed);
	}

	if (SlotIndexInput)
	{
		SlotIndexInput->SetVisibility(
			BlockData.bHasSlotIndexParameter
				? ESlateVisibility::Visible
				: ESlateVisibility::Collapsed);

		if (BlockData.bHasSlotIndexParameter)
		{
			SlotIndexInput->SetText(FText::FromString(TEXT("0")));
		}
	}

	if (SlotValueInput)
	{
		SlotValueInput->SetVisibility(
			BlockData.bHasSlotValueParameter
				? ESlateVisibility::Visible
				: ESlateVisibility::Collapsed);

		if (BlockData.bHasSlotValueParameter)
		{
			SlotValueInput->SetText(FText::FromString(TEXT("0")));
		}
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
	if (InMouseEvent.GetEffectingButton() == EKeys::RightMouseButton)
	{
		RequestRemove();

		return FReply::Handled();
	}
	
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

	UBlockWidget* DragVisual = CreateWidget<UBlockWidget>(GetWorld(), GetClass());
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
