#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/TextBlock.h"
#include "GameHUD.generated.h"

UCLASS()
class SCRIPTED_SERVICE_API UGameHUD : public UUserWidget
{
    GENERATED_BODY()

public:
    /** Displays the current score. */
    UPROPERTY(meta = (BindWidgetOptional))
    UTextBlock* ScoreText;

    /** Displays the running tip total in dollars. */
    UPROPERTY(meta = (BindWidgetOptional))
    UTextBlock* TipsText;

    UFUNCTION(BlueprintImplementableEvent)
    void OnGrabItem(bool isHolding);

    UFUNCTION(BlueprintImplementableEvent)
    void OnRobotFailCommand(const FString& ErrorMessageText);

    UFUNCTION(BlueprintImplementableEvent)
    void OnTipsChanged(float TipsIncreaseValue);

protected:
    virtual void NativeConstruct() override;

private:
    /** Tracks the previous tip value so we can compute the delta for OnTipsChanged. */
    float Tips = -1.0f;

    UFUNCTION()
    void RefreshStats(int32 Score, float TotalTips);

    UFUNCTION()
    void HandleGrabItem(bool bisHoldingItem);

    UFUNCTION()
    void HandleRobotFailCommand(FString ErrorMessageText);
};