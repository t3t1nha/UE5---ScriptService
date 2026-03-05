// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/WrapBox.h"
#include "Components/TextBlock.h"
#include "ProgramSequenceWidget.h"
#include "BlockWidget.h"
#include "RobotCharacter.h"
#include "ProgrammingMenu.generated.h"

/**
 * UProgrammingMenu
 *
 * The top-level HUD widget for the robot programming system.
 * It contains:
 *   - A block palette  (WrapBox)              showing all draggable instructions.
 *   - A program sequence (UProgramSequenceWidget) as the drag-and-drop target.
 *   - Run / Stop / Clear controls callable from Blueprint button bindings.
 *
 * --- Required Blueprint bindings ---
 *   SequenceWidget   (UProgramSequenceWidget)  The drop zone / program builder.
 *   PaletteBox       (UWrapBox)               Holds the draggable palette blocks.
 *
 * --- Optional Blueprint bindings ---
 *   StatusText       (UTextBlock)             Displays runtime status messages.
 *
 * --- Blueprint setup ---
 *   1. Create WBP_ProgrammingMenu based on this class.
 *   2. Add WBP_ProgramSequence (UProgramSequenceWidget) named "SequenceWidget".
 *   3. Add a WrapBox named "PaletteBox" where palette blocks will be spawned.
 *   4. Assign BlockWidgetClass to your WBP_BlockWidget.
 *   5. Wire Run/Stop/Clear buttons to RunProgram(), StopProgram(), ClearProgram().
 *   6. The menu auto-populates the palette in NativeConstruct.
 *
 * --- Robot wiring ---
 *   Call SetTargetRobot() before calling RunProgram().  You can do this from
 *   Player_Character when it opens the menu next to a robot, or from a
 *   game-mode / level Blueprint when the player enters an interaction trigger.
 */
UCLASS()
class SCRIPTED_SERVICE_API UProgrammingMenu : public UUserWidget
{
	GENERATED_BODY()

public:

	// ─── Bound Widgets ───────────────────────────────────────────────────────

	/**
	 * The drag-and-drop sequence panel.  Must be a WBP_ProgramSequence
	 * (UProgramSequenceWidget) in the Blueprint layout with this exact name.
	 */
	UPROPERTY(meta = (BindWidget))
	UProgramSequenceWidget* SequenceWidget;

	/**
	 * Wrap-box that contains the palette block widgets.
	 * Populated at construction by PopulatePalette() using BlockLibrary defaults.
	 */
	UPROPERTY(meta = (BindWidget))
	UWrapBox* PaletteBox;

	/**
	 * Optional text block used to display status messages such as
	 * "Running…", "Program complete!", or error descriptions.
	 * Leave unbound if you handle status feedback another way.
	 */
	UPROPERTY(meta = (BindWidgetOptional))
	UTextBlock* StatusText;

	// ─── Configuration ───────────────────────────────────────────────────────

	/**
	 * The UBlockWidget subclass to instantiate for palette entries.
	 * Must match the class used inside WBP_ProgramSequence so that dragged
	 * blocks look and behave consistently.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Menu|Config")
	TSubclassOf<UBlockWidget> BlockWidgetClass;

	// ─── Robot Reference ─────────────────────────────────────────────────────

	/**
	 * The robot that will receive and execute programs built in this menu.
	 * Set via SetTargetRobot() before the player clicks Run.
	 * If null, RunProgram() will attempt to auto-find a robot in the level.
	 */
	UPROPERTY(BlueprintReadOnly, Category = "Menu|Robot")
	ARobotCharacter* TargetRobot;

	/**
	 * Assigns the robot that programs will be sent to.
	 * Call this from Player_Character::ToggleMenu() or an interaction trigger.
	 *
	 * @param InRobot  The robot to program; pass nullptr to clear the reference.
	 */
	UFUNCTION(BlueprintCallable, Category = "Menu|Robot")
	void SetTargetRobot(ARobotCharacter* InRobot);

	// ─── Menu Visibility ─────────────────────────────────────────────────────

	/**
	 * Makes the menu visible and triggers any open animation.
	 * Override in Blueprint to add slide-in / fade effects.
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Programming Menu")
	void ShowMenu();
	virtual void ShowMenu_Implementation();

	/**
	 * Collapses the menu and triggers any close animation.
	 * Override in Blueprint to add slide-out / fade effects.
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Programming Menu")
	void HideMenu();
	virtual void HideMenu_Implementation();

	/** @return true if the menu is currently visible/open. */
	UFUNCTION(BlueprintPure, Category = "Programming Menu")
	bool IsMenuOpen() const { return bIsOpen; }

	// ─── Program Controls ────────────────────────────────────────────────────

	/**
	 * Reads the current sequence, sends it to TargetRobot, and starts
	 * execution.  Does nothing and logs an error if:
	 *   - The sequence is empty.
	 *   - No robot is assigned and none can be found in the level.
	 *
	 * Bind this to a "Run" button's OnClicked event in the Blueprint layout.
	 */
	UFUNCTION(BlueprintCallable, Category = "Menu|Controls")
	void RunProgram();

	/**
	 * Stops the currently executing program on TargetRobot immediately.
	 * Bind this to a "Stop" button's OnClicked event.
	 */
	UFUNCTION(BlueprintCallable, Category = "Menu|Controls")
	void StopProgram();

	/**
	 * Clears all blocks from the program sequence.
	 * Bind this to a "Clear" button's OnClicked event.
	 */
	UFUNCTION(BlueprintCallable, Category = "Menu|Controls")
	void ClearProgram();

	/**
	 * Re-populates the palette from UBlockLibrary::GetDefaultBlocks().
	 * Called automatically in NativeConstruct; expose to Blueprint in case
	 * the palette needs to be refreshed at runtime (e.g. after an unlock).
	 */
	UFUNCTION(BlueprintCallable, Category = "Menu|Palette")
	void PopulatePalette();

protected:

	/** Populates the palette when the widget is first created. */
	virtual void NativeConstruct() override;

	/** Tracks whether the menu is currently shown. */
	UPROPERTY(BlueprintReadOnly, Category = "Programming Menu")
	bool bIsOpen = false;

private:

	/**
	 * Attempts to find a robot in the current level via TActorIterator.
	 * Used as a fallback inside RunProgram() when TargetRobot is null.
	 *
	 * @return  The first ARobotCharacter found, or nullptr if none exist.
	 */
	ARobotCharacter* FindRobotInLevel() const;

	/**
	 * Updates the StatusText block (if bound) with the given message.
	 * Safe to call when StatusText is not bound.
	 *
	 * @param Message  The string to display.
	 * @param Color    Text colour (default white).
	 */
	void SetStatusMessage(const FString& Message,
		FLinearColor Color = FLinearColor::White);
};