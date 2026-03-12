// Fill out your copyright notice in the Description page of Project Settings.


#include "InnerDropZoneWidget.h"
#include "Components/VerticalBoxSlot.h"

TArray<FRobotInstruction> UInnerDropZoneWidget::GetInnerInstructions() const
{
    TArray<FRobotInstruction> Result;
    for (UBlockWidget* Block : InnerBlocks)
    {
        if (Block)
        {
            // GetInstructions() is virtual — plain blocks return one instruction
            Result.Append(Block->GetInstructions());
        }
    }
    return Result;
}

void UInnerDropZoneWidget::ClearInnerBlocks()
{
    InnerBlocks.Empty();
    if (InnerSequenceBox) InnerSequenceBox->ClearChildren();
}

bool UInnerDropZoneWidget::NativeOnDrop(
    const FGeometry& InGeometry,
    const FDragDropEvent& InDragDropEvent,
    UDragDropOperation* InOperation)
{
    SetDropHighlight(false);

    if (!InOperation) return false;

    UBlockWidget* DraggedBlock = Cast<UBlockWidget>(InOperation->Payload);
    if (!DraggedBlock) return false;

    // Reject container blocks — inner zones only accept flat action blocks.
    // This keeps the nesting depth at exactly one level.
    if (DraggedBlock->BlockData.bIsContainerBlock)
    {
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(
                -1, 2.0f, FColor::Orange,
                TEXT("Cannot nest If/Loop blocks inside another block"));
        }
        return false;
    }

    UBlockWidget* NewBlock = CreateAndAddInnerBlock(DraggedBlock->BlockData);
    if (!NewBlock) return false;

    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(
            -1, 2.0f, FColor::Cyan,
            FString::Printf(TEXT("Inner: Added %s"), *DraggedBlock->BlockData.DisplayName.ToString()));
    }

    return true;
}

bool UInnerDropZoneWidget::NativeOnDragOver(
    const FGeometry& InGeometry,
    const FDragDropEvent& InDragDropEvent,
    UDragDropOperation* InOperation)
{
    if (!InOperation) return false;

    UBlockWidget* Payload = Cast<UBlockWidget>(InOperation->Payload);
    // Only highlight if the dragged block is a plain (non-container) block
    if (Payload && !Payload->BlockData.bIsContainerBlock)
    {
        SetDropHighlight(true);
        return true;
    }
    return false;
}

void UInnerDropZoneWidget::NativeOnDragLeave(
    const FDragDropEvent& InDragDropEvent,
    UDragDropOperation* InOperation)
{
    SetDropHighlight(false);
}

FReply UInnerDropZoneWidget::NativeOnMouseButtonDown(
    const FGeometry& InGeometry,
    const FPointerEvent& InMouseEvent)
{
    // Consume the event so it never reaches ContainerBlockWidget::NativeOnMouseButtonDown,
    // which would start dragging the entire C-block when the player clicks inside the body.
    return FReply::Handled();
}

UBlockWidget* UInnerDropZoneWidget::CreateAndAddInnerBlock(const FBlockData& BlockData)
{
    if (!BlockWidgetClass)
    {
        UE_LOG(LogTemp, Error,
            TEXT("InnerDropZoneWidget: BlockWidgetClass is null. "
                 "Set it via ContainerBlockWidget::InitializeContainerBlock()."));
        return nullptr;
    }

    UBlockWidget* NewBlock = CreateWidget<UBlockWidget>(GetOwningPlayer(), BlockWidgetClass);
    if (!NewBlock) return nullptr;

    NewBlock->InitializeBlock(BlockData);

    // Bind the remove delegate so the player can delete inner blocks
    NewBlock->OnRemoveRequested.AddDynamic(
        this, &UInnerDropZoneWidget::HandleInnerBlockRemoveRequested);

    InnerBlocks.Add(NewBlock);

    if (InnerSequenceBox)
    {
        UVerticalBoxSlot* BoxSlot = InnerSequenceBox->AddChildToVerticalBox(NewBlock);
        if (BoxSlot)
        {
            BoxSlot->SetPadding(FMargin(0.0f, 0.0f, 0.0f, BlockSpacing));
            BoxSlot->SetHorizontalAlignment(HAlign_Fill);
            BoxSlot->SetVerticalAlignment(VAlign_Top);
        }
    }

    return NewBlock;
}

void UInnerDropZoneWidget::HandleInnerBlockRemoveRequested(UBlockWidget* Block)
{
    const int32 Index = InnerBlocks.Find(Block);
    if (Index == INDEX_NONE) return;

    InnerBlocks.RemoveAt(Index);
    if (InnerSequenceBox) InnerSequenceBox->RemoveChild(Block);
}

void UInnerDropZoneWidget::SetDropHighlight(bool bHighlight)
{
    if (InnerDropBorder)
    {
        InnerDropBorder->SetBrushColor(bHighlight ? DragOverColor : DefaultBorderColor);
    }
}