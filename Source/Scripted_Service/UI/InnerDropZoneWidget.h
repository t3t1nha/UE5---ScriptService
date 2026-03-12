// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Blueprint/DragDropOperation.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Components/Border.h"
#include "BlockWidget.h"
#include "StructTypes.h"
#include "InnerDropZoneWidget.generated.h"


UCLASS()
class SCRIPTED_SERVICE_API UInnerDropZoneWidget : public UUserWidget
{
	GENERATED_BODY()
    
	public:

    /** Vertical list that holds the inner body blocks */
    UPROPERTY(meta = (BindWidget))
    UVerticalBox* InnerSequenceBox;

    /** Optional highlight border shown during drag-over */
    UPROPERTY(meta = (BindWidgetOptional))
    UBorder* InnerDropBorder;


    /** The UBlockWidget subclass to create when a block is dropped here. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
    TSubclassOf<UBlockWidget> BlockWidgetClass;

    /** Vertical gap between blocks */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config",
        meta = (ClampMin = "0.0"))
    float BlockSpacing = 4.0f;

    /** Tint applied while a block is being dragged over this zone */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
    FLinearColor DragOverColor = FLinearColor(0.3f, 0.8f, 1.0f, 0.35f);

    /** Resting colour of InnerDropBorder */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
    FLinearColor DefaultBorderColor = FLinearColor(0.06f, 0.06f, 0.06f, 0.5f);

    /**
     * Collects instructions from every inner block (top to bottom) and returns
     * them as a flat TArray.  Called by ContainerBlockWidget::GetInstructions().
     */
    UFUNCTION(BlueprintCallable, Category = "InnerZone")
    TArray<FRobotInstruction> GetInnerInstructions() const;

    /** Removes all inner blocks and clears the vertical box */
    UFUNCTION(BlueprintCallable, Category = "InnerZone")
    void ClearInnerBlocks();

    /** Returns the number of blocks currently in the inner zone */
    UFUNCTION(BlueprintPure, Category = "InnerZone")
    int32 GetInnerBlockCount() const { return InnerBlocks.Num(); }

protected:

    virtual bool NativeOnDrop(
        const FGeometry& InGeometry,
        const FDragDropEvent& InDragDropEvent,
        UDragDropOperation* InOperation) override;

    virtual bool NativeOnDragOver(
        const FGeometry& InGeometry,
        const FDragDropEvent& InDragDropEvent,
        UDragDropOperation* InOperation) override;

    virtual void NativeOnDragLeave(
        const FDragDropEvent& InDragDropEvent,
        UDragDropOperation* InOperation) override;

    virtual FReply NativeOnMouseButtonDown(
        const FGeometry& InGeometry,
        const FPointerEvent& InMouseEvent) override;

private:

    /** Tracks inner blocks in order — mirrors InnerSequenceBox children */
    UPROPERTY()
    TArray<UBlockWidget*> InnerBlocks;

    /** Creates a BlockWidget, initialises it, and adds it to the inner list */
    UBlockWidget* CreateAndAddInnerBlock(const FBlockData& BlockData);

    /** Removes a specific block by pointer */
    UFUNCTION()
    void HandleInnerBlockRemoveRequested(UBlockWidget* Block);

    void SetDropHighlight(bool bHighlight);
};
