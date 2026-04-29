// Fill out your copyright notice in the Description page of Project Settings.


#include "TableActor.h"

ATableActor::ATableActor()
{
	PrimaryActorTick.bCanEverTick = true;
	TableNumber = 0;
	OrderWidgetInstance = nullptr;

	TableMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("TableMesh"));
	RootComponent = TableMesh;

	OrderIndicatorComponent = CreateDefaultSubobject<UWidgetComponent>(TEXT("OrderIndicatorComponent"));
	OrderIndicatorComponent->SetupAttachment(TableMesh);
	OrderIndicatorComponent->SetWidgetSpace(EWidgetSpace::World);
	OrderIndicatorComponent->SetDrawAtDesiredSize(true);
	OrderIndicatorComponent->SetVisibility(true);
}

void ATableActor::BeginPlay()
{
	Super::BeginPlay();

	CurrentOrder.OrderState    = EOrderState::Waiting;
	CurrentOrder.RequestedDish = nullptr;
	CurrentOrder.TableNumber   = TableNumber;
	CurrentOrder.TimeWaiting   = 0.0f;

	OrderIndicatorComponent->SetRelativeLocation(FVector(0.f, 0.f, 200.f));
 
	if (OrderWidgetClass)
	{
		OrderIndicatorComponent->SetWidgetClass(OrderWidgetClass);
 
		OrderWidgetInstance =
			Cast<UOrderWidget>(OrderIndicatorComponent->GetUserWidgetObject());
 
		if (!OrderWidgetInstance)
		{
			UE_LOG(LogTemp, Warning,
				TEXT("Table %d: OrderWidgetClass set but widget instance is null. "
					 "Make sure the class derives from UTableOrderWidget."),
				TableNumber);
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning,
			TEXT("Table %d: OrderWidgetClass is not set — no floating indicator "
				 "will be shown. Assign WBP_TableOrder in the Details panel."),
			TableNumber);
	}
	
	if (PossibleDishes.Num() == 0)
	{
		UE_LOG(LogTemp, Warning,
			TEXT("Table %d: PossibleDishes is empty — no automatic orders will be generated. "
				 "Add dish classes to the PossibleDishes"),
			TableNumber);
		return;
	}

	if (bGenerateOrderOnBeginPlay)
	{
		float InitialDelay = FMath::FRandRange(0.5f, 3.0f);
		GetWorldTimerManager().SetTimer(
			OrderGenerationTimerHandle,
			this,
			&ATableActor::GenerateRandomOrder,
			InitialDelay,
			false
		);
	}
	else
	{
		// Just wait for the normal interval before the first order.
		ScheduleNextOrder();
	}
}

/**
 *IOrderable INTERFACE IMPLEMENTATION
 */
bool ATableActor::HasPendingOrder() const
{
	return CurrentOrder.OrderState == EOrderState::Waiting
		&& CurrentOrder.RequestedDish != nullptr;
}

FOrderData ATableActor::GetCurrentOrder() const
{
	return CurrentOrder;
}

void ATableActor::PlaceOrder(TSubclassOf<ABaseIngredient> Dish)
{
	if (!Dish)
	{
		UE_LOG(LogTemp, Warning, TEXT("Table %d: Attempted to place order with null dish"), TableNumber);
		return;
	}
    
	CurrentOrder.RequestedDish = Dish;
	CurrentOrder.OrderState = EOrderState::Waiting;
	CurrentOrder.TableNumber = TableNumber;
	CurrentOrder.TimeWaiting = 0.0f;
    
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(
			TableNumber,
			8.0f,
			FColor::Cyan,
			FString::Printf(TEXT("🍽  Table %d wants: %s"), TableNumber, *Dish->GetName())
		);
	}

	ShowOrderIndicator();
	
	// Broadcast so Blueprint / HUD can react
	OnOrderPlaced.Broadcast(TableNumber, CurrentOrder);

	// Start the timeout countdown (if enabled).
	if (OrderTimeoutDuration > 0.0f)
	{
		// Clear any existing timeout before setting a fresh one.
		ClearTimeoutTimer();

		GetWorldTimerManager().SetTimer(
			OrderTimeoutTimerHandle,
			this,
			&ATableActor::OnOrderTimedOut,
			OrderTimeoutDuration,
			false
		);
	}
}

bool ATableActor::DeliverOrder(TSubclassOf<ABaseIngredient> Dish)
{
	// Stop the timeout — the robot arrived in time
	ClearTimeoutTimer();

	if (Dish == CurrentOrder.RequestedDish)
	{
		CurrentOrder.OrderState = EOrderState::Delivered;

		HideOrderIndicator();

		UE_LOG(LogTemp, Log,
			TEXT("Table %d: Correct dish delivered!"), TableNumber);

		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(
				TableNumber, 4.0f, FColor::Green,
				FString::Printf(TEXT("Table %d: Correct dish delivered!"), TableNumber)
			);
		}

		// Notify listeners
		OnOrderDelivered.Broadcast(TableNumber, true);

		// Schedule the next order after the success
		ScheduleNextOrder();
		return true;
	}
	else
	{
		UE_LOG(LogTemp, Error,
			TEXT("Table %d: Wrong dish delivered! Expected '%s', got '%s'."),
			TableNumber,
			CurrentOrder.RequestedDish ? *CurrentOrder.RequestedDish->GetName() : TEXT("None"),
			Dish                       ? *Dish->GetName()                        : TEXT("None"));

		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(
				TableNumber, 4.0f, FColor::Red,
				FString::Printf(TEXT("✗  Table %d: Wrong dish!"), TableNumber)
			);
		}

		// Notify listeners — wrong delivery
		OnOrderDelivered.Broadcast(TableNumber, false);

		// Re-start the timeout since the order is still pending
		if (OrderTimeoutDuration > 0.0f)
		{
			GetWorldTimerManager().SetTimer(
				OrderTimeoutTimerHandle,
				this,
				&ATableActor::OnOrderTimedOut,
				OrderTimeoutDuration,
				false
			);
		}

		return false;
	}
}

FVector ATableActor::GetInteractionLocation() const
{
	return GetActorLocation();
}

int32 ATableActor::GetOrderableID() const
{
	return TableNumber;
}

void ATableActor::GenerateRandomOrder()
{
	if (PossibleDishes.Num() == 0)
	{
		UE_LOG(LogTemp, Warning,
			TEXT("Table %d: GenerateRandomOrder called but PossibleDishes is empty."),
			TableNumber);
		return;
	}

	// Guard: don't overwrite an in-progress order.
	if (CurrentOrder.RequestedDish != nullptr
		&& CurrentOrder.OrderState != EOrderState::Delivered)
	{
		UE_LOG(LogTemp, Verbose,
			TEXT("Table %d: Skipping new order — a previous order is still active (state %d)."),
			TableNumber, (int32)CurrentOrder.OrderState);
		return;
	}

	// Pick a uniformly random dish from the pool.
	const int32 RandomIndex = FMath::RandRange(0, PossibleDishes.Num() - 1);
	TSubclassOf<ABaseIngredient> ChosenDish = PossibleDishes[RandomIndex];

	UE_LOG(LogTemp, Log,
		TEXT("Table %d: Randomly selected dish '%s' (index %d / %d)."),
		TableNumber, *ChosenDish->GetName(), RandomIndex, PossibleDishes.Num() - 1);

	// Delegate to PlaceOrder which handles state, feedback, and timeout.
	PlaceOrder(ChosenDish);
}

void ATableActor::CancelCurrentOrder()
{
	// Stop any running timers.
	GetWorldTimerManager().ClearTimer(OrderGenerationTimerHandle);
	ClearTimeoutTimer();

	// Reset order state.
	CurrentOrder.RequestedDish = nullptr;
	CurrentOrder.OrderState    = EOrderState::Waiting;
	CurrentOrder.TimeWaiting   = 0.0f;

	UE_LOG(LogTemp, Log, TEXT("Table %d: Order cancelled and table reset."), TableNumber);
}

void ATableActor::ScheduleNextOrder()
{
	if (PossibleDishes.Num() == 0)
	{
		return;
	}

	// Clamp so MaxOrderInterval is always >= MinOrderInterval.
	const float ClampedMax = FMath::Max(MaxOrderInterval, MinOrderInterval);
	const float Delay      = FMath::FRandRange(MinOrderInterval, ClampedMax);

	UE_LOG(LogTemp, Log,
		TEXT("Table %d: Next order scheduled in %.1f seconds."),
		TableNumber, Delay);

	GetWorldTimerManager().SetTimer(
		OrderGenerationTimerHandle,
		this,
		&ATableActor::GenerateRandomOrder,
		Delay,
		false
	);
}

void ATableActor::OnOrderTimedOut()
{
	UE_LOG(LogTemp, Warning,
		TEXT("Table %d: Order for '%s' TIMED OUT after %.1f seconds — no robot came in time."),
		TableNumber,
		CurrentOrder.RequestedDish ? *CurrentOrder.RequestedDish->GetName() : TEXT("None"),
		OrderTimeoutDuration);

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(
			TableNumber, 5.0f, FColor::Orange,
			FString::Printf(TEXT("⏰  Table %d: Order timed out!"), TableNumber)
		);
	}

	HideOrderIndicator();
	
	// Notify listeners.
	OnOrderExpired.Broadcast(TableNumber);

	// Clear order and schedule a fresh one.
	CurrentOrder.RequestedDish = nullptr;
	CurrentOrder.OrderState    = EOrderState::Waiting;
	CurrentOrder.TimeWaiting   = 0.0f;

	ScheduleNextOrder();
}

void ATableActor::ClearTimeoutTimer()
{
	if (GetWorldTimerManager().IsTimerActive(OrderTimeoutTimerHandle))
	{
		GetWorldTimerManager().ClearTimer(OrderTimeoutTimerHandle);
	}
}

void ATableActor::ShowOrderIndicator()
{
	OrderIndicatorComponent->SetVisibility(true);
 
	if (OrderWidgetInstance)
	{
		OrderWidgetInstance->ShowOrder(TableNumber, CurrentOrder);
	}
 
	UE_LOG(LogTemp, Log,
		TEXT("Table %d: Order indicator shown."), TableNumber);
}
 
void ATableActor::HideOrderIndicator()
{
	OrderIndicatorComponent->SetVisibility(false);
 
	if (OrderWidgetInstance)
	{
		OrderWidgetInstance->HideOrder();
	}
 
	UE_LOG(LogTemp, Log,
		TEXT("Table %d: Order indicator hidden."), TableNumber);
}