// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Blueprint/DragDropOperation.h"
#include "Components/TextBlock.h"
#include "Components/Border.h"
#include "Components/EditableTextBox.h"
#include "StructTypes.h"
#include "BlockWidget.generated.h"

/**
 * Fired when a block in the program sequence requests its own removal.
 * Bound by UProgramSequenceWidget so the sequence can remove the block
 * without the block needing a direct reference to the sequence.
 *
 * @param Block  The widget that wants to be removed.
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnBlockRemoveRequested, UBlockWidget*, Block);

/**
 * --- Required Blueprint bindings ---
 *   BlockNameText      (UTextBlock)         Displays the instruction label.
 *   BlockBorder        (UBorder)            Tinted with BlockData.BlockColor.
 *   TableNumberInput   (UEditableTextBox)   Visible only when bHasTableParameter.
 *   WaitValueInput     (UEditableTextBox)   Visible only when bHasWaitParameter.
**/

UCLASS()
class SCRIPTED_SERVICE_API UBlockWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	/** The current instruction values, kept in sync with any input fields */
	UPROPERTY(BlueprintReadWrite, Category = "Block")
	FRobotInstruction Instruction;

	/**
	 * Definition data for this block (type, colour, display name, flags)
	 */
	UPROPERTY(BlueprintReadWrite, Category = "Block")
	FBlockData BlockData;

	/** Label showing the readable instruction name */
	UPROPERTY(meta = (BindWidget))
	UTextBlock* BlockNameText;

	/**
	 * The coloured background border
	 */
	UPROPERTY(meta = (BindWidget))
	UBorder* BlockBorder;

	/**
	 * Input field for the target table number.
	 * Collapsed automatically when BlockData.bHasTableParameter is false.
	 */
	UPROPERTY(meta = (BindWidget), BlueprintReadWrite)
	UEditableTextBox* TableNumberInput;

	/**
	 * Input field for the wait duration.
	 * Collapsed automatically when BlockData.bHasWaitParameter is false.
	 */
	UPROPERTY(meta = (BindWidget), BlueprintReadWrite)
	UEditableTextBox* WaitValueInput;

	/**
	 * Broadcast when the block requests removal from the program sequence
	 */
	UPROPERTY(BlueprintAssignable, Category = "Block|Events")
	FOnBlockRemoveRequested OnRemoveRequested;

	/**
	 * Initialises the block with the given data and refreshes all visuals.
	 *
	 * @param InBlockData  The block definition to apply.
	 */
	UFUNCTION(BlueprintCallable, Category = "Block")
	void InitializeBlock(FBlockData InBlockData);

	/**
	 * Reads the current values from any visible input fields and returns the
	 * fully populated FRobotInstruction.
	 *
	 * @return  The instruction with table number / wait value filled in.
	 */
	UFUNCTION(BlueprintCallable, Category = "Block")
	FRobotInstruction GetInstruction();

	/**
	 * Re-applies colours, text, and parameter visibility from the current BlockData
	 */
	UFUNCTION(BlueprintCallable, Category = "Block")
	void UpdateVisuals();

	/**
	 * Fires OnRemoveRequested with this widget as the argument.
	 */
	UFUNCTION(BlueprintCallable, Category = "Block")
	void RequestRemove();

	/**
	 * Detects the start of a left-mouse drag on this widget.
	 * Returns a Handled reply that begins drag detection;
	 */
	virtual FReply NativeOnMouseButtonDown(
		const FGeometry& InGeometry,
		const FPointerEvent& InMouseEvent) override;

	/**
	 * Creates the UDragDropOperation when the drag threshold is crossed
	 * DefaultDragVisual : a fresh UBlockWidget clone for cursor display.
	 * Payload           : this widget (so the drop target can read BlockData).
	 */
	virtual void NativeOnDragDetected(
		const FGeometry& InGeometry,
		const FPointerEvent& InMouseEvent,
		UDragDropOperation*& OutOperation) override;

	UFUNCTION(BlueprintCallable, Category = "Block")
	virtual TArray<FRobotInstruction> GetInstructions();
protected:

	virtual void NativeConstruct() override;
};


