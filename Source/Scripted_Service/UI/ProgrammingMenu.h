// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ProgrammingMenu.generated.h"

/**
 */
UCLASS()
class SCRIPTED_SERVICE_API UProgrammingMenu : public UUserWidget
{
	GENERATED_BODY()

public:

	/**
	 * Call this to make the menu visible and run its open animation (if any).
	 * Override in Blueprint to add slide-in / fade animations.
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Programming Menu")
	void ShowMenu();
	virtual void ShowMenu_Implementation();

	/**
	 * Call this to collapse the menu and run its close animation (if any).
	 * Override in Blueprint to add slide-out / fade animations.
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Programming Menu")
	void HideMenu();
	virtual void HideMenu_Implementation();

	/**
	 * @return true if the menu is currently open/visible.
	 * Read this from Blueprint HUD or GameMode if you need to know the state.
	 */
	UFUNCTION(BlueprintPure, Category = "Programming Menu")
	bool IsMenuOpen() const { return bIsOpen; }

protected:

	/** Tracks whether the menu is currently shown. Set by Show/HideMenu. */
	UPROPERTY(BlueprintReadOnly, Category = "Programming Menu")
	bool bIsOpen = false;
};