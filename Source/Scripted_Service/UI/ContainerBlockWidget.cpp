// ContainerBlockWidget.cpp
#include "ContainerBlockWidget.h"

void UContainerBlockWidget::InitializeContainerBlock(
    FBlockData InBlockData,
    TSubclassOf<UBlockWidget> InBlockWidgetClass)
{
    // Run the base class initialisation (sets BlockData, default Instruction,
    // and updates BlockNameText / BlockBorder colour via UpdateVisuals())
    InitializeBlock(InBlockData);

    // Propagate the block widget class to the inner drop zone so both the outer
    // sequence and the inner body share the same visual widget style.
    if (InnerDropZone)
    {
        InnerDropZone->BlockWidgetClass = InBlockWidgetClass;
    }

    // Apply container-specific visibility (loop count field, footer text)
    UpdateContainerVisuals();
}

TArray<FRobotInstruction> UContainerBlockWidget::GetInstructions()
{
    TArray<FRobotInstruction> Result;

    Result.Add(BuildHeaderInstruction());

    if (InnerDropZone)
    {
        Result.Append(InnerDropZone->GetInnerInstructions());
    }

    FRobotInstruction EndInstr;
    EndInstr.InstructionType = EInstructionType::EndBlock;
    Result.Add(EndInstr);

    UE_LOG(LogTemp, Log,
        TEXT("ContainerBlockWidget: %s compiled to %d instruction(s)"),
        *BlockData.DisplayName.ToString(), Result.Num());

    return Result;
}

FRobotInstruction UContainerBlockWidget::BuildHeaderInstruction() const
{
    FRobotInstruction Header;
    Header.InstructionType = BlockData.InstructionType;

    // Read the table number from the inherited TableNumberInput field (if shown)
    if (TableNumberInput &&
        TableNumberInput->GetVisibility() == ESlateVisibility::Visible)
    {
        Header.TargetTableNumber =
            FCString::Atoi(*TableNumberInput->GetText().ToString());
    }

    // Read the loop count from the LoopCountInput field (if shown)
    if (LoopCountInput &&
        LoopCountInput->GetVisibility() == ESlateVisibility::Visible)
    {
        // Clamp to at least 1 so a RepeatLoop(0) doesn't silently do nothing
        Header.LoopCount =
            FMath::Max(1, FCString::Atoi(*LoopCountInput->GetText().ToString()));
    }

    return Header;
}

void UContainerBlockWidget::UpdateContainerVisuals()
{
    // Show loop count input only for the RepeatLoop block type
    if (LoopCountInput)
    {
        bool bShowLoopCount = BlockData.bHasLoopCountParameter;
        LoopCountInput->SetVisibility(
            bShowLoopCount ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);

        if (bShowLoopCount)
        {
            LoopCountInput->SetText(FText::FromString(TEXT("3")));
        }
    }

    // Set footer text based on block type
    if (FooterLabel)
    {
        const EInstructionType Type = BlockData.InstructionType;

        if (Type == EInstructionType::RepeatLoop || Type == EInstructionType::LoopForever)
        {
            FooterLabel->SetText(FText::FromString(TEXT("END LOOP")));
        }
        else
        {
            FooterLabel->SetText(FText::FromString(TEXT("END IF")));
        }
    }
}