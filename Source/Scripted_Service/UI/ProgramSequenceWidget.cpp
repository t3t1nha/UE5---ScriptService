// Fill out your copyright notice in the Description page of Project Settings.

#include "ProgramSequenceWidget.h"
#include "Components/VerticalBoxSlot.h"

TArray<FRobotInstruction> UProgramSequenceWidget::GetProgram() const
{
	TArray<FRobotInstruction> Program;
	Program.Reserve(SequenceBlocks.Num());

	for (UBlockWidget* Block : SequenceBlocks)
	{
		if (Block)
		{
			Program.Add(Block->GetInstruction());
		}
	}

	UE_LOG(LogTemp, Log,
		TEXT("ProgramSequenceWidget: Built program with %d instruction(s)"),
		Program.Num());

	return Program;
}

void UProgramSequenceWidget::AddBlock(FBlockData BlockData)
{
	UBlockWidget* NewWidget = CreateAndAddBlockWidget(BlockData);
	if (NewWidget)
	{
		// Notify any listeners (e.g. a counter label in the menu)
		OnSequenceChanged.Broadcast();

		UE_LOG(LogTemp, Log,
			TEXT("ProgramSequenceWidget: Added block '%s' (total: %d)"),
			*BlockData.DisplayName.ToString(), SequenceBlocks.Num());
	}
}

void UProgramSequenceWidget::RemoveBlockAtIndex(int32 Index)
{
	if (!SequenceBlocks.IsValidIndex(Index))
	{
		UE_LOG(LogTemp, Warning,
			TEXT("ProgramSequenceWidget: RemoveBlockAtIndex – index %d is out of range "
				 "(sequence has %d block(s))"),
			Index, SequenceBlocks.Num());
		return;
	}

	UBlockWidget* Block = SequenceBlocks[Index];

	// Remove from the internal tracking array
	SequenceBlocks.RemoveAt(Index);

	// Remove the widget from the vertical box
	if (SequenceBox && Block)
	{
		SequenceBox->RemoveChild(Block);
	}

	OnSequenceChanged.Broadcast();

	UE_LOG(LogTemp, Log,
		TEXT("ProgramSequenceWidget: Removed block at index %d (total: %d)"),
		Index, SequenceBlocks.Num());
}

void UProgramSequenceWidget::ClearSequence()
{
	SequenceBlocks.Empty();

	if (SequenceBox)
	{
		SequenceBox->ClearChildren();
	}

	OnSequenceChanged.Broadcast();

	UE_LOG(LogTemp, Log, TEXT("ProgramSequenceWidget: Sequence cleared"));
}

int32 UProgramSequenceWidget::GetBlockCount() const
{
	return SequenceBlocks.Num();
}

UBlockWidget* UProgramSequenceWidget::GetBlockAtIndex(int32 Index) const
{
	if (!SequenceBlocks.IsValidIndex(Index))
	{
		return nullptr;
	}
	return SequenceBlocks[Index];
}

bool UProgramSequenceWidget::NativeOnDrop(
	const FGeometry& InGeometry,
	const FDragDropEvent& InDragDropEvent,
	UDragDropOperation* InOperation)
{
	// Always restore highlight first so the border is clean regardless of outcome
	SetDropHighlight(false);

	if (!InOperation)
	{
		return false;
	}

	// The payload was set in UBlockWidget::NativeOnDragDetected as the source block
	UBlockWidget* DraggedBlock = Cast<UBlockWidget>(InOperation->Payload);
	if (!DraggedBlock)
	{
		UE_LOG(LogTemp, Warning,
			TEXT("ProgramSequenceWidget: Drop rejected – payload is not a UBlockWidget"));
		return false;
	}

	// Create a *copy* of the block data so the palette widget is left untouched
	AddBlock(DraggedBlock->BlockData);

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(
			-1, 2.0f, FColor::Cyan,
			FString::Printf(TEXT("Added: %s"), *DraggedBlock->BlockData.DisplayName.ToString()));
	}

	return true; // Event consumed
}

bool UProgramSequenceWidget::NativeOnDragOver(
	const FGeometry& InGeometry,
	const FDragDropEvent& InDragDropEvent,
	UDragDropOperation* InOperation)
{
	if (!InOperation)
	{
		return false;
	}

	// Only highlight for valid block payloads; ignore other drag types
	if (Cast<UBlockWidget>(InOperation->Payload))
	{
		SetDropHighlight(true);
		return true; // Signal to UMG: "this widget can accept this drag"
	}

	return false;
}

void UProgramSequenceWidget::NativeOnDragLeave(
	const FDragDropEvent& InDragDropEvent,
	UDragDropOperation* InOperation)
{
	SetDropHighlight(false);
}

UBlockWidget* UProgramSequenceWidget::CreateAndAddBlockWidget(const FBlockData& BlockData)
{
    UBlockWidget* NewBlock = nullptr;

    if (BlockData.bIsContainerBlock)
    {
        if (!ContainerBlockWidgetClass)
        {
            UE_LOG(LogTemp, Error,
                TEXT("ProgramSequenceWidget: ContainerBlockWidgetClass is null. "
                     "Assign WBP_ContainerBlock in the Blueprint defaults."));
            return nullptr;
        }

        UContainerBlockWidget* Container =
            CreateWidget<UContainerBlockWidget>(GetOwningPlayer(), ContainerBlockWidgetClass);
        if (!Container) return nullptr;

        // Pass down BlockWidgetClass so the inner drop zone uses the same style
        Container->InitializeContainerBlock(BlockData, BlockWidgetClass);
        NewBlock = Container;
    }
    else
    {
        if (!BlockWidgetClass)
        {
            UE_LOG(LogTemp, Error,
                TEXT("ProgramSequenceWidget: BlockWidgetClass is null."));
            return nullptr;
        }

        UBlockWidget* Plain = CreateWidget<UBlockWidget>(GetOwningPlayer(), BlockWidgetClass);
        if (!Plain) return nullptr;

        Plain->InitializeBlock(BlockData);
        NewBlock = Plain;
    }
	
    NewBlock->OnRemoveRequested.AddDynamic(
        this, &UProgramSequenceWidget::HandleBlockRemoveRequested);

    SequenceBlocks.Add(NewBlock);

    if (SequenceBox)
    {
        UVerticalBoxSlot* BoxSlot = SequenceBox->AddChildToVerticalBox(NewBlock);
        if (BoxSlot)
        {
            BoxSlot->SetPadding(FMargin(0.0f, 0.0f, 0.0f, BlockSpacing));
            BoxSlot->SetHorizontalAlignment(HAlign_Fill);
            BoxSlot->SetVerticalAlignment(VAlign_Top);
        }
    }

    OnSequenceChanged.Broadcast();
    return NewBlock;
}
void UProgramSequenceWidget::HandleBlockRemoveRequested(UBlockWidget* Block)
{
	// Find the block by pointer and remove it
	const int32 Index = SequenceBlocks.Find(Block);
	if (Index != INDEX_NONE)
	{
		RemoveBlockAtIndex(Index);
	}
	else
	{
		UE_LOG(LogTemp, Warning,
			TEXT("ProgramSequenceWidget: HandleBlockRemoveRequested – "
				 "block not found in sequence"));
	}
}

void UProgramSequenceWidget::SetDropHighlight(bool bHighlight)
{
	if (DropZoneBorder)
	{
		DropZoneBorder->SetBrushColor(
			bHighlight ? DragOverHighlightColor : DefaultBorderColor);
	}
}