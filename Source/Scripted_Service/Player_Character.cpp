// Fill out your copyright notice in the Description page of Project Settings.

#include "Player_Character.h"
#include "ApparatusActor.h"

// Sets default values
APlayer_Character::APlayer_Character()
{
	PrimaryActorTick.bCanEverTick = true;
	
	FirstPersonMeshComponent = CreateDefaultSubobject<USkeletalMeshComponent>(FName("FirstPersonMeshComponent"));
	check(FirstPersonMeshComponent != nullptr);

	FirstPersonMeshComponent->SetupAttachment(GetMesh());
	FirstPersonMeshComponent->FirstPersonPrimitiveType = EFirstPersonPrimitiveType::FirstPerson;

	GetMesh()->FirstPersonPrimitiveType = EFirstPersonPrimitiveType::WorldSpaceRepresentation;
	
	FirstPersonCameraComponent = CreateDefaultSubobject<UCameraComponent>(FName("FirstPersonCameraComponent"));
	check(FirstPersonCameraComponent != nullptr);

	FirstPersonCameraComponent->SetupAttachment(FirstPersonMeshComponent, FName("head"));

	FirstPersonCameraComponent->SetRelativeLocationAndRotation(FirstPersonCameraOffset, FRotator(0.0f, 90.0f, -90.0f));

	FirstPersonCameraComponent->bUsePawnControlRotation = true;

	FirstPersonCameraComponent->bEnableFirstPersonFieldOfView = true;
	FirstPersonCameraComponent->bEnableFirstPersonScale = true;
	FirstPersonCameraComponent->FirstPersonFieldOfView = FirstPersonFOV;
	FirstPersonCameraComponent->FirstPersonScale = FirstPersonScale;

	PhysicsHandleComponent = CreateDefaultSubobject<UPhysicsHandleComponent>(TEXT("PhysicsHandleComponent"));

	AddInstanceComponent(PhysicsHandleComponent);
	PhysicsHandleComponent->OnComponentCreated();
	PhysicsHandleComponent->RegisterComponent(); 
}

// Called when the game starts or when spawned
void APlayer_Character::BeginPlay()
{
	Super::BeginPlay();
	
	check(GEngine != nullptr);

	FirstPersonMeshComponent->SetOnlyOwnerSee(true);

	FirstPersonMeshComponent->SetAnimInstanceClass(FirstPersonDefaultAnim->GeneratedClass);
	
	if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(InputMappingContext, 0);
		}
	}

	if (ProgrammingMenuClass)
	{
		ProgrammingMenuInstance = CreateWidget<UProgrammingMenu>(GetWorld(), ProgrammingMenuClass);

		if (ProgrammingMenuInstance)
		{
			// ZOrder 10 keeps it above most other HUD elements
			ProgrammingMenuInstance->AddToViewport(10);

			// Start hidden — HideMenu sets Collapsed and bIsOpen = false
			ProgrammingMenuInstance->HideMenu();
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Player_Character: ProgrammingMenuClass is not set! "
			"Assign WBP_ProgrammingMenu in the Blueprint defaults."));
	}

	if (HUDWidgetClass)
	{
		HUDWidgetInstance = CreateWidget<UGameHUD>(GetWorld(), HUDWidgetClass);
		if (HUDWidgetInstance)
		{
			// ZOrder 0 — sits below the Programming Menu (which can use ZOrder 1+)
			HUDWidgetInstance->AddToViewport(0);
 
			UE_LOG(LogTemp, Log,
				TEXT("Player_Character: HUD widget created and added to viewport."));
		}
		else
		{
			UE_LOG(LogTemp, Warning,
				TEXT("Player_Character: Failed to create HUD widget from HUDWidgetClass."));
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning,
			TEXT("Player_Character: HUDWidgetClass is not set. "
				 "Assign WBP_GameHUD in the Blueprint defaults."));
	}
}

// Called every frame
void APlayer_Character::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (HeldItem != nullptr)
	{
		FVector WorldLocation = FirstPersonCameraComponent->GetComponentLocation();
		FVector ForwardVector = FirstPersonCameraComponent->GetForwardVector();

		FVector NewLocation = WorldLocation + (ForwardVector * GrabDistance);
		
		PhysicsHandleComponent->SetTargetLocation(NewLocation);
	}
}

void APlayer_Character::Move(const FInputActionValue& Value)
{
	if (bIsMenuOpen) return;
	
	const FVector2D MovementValue = Value.Get<FVector2D>();

	if (Controller)
	{
		// Add Right and Left movement
		const FVector Right = GetActorRightVector();
		AddMovementInput(Right, MovementValue.X);

		// Add forward and Backward movement
		const FVector Forward = GetActorForwardVector();
		AddMovementInput(Forward, MovementValue.Y);
	}
}

void APlayer_Character::Look(const FInputActionValue& Value)
{
	if (bIsMenuOpen) return;
	
	const FVector2D LookAxisValue = Value.Get<FVector2D>();

	if (Controller)
	{
		AddControllerYawInput(LookAxisValue.X);
		AddControllerPitchInput(LookAxisValue.Y);
	}
}

void APlayer_Character::Interact()
{
	if (bIsMenuOpen) return;
	
	FVector WorldLocation = FirstPersonCameraComponent->GetComponentLocation();
	FVector Forward = FirstPersonCameraComponent->GetForwardVector();

	FCollisionQueryParams RV_TraceParams = FCollisionQueryParams(FName(TEXT("RV_Trace")), true, this);
	RV_TraceParams.bTraceComplex = true;
	RV_TraceParams.bReturnPhysicalMaterial = false;

	if (HeldItem != nullptr)
	{
		RV_TraceParams.AddIgnoredActor(HeldItem);
	}
	
	FHitResult Hit;
	FVector Start = WorldLocation;
	FVector End = InteractDistance * Forward + WorldLocation;
	
	GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility, RV_TraceParams);

	if (Hit.bBlockingHit)
	{
		AActor* HitActor = Hit.GetActor();
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Yellow, 
		FString::Printf(TEXT("Hit: %s"), *HitActor->GetName()));

		// Grabable Check
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Cyan, 
			FString::Printf(TEXT("IsGrabable: %d"), HitActor->Implements<UGrabableInterface>()));

		// Interactable Check
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, 
			FString::Printf(TEXT("IsInteractable: %d"), HitActor->Implements<UInteractInterface>()));

		if (HeldItem != nullptr)
		{
			AApparatusActor* Apparatus = Cast<AApparatusActor>(HitActor);
			if (Apparatus != nullptr)
			{
				Apparatus->SnapIngredient(HeldItem);
				Drop();
			}
		}	

		const bool bIsGrabable = HitActor->Implements<UGrabableInterface>();

		if (HitActor->Implements<UInteractInterface>())
		{
			IInteractInterface::Execute_Interact(HitActor);
		}
		else if (bIsGrabable)
		{
			UPrimitiveComponent* HitComponent = Hit.GetComponent();
			FVector HitLocation = HitActor->GetActorLocation();
			FRotator HitRotation = HitActor->GetActorRotation();

			UPrimitiveComponent* Root = Cast<UPrimitiveComponent>(HitActor->GetRootComponent());
			if (Root)
			{
				Root->SetSimulatePhysics(true);
			}
			
			PhysicsHandleComponent->GrabComponentAtLocationWithRotation(HitComponent, FName("None") , HitLocation, HitRotation);
			ABaseIngredient* HitIngredient = Cast<ABaseIngredient>(HitActor);
			HeldItem = HitIngredient;
			OnPlayerGrabItem.Broadcast(true);
		}
	}
}

void APlayer_Character::Drop()
{
	if (HeldItem != nullptr)
	{
		PhysicsHandleComponent->ReleaseComponent();
		HeldItem = nullptr;
		OnPlayerGrabItem.Broadcast(false);
	};
};

void APlayer_Character::ToggleMenu()
{
	if (!ProgrammingMenuInstance)
	{
		UE_LOG(LogTemp, Warning, TEXT("Player_Character::ToggleMenu — "
			"ProgrammingMenuInstance is null"));
		return;
	}

	APlayerController* PC = Cast<APlayerController>(Controller);
	if (!PC)
	{
		return;
	}

	if (bIsMenuOpen)
	{
		// CLOSE MENU
		// Hide the widget, restore Game-Only input, hide cursor
		ProgrammingMenuInstance->HideMenu();
		bIsMenuOpen = false;

		// FInputModeGameOnly: mouse is captured, no cursor, full game input.
		FInputModeGameOnly GameInputMode;
		PC->SetInputMode(GameInputMode);
		PC->bShowMouseCursor = false;

		UE_LOG(LogTemp, Log, TEXT("Programming menu closed — game input restored"));
	}
	else
	{
		// OPEN MENU
		// Show the widget, switch to UI-Only input, show cursor
		ProgrammingMenuInstance->ShowMenu();
		bIsMenuOpen = true;

		// FInputModeUIOnly: all input goes to the widget, camera does NOT
		// move while the menu is open
		FInputModeGameAndUI GameAndUIMode;
		GameAndUIMode.SetWidgetToFocus(ProgrammingMenuInstance->TakeWidget());
		GameAndUIMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
		PC->SetInputMode(GameAndUIMode);
		PC->bShowMouseCursor = true;

		UE_LOG(LogTemp, Log, TEXT("Programming menu opened — UI input mode active"));
	}
}

void APlayer_Character::CloseActiveUI()
{
	if (bIsOSOpen)
	{
		CloseRobotOS();
	} else if (bIsMenuOpen)
	{
		ToggleMenu();
	}
}

void APlayer_Character::OpenRobotOS(ARobotCharacter* Robot)
{
	if (!Robot) return;

	if (!RobotOSInstance)
	{
		if (!RobotOSWidgetClass) return;

		RobotOSInstance = CreateWidget<URobotOSWidget>(GetWorld(), RobotOSWidgetClass);

		RobotOSInstance->AddToViewport(20);
		RobotOSInstance->OnCloseRequested.AddDynamic(this, &APlayer_Character::CloseRobotOS);
	}

	bRobotWasPausedOnOpen = Robot->bIsExecuting && Robot->bIsPaused;
	Robot = Robot;

	// ── Initialize and show ───────────────────────────────────────────────────
	RobotOSInstance->InitializeOS(Robot);
	RobotOSInstance->SetVisibility(ESlateVisibility::Visible);
 
	// ── Input mode ────────────────────────────────────────────────────────────
	APlayerController* PC = Cast<APlayerController>(Controller);
	if (PC)
	{
		// GameAndUI: game movement is blocked by our bIsMenuOpen guard in Move/Look,
		// but the Enhanced Input bindings (CloseOSAction etc.) still fire.
		FInputModeGameAndUI GameAndUIMode;
		GameAndUIMode.SetWidgetToFocus(RobotOSInstance->TakeWidget());
		GameAndUIMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
		PC->SetInputMode(GameAndUIMode);
		PC->bShowMouseCursor = true;
	}
 
	// Suppress Move / Look / Interact
	bIsMenuOpen = true;
	bIsOSOpen   = true;
 
	UE_LOG(LogTemp, Log,
		TEXT("Player_Character: Robot OS opened for '%s'. "
			 "Robot was %s when OS opened."),
		*Robot->GetName(),
		bRobotWasPausedOnOpen ? TEXT("PAUSED") : TEXT("IDLE"));
}

void APlayer_Character::CloseRobotOS()
{
	// Guard against spurious calls (e.g. ESC pressed when OS is already closed)
    if (!bIsOSOpen)
    {
        return;
    }
 
    // ── Hide the overlay ──────────────────────────────────────────────────────
    if (RobotOSInstance)
    {
        RobotOSInstance->SetVisibility(ESlateVisibility::Collapsed);
    }
 
    // ── Restore game input ────────────────────────────────────────────────────
    APlayerController* PC = Cast<APlayerController>(Controller);
    if (PC)
    {
        FInputModeGameOnly GameInputMode;
        PC->SetInputMode(GameInputMode);
        PC->bShowMouseCursor = false;
    }
 
    bIsMenuOpen = false;
    bIsOSOpen   = false;
 
    // ── Robot resume logic ────────────────────────────────────────────────────
    //
    // We auto-resume ONLY if ALL of the following are true:
    //   a) The robot had a program paused when the OS opened  (bRobotWasPausedOnOSOpen)
    //   b) The robot is still in a paused state right now     (bIsExecuting && bIsPaused)
    //      → This would be false if the player pressed Run inside the OS, which
    //        started a fresh execution and the robot is no longer paused.
    //   c) The player made NO changes to the program sequence (HasPendingChanges == false)
    //      → If dirty, the player must press Run to commit changes explicitly.
    //
    if (targetRobot)
    {
        const bool bStillPaused =
            targetRobot->bIsExecuting && targetRobot->bIsPaused;
 
        const bool bHasChanges =
            RobotOSInstance && RobotOSInstance->HasPendingChanges();
 
        if (bRobotWasPausedOnOpen && bStillPaused && !bHasChanges)
        {
            UE_LOG(LogTemp, Log,
                TEXT("Player_Character: No changes made — auto-resuming robot '%s'."),
                *targetRobot->GetName());
 
            targetRobot->ResumeProgram();
        }
        else if (bHasChanges)
        {
            UE_LOG(LogTemp, Log,
                TEXT("Player_Character: Program was modified — robot '%s' stays paused. "
                     "Press Run in the OS to execute the new program."),
                *targetRobot->GetName());
 
            if (GEngine)
            {
                GEngine->AddOnScreenDebugMessage(
                    -1, 4.0f, FColor::Yellow,
                    TEXT("⚠  Program changed — press Run to apply."));
            }
        }
        else
        {
            UE_LOG(LogTemp, Log,
                TEXT("Player_Character: Robot '%s' was idle when OS opened — no resume needed."),
                *targetRobot->GetName());
        }
    }
 
    // Clear the snapshot — next OS open will refresh it
    targetRobot           = nullptr;
    bRobotWasPausedOnOpen = false;
 
    UE_LOG(LogTemp, Log, TEXT("Player_Character: Robot OS closed."));
}

// Called to bind functionality to input
void APlayer_Character::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &APlayer_Character::Move);

		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &APlayer_Character::Look);

		EnhancedInputComponent->BindAction(InteractAction, ETriggerEvent::Started, this, &APlayer_Character::Interact);
		EnhancedInputComponent->BindAction(DropAction, ETriggerEvent::Started, this, &APlayer_Character::Drop);
		
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ACharacter::Jump);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);

		EnhancedInputComponent->BindAction(ToggleMenuAction, ETriggerEvent::Started,   this, &APlayer_Character::ToggleMenu);
		EnhancedInputComponent->BindAction(CloseAction, ETriggerEvent::Triggered, this, &APlayer_Character::CloseActiveUI);
	}
}