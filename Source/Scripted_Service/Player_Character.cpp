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

	if (bIsHoldingItem)
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
	
	FHitResult Hit;
	FVector Start = WorldLocation;
	FVector End = InteractDistance * Forward + WorldLocation;
	
	GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility, RV_TraceParams);

	if (bIsHoldingItem)
	{
		PhysicsHandleComponent->ReleaseComponent();
		bIsHoldingItem = false;
	}
	else
	{
		if (Hit.bBlockingHit)
		{
			AActor* HitActor = Hit.GetActor();
			
			const bool bIsGrabable = HitActor->Implements<UGrabableInterface>();
			if (bIsGrabable)
			{
				UPrimitiveComponent* HitComponent = Hit.GetComponent();
				FVector HitLocation = HitActor->GetActorLocation();
				FRotator HitRotation = HitActor->GetActorRotation();
				
				PhysicsHandleComponent->GrabComponentAtLocationWithRotation(HitComponent, FName("None") , HitLocation, HitRotation);
				bIsHoldingItem = true;
			}
			else if (HitActor->Implements<UInteractInterface>())
			{
				IInteractInterface::Execute_Interact(HitActor);
			}
			GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, TEXT("Interact" + HitActor->GetName() + " hit"));
		}
	}
}

void APlayer_Character::ToggleMenu()
{
	if (!ProgrammingMenuInstance)
	{
		UE_LOG(LogTemp, Warning, TEXT("Player_Character::ToggleMenu — "
			"ProgrammingMenuInstance is null. Did you set ProgrammingMenuClass?"));
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

// Called to bind functionality to input
void APlayer_Character::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &APlayer_Character::Move);

		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &APlayer_Character::Look);

		EnhancedInputComponent->BindAction(InteractAction, ETriggerEvent::Started, this, &APlayer_Character::Interact);
		
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ACharacter::Jump);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);

		EnhancedInputComponent->BindAction(ToggleMenuAction, ETriggerEvent::Started,   this, &APlayer_Character::ToggleMenu);
	}
}