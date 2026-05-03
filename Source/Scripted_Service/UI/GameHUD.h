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

    /** Running count of correctly delivered orders. */
    UPROPERTY(meta = (BindWidgetOptional))
    UTextBlock* OrdersCorrectText;

    /** Running count of orders delivered with the wrong dish. */
    UPROPERTY(meta = (BindWidgetOptional))
    UTextBlock* OrdersWrongText;

    /** Running count of orders that timed out before delivery. */
    UPROPERTY(meta = (BindWidgetOptional))
    UTextBlock* OrdersExpiredText;

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
    void RefreshStats(int32 Score, float TotalTips,
                      int32 OrdersCorrect, int32 OrdersWrong, int32 OrdersExpired);

    UFUNCTION()
    void HandleGrabItem(bool bisHoldingItem);

    UFUNCTION()
    void HandleRobotFailCommand(FString ErrorMessageText);
};