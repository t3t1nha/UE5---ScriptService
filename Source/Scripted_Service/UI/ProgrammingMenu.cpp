// Fill out your copyright notice in the Description page of Project Settings.


#include "ProgrammingMenu.h"

void UProgrammingMenu::ShowMenu_Implementation()
{
	SetVisibility(ESlateVisibility::Visible);
	bIsOpen = true;
}

void UProgrammingMenu::HideMenu_Implementation()
{
	SetVisibility(ESlateVisibility::Collapsed);
	bIsOpen = false;
}