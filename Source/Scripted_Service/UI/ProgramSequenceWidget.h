// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Blueprint/DragDropOperation.h"
#include "ContainerBlockWidget.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Components/Border.h"
#include "BlockWidget.h"
#include "StructTypes.h"
#include "ProgramSequenceWidget.generated.h"

/**
 * Fired whenever blocks are added, removed, or the sequence is cleared.
 * Bind this in Blueprint (e.g. to refresh an instruction counter label).
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnSequenceChanged);

UCLASS()
class SCRIPTED_SERVICE_API UProgramSequenceWidget : public UUserWidget
{
	GENERATED_BODY()

public:

	/**
	 * The vertical list that holds program blocks in execution order.
	 * Must exist in the Blueprint layout with this exact name.
	 */
	UPROPERTY(meta = (BindWidget))
	UVerticalBox* SequenceBox;

	/**
	 * Optional background border.  Its brush colour is lerped to
	 * DragOverHighlightColor while a compatible block hovers above this widget,
	 * then restored to DefaultBorderColor when the cursor leaves.
	 * Omit from the layout if you do not want this effect.
	 */
	UPROPERTY(meta = (BindWidgetOptional))
	UBorder* DropZoneBorder;

	/**
	 * The UBlockWidget subclass to instantiate for every new sequence entry.
	 * Set this to the same WBP_BlockWidget used in the palette so that
	 * visual style is consistent.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sequence|Config")
	TSubclassOf<UBlockWidget> BlockWidgetClass;

	/**
	 * Colour applied to DropZoneBorder's brush while a block is being dragged
	 * over this widget.  Alpha controls the intensity of the highlight.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sequence|Config")
	FLinearColor DragOverHighlightColor = FLinearColor(0.3f, 0.8f, 1.0f, 0.35f);

	/**
	 * Resting colour of DropZoneBorder when nothing is being dragged.
	 * Should match your panel background colour.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sequence|Config")
	FLinearColor DefaultBorderColor = FLinearColor(0.08f, 0.08f, 0.08f, 0.6f);

	/** Vertical padding (in Slate units) inserted between each block entry. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sequence|Config",
		meta = (ClampMin = "0.0"))
	float BlockSpacing = 4.0f;

	/**
 * The UContainerBlockWidget subclass (WBP_ContainerBlock) to spawn when a
 * block with bIsContainerBlock == true is dropped onto the sequence.
 * Must be set in the Blueprint defaults of WBP_ProgramSequence.
 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sequence|Config")
	TSubclassOf<UContainerBlockWidget> ContainerBlockWidgetClass;

	/**
	 * Collects a FRobotInstruction from every block widget (top to bottom)
	 * and returns them as an ordered TArray, ready for
	 * ARobotCharacter::LoadProgram().
	 *
	 * @return  Ordered array of robot instructions; empty if sequence is empty.
	 */
	UFUNCTION(BlueprintCallable, Category = "Sequence")
	TArray<FRobotInstruction> GetProgram() const;

	/**
	 * Instantiates a new UBlockWidget from the supplied data and appends it
	 * to the bottom of the sequence.
	 *
	 * @param BlockData  The block definition (type, colour, parameters) to add.
	 */
	UFUNCTION(BlueprintCallable, Category = "Sequence")
	void AddBlock(FBlockData BlockData);

	/**
	 * Removes the block at the given zero-based index.
	 * Logs a warning and does nothing if the index is out of range.
	 *
	 * @param Index  Zero-based position of the block to remove.
	 */
	UFUNCTION(BlueprintCallable, Category = "Sequence")
	void RemoveBlockAtIndex(int32 Index);

	/** Removes every block and clears the SequenceBox. */
	UFUNCTION(BlueprintCallable, Category = "Sequence")
	void ClearSequence();

	/** Returns the number of blocks currently in the sequence. */
	UFUNCTION(BlueprintPure, Category = "Sequence")
	int32 GetBlockCount() const;

	/**
	 * Returns the block widget at the given index, or nullptr if out of range.
	 * Useful for Blueprint logic that needs to inspect a specific step.
	 *
	 * @param Index  Zero-based index.
	 */
	UFUNCTION(BlueprintPure, Category = "Sequence")
	UBlockWidget* GetBlockAtIndex(int32 Index) const;

	/**
	 * Fired every time the sequence is modified (block added, removed, or
	 * cleared).  Bind in Blueprint to refresh counters or enable/disable the
	 * Run button.
	 */
	UPROPERTY(BlueprintAssignable, Category = "Sequence|Events")
	FOnSequenceChanged OnSequenceChanged;

protected:
	/**
	 * Called when a dragged object is released over this widget.
	 * Reads the UBlockWidget payload, creates a copy of its data in the
	 * sequence, and returns true to consume the event.
	 *
	 * @param InOperation  The active drag-drop operation (payload = UBlockWidget*).
	 * @return true if the drop was accepted; false if the payload was invalid.
	 */
	virtual bool NativeOnDrop(
		const FGeometry& InGeometry,
		const FDragDropEvent& InDragDropEvent,
		UDragDropOperation* InOperation) override;

	/**
	 * Called every frame while a dragged object moves over this widget.
	 * Tints DropZoneBorder to signal that a drop is possible here.
	 *
	 * @return true to tell UMG this widget accepts the dragged payload.
	 */
	virtual bool NativeOnDragOver(
		const FGeometry& InGeometry,
		const FDragDropEvent& InDragDropEvent,
		UDragDropOperation* InOperation) override;

	/**
	 * Called when the cursor leaves this widget during a drag.
	 * Restores DropZoneBorder to its default colour.
	 */
	virtual void NativeOnDragLeave(
		const FDragDropEvent& InDragDropEvent,
		UDragDropOperation* InOperation) override;

private:

	/**
	 * Internal ordered list of block widgets that mirrors SequenceBox's
	 * children.  Always kept in sync with SequenceBox.
	 */
	UPROPERTY()
	TArray<UBlockWidget*> SequenceBlocks;

	/**
	 * Creates a new UBlockWidget, initialises it with BlockData, binds its
	 * OnRemoveRequested delegate, and appends it to SequenceBox.
	 *
	 * @param BlockData  Data used to initialise the new block widget.
	 * @return           Pointer to the new widget, or nullptr on failure.
	 */
	UBlockWidget* CreateAndAddBlockWidget(const FBlockData& BlockData);

	/**
	 * Bound to each sequence block's OnRemoveRequested delegate.
	 * Locates the block by pointer and removes it from the sequence.
	 *
	 * @param Block  The block that requested its own removal.
	 */
	UFUNCTION()
	void HandleBlockRemoveRequested(UBlockWidget* Block);

	/**
	 * Applies or removes the visual drag-over highlight from DropZoneBorder.
	 *
	 * @param bHighlight  true to show the highlight; false to restore default.
	 */
	void SetDropHighlight(bool bHighlight);
};