// Fill out your copyright notice in the Description page of Project Settings.

#include "OrderWidget.h"
#include "BaseIngredient.h"

void UOrderWidget::NativeConstruct()
{
    Super::NativeConstruct();

    SetVisibility(ESlateVisibility::Collapsed);
}

void UOrderWidget::ShowOrder(int32 TableNumber, const FOrderData& Order)
{
    if (TableLabel)
    {
        TableLabel->SetText(
            FText::Format(
                FText::FromString(TEXT("Table #{0}")),
                FText::AsNumber(TableNumber)
            )
        );
    }

    if (DishLabel)
    {
        FText DishName = FText::FromString(TEXT("Unknown"));

        if (Order.RequestedDish)
        {
            const ABaseIngredient* DefaultObj =
                Order.RequestedDish->GetDefaultObject<ABaseIngredient>();

            if (DefaultObj && !DefaultObj->DisplayName.IsNone())
            {
                DishName = FText::FromName(DefaultObj->DisplayName);
            }
            else
            {
                DishName = FText::FromString(Order.RequestedDish->GetName());
            }
        }

        DishLabel->SetText(DishName);
    }

    if (DishIcon)
    {
        DishIcon->SetVisibility(ESlateVisibility::Collapsed);
    }

    SetVisibility(ESlateVisibility::HitTestInvisible);

    OnOrderShown();

    UE_LOG(LogTemp, Log,
        TEXT("TableOrderWidget: Showing order for Table %d — %s"),
        TableNumber,
        *DishLabel->GetText().ToString());
}

void UOrderWidget::HideOrder()
{
    if (TableLabel)
    {
        TableLabel->SetText(FText::GetEmpty());
    }
    if (DishLabel)
    {
        DishLabel->SetText(FText::GetEmpty());
    }

    SetVisibility(ESlateVisibility::Collapsed);

    OnOrderHidden();

    UE_LOG(LogTemp, Log, TEXT("TableOrderWidget: Order hidden."));
}