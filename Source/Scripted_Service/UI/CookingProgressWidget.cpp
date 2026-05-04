// Fill out your copyright notice in the Description page of Project Settings.


#include "CookingProgressWidget.h"

void UCookingProgressWidget::UpdateProgress(float NewProgress)
{
	// Clamp defensively — caller should already be in [0,1]
	CookingProgress = FMath::Clamp(NewProgress, 0.0f, 1.0f);

	OnProgressUpdated(CookingProgress);
}