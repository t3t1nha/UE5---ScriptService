// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "Camera/CameraComponent.h"
#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Player_Character.generated.h"

class UInputMappingContext;
class UInputAction;
class UInputComponent;

UCLASS()
class SCRIPTED_SERVICE_API APlayer_Character : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	APlayer_Character();

protected:
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputMappingContext> InputMappingContext;

	// Move Input Actions
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	TObjectPtr<UInputAction> MoveAction;
 
	// Jump Input Actions
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	TObjectPtr<UInputAction> JumpAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	UInputAction* LookAction;
	
public:
	virtual void Tick(float DeltaTime) override;
	
	UFUNCTION()
	void Move(const FInputActionValue& Value);
	UFUNCTION()
	void Look(const FInputActionValue& Value);
	
	UPROPERTY(VisibleAnywhere, Category = Camera)
	UCameraComponent* FirstPersonCamera;
	UPROPERTY(VisibleAnywhere, Category = Camera)
	FVector FirstPersonCameraOffset = FVector(2.8f, 5.9f, 0);

	// First Person primitives Field Of View
	UPROPERTY(EditAnywhere, Category = Camera)
	float FirstPersonFOV = 70.0f;
	// First Person primitives view scale
	UPROPERTY(EditAnywhere, Category = Camera)
	float FirstPersonScale = 0.6f;

	// First Person mesh
	UPROPERTY(VisibleAnywhere, Category = Mesh)
	USkeletalMeshComponent* FirstPersonMeshComponent;
	
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
};
