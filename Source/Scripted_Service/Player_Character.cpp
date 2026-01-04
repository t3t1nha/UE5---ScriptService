// Fill out your copyright notice in the Description page of Project Settings.

#include "Player_Character.h"
#include "ApparatusActor.h"

// Sets default values
APlayer_Character::APlayer_Character()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	
	FirstPersonMeshComponent = CreateDefaultSubobject<USkeletalMeshComponent>(FName("FirstPersonMeshComponent"));
	check(FirstPersonMeshComponent != nullptr);

	// Attach the FirstPerson mesh to the skeletal mesh
	FirstPersonMeshComponent->SetupAttachment(GetMesh());
	FirstPersonMeshComponent->FirstPersonPrimitiveType = EFirstPersonPrimitiveType::FirstPerson;

	GetMesh()->FirstPersonPrimitiveType = EFirstPersonPrimitiveType::WorldSpaceRepresentation;
	
	FirstPersonCameraComponent = CreateDefaultSubobject<UCameraComponent>(FName("FirstPersonCameraComponent"));
	check(FirstPersonCameraComponent != nullptr);

	// Attach the camera component to the first-person Skeletal Mesh.
	FirstPersonCameraComponent->SetupAttachment(FirstPersonMeshComponent, FName("head"));

	// Position the camera slightly above the eyes and rotate it to behind the player's head
	FirstPersonCameraComponent->SetRelativeLocationAndRotation(FirstPersonCameraOffset, FRotator(0.0f, 90.0f, -90.0f));

	// Enable the pawn to control camera rotation.
	FirstPersonCameraComponent->bUsePawnControlRotation = true;

	// Enable first-person rendering and set default FOV and scale values
	FirstPersonCameraComponent->bEnableFirstPersonFieldOfView = true;
	FirstPersonCameraComponent->bEnableFirstPersonScale = true;
	FirstPersonCameraComponent->FirstPersonFieldOfView = FirstPersonFOV;
	FirstPersonCameraComponent->FirstPersonScale = FirstPersonScale;

	// Create Physics Handle Component
	PhysicsHandleComponent = CreateDefaultSubobject<UPhysicsHandleComponent>(TEXT("PhysicsHandleComponent"));

	// Add Component to Character
	AddInstanceComponent(PhysicsHandleComponent);
	PhysicsHandleComponent->OnComponentCreated();
	PhysicsHandleComponent->RegisterComponent(); 
}

// Called when the game starts or when spawned
void APlayer_Character::BeginPlay()
{
	Super::BeginPlay();
	
	check(GEngine != nullptr);

	// Only the owning player sees the first-person mesh
	FirstPersonMeshComponent->SetOnlyOwnerSee(true);

	// Set the animation on the first person mesh
	FirstPersonMeshComponent->SetAnimInstanceClass(FirstPersonDefaultAnim->GeneratedClass);
	
	// Get the player controller for this character
	if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
	{
		// Get the enhanced input local player subsystem and add a new input mapping context to it
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(InputMappingContext, 0);
		}
	}

	GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, TEXT("Using Player_Character"));
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
	const FVector2D LookAxisValue = Value.Get<FVector2D>();

	if (Controller)
	{
		AddControllerYawInput(LookAxisValue.X);
		AddControllerPitchInput(LookAxisValue.Y);
	}
}

void APlayer_Character::Interact()
{
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

// Called to bind functionality to input
void APlayer_Character::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		// Bind Movement Action
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &APlayer_Character::Move);

		// Bind Look Action
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &APlayer_Character::Look);

		// Bind Interact Action
		EnhancedInputComponent->BindAction(InteractAction, ETriggerEvent::Started, this, &APlayer_Character::Interact);
		
		// Bind Jump Actions
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ACharacter::Jump);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);
	}
}