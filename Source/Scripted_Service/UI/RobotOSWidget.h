// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/Button.h"
#include "Components/Overlay.h"
#include "Components/TextBlock.h"
#include "ProgrammingMenu.h"
#include "RobotOSWidget.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnOSCloseRequested);

/**
 * 
 */
UCLASS()
class SCRIPTED_SERVICE_API URobotOSWidget : public UUserWidget
{
	GENERATED_BODY()
public:
 
    // ── Required Blueprint bindings ───────────────────────────────────────────
 
    /**
     * Root overlay that fills the entire screen.
     * Style this as the desktop background (dark panel, subtle grid, etc.).
     * Named "DesktopOverlay" in the Blueprint widget hierarchy.
     */
    UPROPERTY(meta = (BindWidget))
    UOverlay* DesktopOverlay;
 
    /**
     * Overlay stacked on top of DesktopOverlay where app "windows" are placed.
     * Children added here appear in front of the desktop background.
     * Named "WindowArea" in the Blueprint widget hierarchy.
     */
    UPROPERTY(meta = (BindWidget))
    UOverlay* WindowArea;
 
    /**
     * Clickable icon that opens or focuses the Programming app.
     * Named "ProgrammingAppIcon" in the Blueprint widget hierarchy.
     * Style it as an app tile / icon button (no text required — label is separate).
     */
    UPROPERTY(meta = (BindWidget))
    UButton* ProgrammingAppIcon;
 
    // ── Optional Blueprint bindings ───────────────────────────────────────────
 
    /**
     * Text label rendered beneath the programming icon (e.g. "Program").
     * Bind if your Blueprint layout includes it; harmless if absent.
     */
    UPROPERTY(meta = (BindWidgetOptional))
    UTextBlock* ProgrammingAppLabel;
 
    // ── Configuration (set in Blueprint defaults) ─────────────────────────────
 
    /**
     * The UProgrammingMenu subclass to instantiate when the player opens the app.
     * Set this to WBP_ProgrammingMenu in the Blueprint defaults of WBP_RobotOS.
     * The spawned instance inherits all block/container widget classes from its
     * own Blueprint defaults — you do NOT need to forward them here.
     */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "OS|Config")
    TSubclassOf<UProgrammingMenu> ProgrammingMenuClass;
 
    // ── Public API ────────────────────────────────────────────────────────────
 
    /**
     * Call this every time the OS is shown for a robot.
     * Passes the robot reference to the Programming app (if already open)
     * so any sequence runs on the correct robot.
     *
     * NOTE: Does NOT reset the dirty flag — the dirty flag only clears when
     * the player presses Run.  This means if the player made changes, closed
     * the OS without running, then re-opens it, the robot will still stay
     * paused on close.
     *
     * @param InRobot  The robot this OS session is controlling.
     */
    UFUNCTION(BlueprintCallable, Category = "OS")
    void InitializeOS(ARobotCharacter* InRobot);
 
    /**
     * Opens the Programming app window.  If it is already open, focuses it
     * instead of creating a second instance.
     * Bound to ProgrammingAppIcon's OnClicked; also callable from Blueprint.
     */
    UFUNCTION(BlueprintCallable, Category = "OS")
    void OpenProgrammingApp();
 
    /**
     * Returns true if the player has modified the program sequence since the
     * last time Run was pressed.  Used by Player_Character::CloseRobotOS() to
     * decide whether to auto-resume the robot.
     *
     * Returns false if the Programming app has never been opened (no changes
     * are possible without opening the app).
     */
    UFUNCTION(BlueprintPure, Category = "OS")
    bool HasPendingChanges() const;
 
    /**
     * Broadcasts OnCloseRequested.  Call from Blueprint or bind to an ESC
     * action if you prefer that route over the built-in NativeOnPreviewKeyDown.
     */
    UFUNCTION(BlueprintCallable, Category = "OS")
    void RequestClose();
 
    /**
     * Fired when the player wants to dismiss the OS (ESC key or any future
     * close button).  Player_Character binds here to restore game state.
     */
    UPROPERTY(BlueprintAssignable, Category = "OS|Events")
    FOnOSCloseRequested OnCloseRequested;
 
protected:
 
    virtual void NativeConstruct() override;
 
    /**
     * Called BEFORE the focused child widget handles a key press.
     * We intercept ESC here so it closes the OS regardless of which child
     * widget (e.g. ProgrammingMenu, an editable text box) currently has focus.
     */
    virtual FReply NativeOnPreviewKeyDown(
        const FGeometry& InGeometry,
        const FKeyEvent& InKeyEvent) override;
 
    /** Required for the OS widget itself to receive keyboard preview events. */
    virtual bool NativeSupportsKeyboardFocus() const override { return true; }
 
private:
 
    /** The robot this OS session is currently controlling. */
    UPROPERTY()
    ARobotCharacter* TargetRobot = nullptr;
 
    /**
     * The live Programming app instance inside WindowArea.
     * nullptr until the player first clicks the Programming icon.
     * Persists for the lifetime of the OS widget (not destroyed on close).
     */
    UPROPERTY()
    UProgrammingMenu* ProgrammingMenuInstance = nullptr;
 
    /** True once the Programming app window has been created and added. */
    bool bProgrammingAppOpen = false;
 
    /** Bound to ProgrammingAppIcon::OnClicked in NativeConstruct. */
    UFUNCTION()
    void OnProgrammingIconClicked();
};
