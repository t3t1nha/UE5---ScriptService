// Fill out your copyright notice in the Description page of Project Settings.

#include "RobotOSWidget.h"
#include "RobotCharacter.h"

void URobotOSWidget::NativeConstruct()
{
    Super::NativeConstruct();

    // Wire the icon button click → OpenProgrammingApp
    if (ProgrammingAppIcon)
    {
        ProgrammingAppIcon->OnClicked.AddDynamic(
            this, &URobotOSWidget::OnProgrammingIconClicked);
    }
    else
    {
        UE_LOG(LogTemp, Error,
            TEXT("RobotOSWidget: 'ProgrammingAppIcon' button is not bound. "
                 "Name a UButton 'ProgrammingAppIcon' in the Blueprint layout."));
    }
}

void URobotOSWidget::InitializeOS(ARobotCharacter* InRobot)
{
    TargetRobot = InRobot;

    // If the Programming app was already open from a previous session,
    // update its target robot so Run sends the program to the correct machine.
    if (ProgrammingMenuInstance && InRobot)
    {
        ProgrammingMenuInstance->SetTargetRobot(InRobot);
    }

    UE_LOG(LogTemp, Log,
        TEXT("RobotOSWidget: Initialized for robot '%s'."),
        InRobot ? *InRobot->GetName() : TEXT("NULL"));
}

void URobotOSWidget::OpenProgrammingApp()
{
    // ── App already open: focus it instead of spawning a duplicate ────────────
    if (bProgrammingAppOpen && ProgrammingMenuInstance)
    {
        // Make sure the window is visible (could have been hidden some other way)
        ProgrammingMenuInstance->SetVisibility(ESlateVisibility::Visible);

        // Shift Slate focus to the app so keyboard input routes into it
        ProgrammingMenuInstance->SetFocus();

        UE_LOG(LogTemp, Log,
            TEXT("RobotOSWidget: Programming app already open — focusing."));
        return;
    }

    // ── First open: validate class ────────────────────────────────────────────
    if (!ProgrammingMenuClass)
    {
        UE_LOG(LogTemp, Error,
            TEXT("RobotOSWidget: ProgrammingMenuClass is not set! "
                 "Assign WBP_ProgrammingMenu in the RobotOS Blueprint defaults."));
        return;
    }

    // ── Spawn ─────────────────────────────────────────────────────────────────
    ProgrammingMenuInstance =
        CreateWidget<UProgrammingMenu>(GetOwningPlayer(), ProgrammingMenuClass);

    if (!ProgrammingMenuInstance)
    {
        UE_LOG(LogTemp, Error,
            TEXT("RobotOSWidget: Failed to create ProgrammingMenu widget."));
        return;
    }

    // Pass the current robot so Run knows which machine to program
    if (TargetRobot)
    {
        ProgrammingMenuInstance->SetTargetRobot(TargetRobot);
    }

    // ── Insert into the window layer ──────────────────────────────────────────
    // WindowArea is an UOverlay; adding the menu as a child stacks it on top of
    // the desktop.  The Blueprint controls the menu's size/position via the
    // OverlaySlot padding/alignment settings.
    if (WindowArea)
    {
        WindowArea->AddChild(ProgrammingMenuInstance);
    }
    else
    {
        UE_LOG(LogTemp, Error,
            TEXT("RobotOSWidget: 'WindowArea' overlay is not bound. "
                 "Name a UOverlay 'WindowArea' in the Blueprint layout."));
    }

    bProgrammingAppOpen = true;

    UE_LOG(LogTemp, Log,
        TEXT("RobotOSWidget: Programming app opened for robot '%s'."),
        TargetRobot ? *TargetRobot->GetName() : TEXT("NULL"));
}

bool URobotOSWidget::HasPendingChanges() const
{
    // If the app was never opened the player couldn't have changed anything
    if (!ProgrammingMenuInstance)
    {
        return false;
    }

    return ProgrammingMenuInstance->IsProgramDirty();
}

void URobotOSWidget::RequestClose()
{
    UE_LOG(LogTemp, Log, TEXT("RobotOSWidget: Close requested."));
    OnCloseRequested.Broadcast();
}

FReply URobotOSWidget::NativeOnPreviewKeyDown(
    const FGeometry& InGeometry,
    const FKeyEvent& InKeyEvent)
{
    // NativeOnPreviewKeyDown fires on ancestor widgets BEFORE the focused child
    // widget processes the key.  Intercepting ESC here means it always closes
    // the OS, even if an editable text-box or other child has focus.
    if (InKeyEvent.GetKey() == EKeys::Escape)
    {
        RequestClose();
        return FReply::Handled(); // Consume — don't let children or game see it
    }

    return Super::NativeOnPreviewKeyDown(InGeometry, InKeyEvent);
}

void URobotOSWidget::OnProgrammingIconClicked()
{
    OpenProgrammingApp();
}