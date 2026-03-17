// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "StructTypes.h"
#include "OrderWidget.generated.h"

UCLASS()
class SCRIPTED_SERVICE_API UOrderWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    /**
     * Text block that displays "Table #N".
     * Bound by name — the Blueprint widget must be named "TableLabel".
     */
    UPROPERTY(meta = (BindWidget))
    UTextBlock* TableLabel;

    /**
     * Text block that displays the dish display name (e.g. "Burger", "Pizza").
     * Bound by name — the Blueprint widget must be named "DishLabel".
     */
    UPROPERTY(meta = (BindWidget))
    UTextBlock* DishLabel;

    /**
     * Optional dish icon image.
     * Bound by name — the Blueprint widget must be named "DishIcon".
     * Hidden when no texture is mapped to the requested dish class.
     */
    UPROPERTY(meta = (BindWidgetOptional))
    UImage* DishIcon;

    /**
     * Populate the widget labels and make the widget visible.
     *
     * @param TableNumber  The table number to display (e.g. 1, 2, 3 …).
     * @param Order        The full order data — RequestedDish is used to derive
     *                     the dish name displayed in DishLabel.
     */
    UFUNCTION(BlueprintCallable, Category = "Table|UI")
    void ShowOrder(int32 TableNumber, const FOrderData& Order);

    /**
     * Hide the widget and clear both text labels.
     * Call this when the order is delivered or expires.
     */
    UFUNCTION(BlueprintCallable, Category = "Table|UI")
    void HideOrder();

    /**
     * Called after ShowOrder() populates the labels.
     * Override in Blueprint to play a pop-in animation, pulse effect, etc.
     */
    UFUNCTION(BlueprintImplementableEvent, Category = "Table|UI")
    void OnOrderShown();

    /**
     * Called after HideOrder() clears the labels.
     * Override in Blueprint to play a fade-out animation before Collapse.
     */
    UFUNCTION(BlueprintImplementableEvent, Category = "Table|UI")
    void OnOrderHidden();

protected:

    /** Called by the engine after the widget is fully constructed. */
    virtual void NativeConstruct() override;
};