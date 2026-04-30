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
    /**
     * Displays the current score.
     */
    UPROPERTY(meta = (BindWidgetOptional))
    UTextBlock* ScoreText;

    /**
     * Displays the running tip total in dollars.
     */
    UPROPERTY(meta = (BindWidgetOptional))
    UTextBlock* TipsText;

    /**
     * Running count of correctly delivered orders.
     */
    UPROPERTY(meta = (BindWidgetOptional))
    UTextBlock* OrdersCorrectText;

    /**
     * Running count of orders delivered with the wrong dish
     */
    UPROPERTY(meta = (BindWidgetOptional))
    UTextBlock* OrdersWrongText;

    /**
     * Running count of orders that timed out before delivery.
     */
    UPROPERTY(meta = (BindWidgetOptional))
    UTextBlock* OrdersExpiredText;

    UFUNCTION(BlueprintImplementableEvent)
    void OnGrabItem(bool isHolding);

    UFUNCTION(BlueprintImplementableEvent)
    void OnRobotFailCommand(FString ErrorMessageText);
    
    UFUNCTION(BlueprintImplementableEvent)
    void OnTipsChanged(float TipsIncreaseValue);
    
protected:

    /**
     * Called once when the widget is first created and added to the viewport.
     */
    virtual void NativeConstruct() override;
    
private:
    
    float Tips = -1.0f;
    /**
     * Callback bound to AScriptedServiceGameMode::OnStatsUpdated.
     * Updates every text block with the freshest values.
     *
     * Parameters mirror FOnStatsUpdated:
     * @param Score            Current cumulative score.
     * @param TotalTips        Total tips earned ($).
     * @param OrdersCorrect    Correctly delivered order count.
     * @param OrdersWrong      Wrongly delivered order count.
     * @param OrdersExpired    Timed-out order count.
     */
    UFUNCTION()
    void RefreshStats(int32 Score, float TotalTips,
                      int32 OrdersCorrect, int32 OrdersWrong, int32 OrdersExpired);
};