// Fill out your copyright notice in the Description page of Project Settings.

#include "ProgrammingMenu.h"
#include "BlockLibrary.h"
#include "EngineUtils.h"
#include "Components/WrapBoxSlot.h"

// ─────────────────────────────────────────────────────────────────────────────
// UUserWidget Overrides
// ─────────────────────────────────────────────────────────────────────────────

void UProgrammingMenu::NativeConstruct()
{
	Super::NativeConstruct();

	// Fill the palette immediately so blocks are ready before the player opens
	// the menu for the first time.
	PopulatePalette();
}

// ─────────────────────────────────────────────────────────────────────────────
// Menu Visibility
// ─────────────────────────────────────────────────────────────────────────────

void UProgrammingMenu::ShowMenu_Implementation()
{
	SetVisibility(ESlateVisibility::Visible);
	bIsOpen = true;

	UE_LOG(LogTemp, Log, TEXT("ProgrammingMenu: Menu opened"));
}

void UProgrammingMenu::HideMenu_Implementation()
{
	SetVisibility(ESlateVisibility::Collapsed);
	bIsOpen = false;

	UE_LOG(LogTemp, Log, TEXT("ProgrammingMenu: Menu closed"));
}

// ─────────────────────────────────────────────────────────────────────────────
// Robot Reference
// ─────────────────────────────────────────────────────────────────────────────

void UProgrammingMenu::SetTargetRobot(ARobotCharacter* InRobot)
{
	TargetRobot = InRobot;

	if (TargetRobot)
	{
		UE_LOG(LogTemp, Log,
			TEXT("ProgrammingMenu: Target robot set to '%s'"),
			*TargetRobot->GetName());

		SetStatusMessage(
			FString::Printf(TEXT("Robot: %s"), *TargetRobot->GetName()));
	}
	else
	{
		UE_LOG(LogTemp, Warning,
			TEXT("ProgrammingMenu: Target robot cleared (set to null)"));

		SetStatusMessage(TEXT("No robot selected"), FLinearColor::Yellow);
	}
}

// ─────────────────────────────────────────────────────────────────────────────
// Palette
// ─────────────────────────────────────────────────────────────────────────────

void UProgrammingMenu::PopulatePalette()
{
	if (!PaletteBox)
	{
		UE_LOG(LogTemp, Error,
			TEXT("ProgrammingMenu: PaletteBox is null. "
				 "Add a WrapBox named 'PaletteBox' to the Blueprint layout."));
		return;
	}

	if (!BlockWidgetClass)
	{
		UE_LOG(LogTemp, Error,
			TEXT("ProgrammingMenu: BlockWidgetClass is not set. "
				 "Assign a UBlockWidget subclass in the Blueprint defaults."));
		return;
	}

	// Clear any previously created palette entries before rebuilding
	PaletteBox->ClearChildren();

	// Fetch the canonical list of available instruction blocks
	const TArray<FBlockData> DefaultBlocks = UBlockLibrary::GetDefaultBlocks();

	for (const FBlockData& BlockData : DefaultBlocks)
	{
		UBlockWidget* PaletteBlock =
			CreateWidget<UBlockWidget>(GetOwningPlayer(), BlockWidgetClass);

		if (!PaletteBlock)
		{
			UE_LOG(LogTemp, Error,
				TEXT("ProgrammingMenu: Failed to create palette block for '%s'"),
				*BlockData.DisplayName.ToString());
			continue;
		}

		// Apply colours, label, and parameter visibility
		PaletteBlock->InitializeBlock(BlockData);

		// Add to the wrap-box; each block gets a small margin for breathing room
		UWrapBoxSlot* PaletteSlot = PaletteBox->AddChildToWrapBox(PaletteBlock);
		if (PaletteSlot)
		{
			PaletteSlot->SetPadding(FMargin(4.0f));
		}
	}

	UE_LOG(LogTemp, Log,
		TEXT("ProgrammingMenu: Palette populated with %d block(s)"),
		DefaultBlocks.Num());
}

// ─────────────────────────────────────────────────────────────────────────────
// Program Controls
// ─────────────────────────────────────────────────────────────────────────────

void UProgrammingMenu::RunProgram()
{
	// Validate sequence widget
	if (!SequenceWidget)
	{
		UE_LOG(LogTemp, Error,
			TEXT("ProgrammingMenu: RunProgram – SequenceWidget is null"));
		SetStatusMessage(TEXT("ERROR: Sequence panel missing"), FLinearColor::Red);
		return;
	}

	// Guard against empty programs
	if (SequenceWidget->GetBlockCount() == 0)
	{
		UE_LOG(LogTemp, Warning,
			TEXT("ProgrammingMenu: RunProgram – sequence is empty, nothing to run"));
		SetStatusMessage(TEXT("Add some blocks first!"), FLinearColor::Yellow);

		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(
				-1, 3.0f, FColor::Yellow, TEXT("Program is empty!"));
		}
		return;
	}

	// Resolve robot: use assigned reference, fall back to level search
	if (!TargetRobot)
	{
		UE_LOG(LogTemp, Warning,
			TEXT("ProgrammingMenu: No TargetRobot set – searching level..."));
		TargetRobot = FindRobotInLevel();
	}

	if (!TargetRobot)
	{
		UE_LOG(LogTemp, Error,
			TEXT("ProgrammingMenu: RunProgram – no robot found in level"));
		SetStatusMessage(TEXT("ERROR: No robot found!"), FLinearColor::Red);

		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(
				-1, 3.0f, FColor::Red, TEXT("No robot found in level!"));
		}
		return;
	}

	// Build the instruction array from the sequence widget
	TArray<FRobotInstruction> Program = SequenceWidget->GetProgram();

	UE_LOG(LogTemp, Log,
		TEXT("ProgrammingMenu: Sending %d instruction(s) to robot '%s'"),
		Program.Num(), *TargetRobot->GetName());

	// Load and execute
	TargetRobot->LoadProgram(Program);
	TargetRobot->ExecuteProgram();

	SetStatusMessage(
		FString::Printf(TEXT("Running %d instruction(s)…"), Program.Num()),
		FLinearColor(0.3f, 1.0f, 0.3f));

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(
			-1, 3.0f, FColor::Green,
			FString::Printf(TEXT("Robot running %d instruction(s)"), Program.Num()));
	}
}

void UProgrammingMenu::StopProgram()
{
	if (!TargetRobot)
	{
		// Try to find one; if the player presses Stop without using Run first
		TargetRobot = FindRobotInLevel();
	}

	if (!TargetRobot)
	{
		UE_LOG(LogTemp, Warning,
			TEXT("ProgrammingMenu: StopProgram – no robot to stop"));
		return;
	}

	TargetRobot->StopProgram();

	SetStatusMessage(TEXT("Stopped"), FLinearColor::Yellow);

	UE_LOG(LogTemp, Log,
		TEXT("ProgrammingMenu: Program stopped on robot '%s'"),
		*TargetRobot->GetName());

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(
			-1, 2.0f, FColor::Yellow, TEXT("Program stopped"));
	}
}

void UProgrammingMenu::ClearProgram()
{
	if (SequenceWidget)
	{
		SequenceWidget->ClearSequence();
	}

	SetStatusMessage(TEXT("Sequence cleared"));

	UE_LOG(LogTemp, Log, TEXT("ProgrammingMenu: Sequence cleared by player"));

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(
			-1, 2.0f, FColor::White, TEXT("Sequence cleared"));
	}
}

// ─────────────────────────────────────────────────────────────────────────────
// Private Helpers
// ─────────────────────────────────────────────────────────────────────────────

ARobotCharacter* UProgrammingMenu::FindRobotInLevel() const
{
	if (!GetWorld())
	{
		return nullptr;
	}

	// Return the first robot found; the game currently has one robot per level
	for (TActorIterator<ARobotCharacter> It(GetWorld()); It; ++It)
	{
		return *It;
	}

	return nullptr;
}

void UProgrammingMenu::SetStatusMessage(const FString& Message, FLinearColor Color)
{
	if (StatusText)
	{
		StatusText->SetText(FText::FromString(Message));
		StatusText->SetColorAndOpacity(FSlateColor(Color));
	}
}