// ContainerBlockWidget.h
#pragma once

#include "CoreMinimal.h"
#include "BlockWidget.h"
#include "InnerDropZoneWidget.h"
#include "Components/TextBlock.h"
#include "Components/EditableTextBox.h"
#include "ContainerBlockWidget.generated.h"

/**
 * 
 */
UCLASS()
class SCRIPTED_SERVICE_API UContainerBlockWidget : public UBlockWidget
{
    GENERATED_BODY()

public:
    /** The inner body drop-zone.  Bound by name in the Blueprint layout. */
    UPROPERTY(meta = (BindWidget))
    UInnerDropZoneWidget* InnerDropZone;

    /**
     * Input field for the loop count (RepeatLoop only).
     * Shown / hidden automatically by UpdateContainerVisuals().
     */
    UPROPERTY(meta = (BindWidgetOptional))
    UEditableTextBox* LoopCountInput;

    /**
     * Footer text block (e.g. "END IF", "END LOOP").
     * Set automatically by UpdateContainerVisuals().
     */
    UPROPERTY(meta = (BindWidgetOptional))
    UTextBlock* FooterLabel;

    /**
     * Initialise this C-block with the given block data and the widget class
     * to use for inner blocks.
     *
     * @param InBlockData        Block definition (type, colour, label, flags).
     * @param InBlockWidgetClass Widget class passed down to InnerDropZone so
     *                           inner blocks look visually consistent.
     */
    UFUNCTION(BlueprintCallable, Category = "ContainerBlock")
    void InitializeContainerBlock(FBlockData InBlockData,
                                  TSubclassOf<UBlockWidget> InBlockWidgetClass);

    /**
     * Emits the flattened bytecode for this C-block:
     *   1. Header instruction  (IfTableHasOrder / IfCarryingDish / RepeatLoop / etc.)
     *   2. All inner-body instructions  (from InnerDropZone)
     *   3. EndBlock marker
     *
     * @return  Flat TArray for ARobotCharacter::LoadProgram().
     */
    virtual TArray<FRobotInstruction> GetInstructions() override;

private:

    /** Builds and returns just the header instruction from current widget state */
    FRobotInstruction BuildHeaderInstruction() const;

    /** Shows/hides LoopCountInput and sets FooterLabel text */
    void UpdateContainerVisuals();
};