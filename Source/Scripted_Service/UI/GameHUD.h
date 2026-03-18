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

    // ── Blueprint-bound text blocks ───────────────────────────────────────────

    /**
     * Displays the current score.
     * Example text: "Score: 300"
     * Named "ScoreText" in the WBP_GameHUD widget Blueprint.
     */
    UPROPERTY(meta = (BindWidgetOptional))
    UTextBlock* ScoreText;

    /**
     * Displays the running tip total in dollars.
     * Example text: "Tips: $15.00"
     * Named "TipsText" in the WBP_GameHUD widget Blueprint.
     */
    UPROPERTY(meta = (BindWidgetOptional))
    UTextBlock* TipsText;

    /**
     * Running count of correctly delivered orders.
     * Example text: "✓ 3"
     * Named "OrdersCorrectText" in the WBP_GameHUD widget Blueprint.
     */
    UPROPERTY(meta = (BindWidgetOptional))
    UTextBlock* OrdersCorrectText;

    /**
     * Running count of orders delivered with the wrong dish.
     * Example text: "✗ 1"
     * Named "OrdersWrongText" in the WBP_GameHUD widget Blueprint.
     */
    UPROPERTY(meta = (BindWidgetOptional))
    UTextBlock* OrdersWrongText;

    /**
     * Running count of orders that timed out before delivery.
     * Example text: "⏱ 0"
     * Named "OrdersExpiredText" in the WBP_GameHUD widget Blueprint.
     */
    UPROPERTY(meta = (BindWidgetOptional))
    UTextBlock* OrdersExpiredText;

protected:

    /**
     * Called once when the widget is first created and added to the viewport.
     *
     * Finds AScriptedServiceGameMode and binds RefreshStats to its
     * OnStatsUpdated delegate.  Also fires an immediate RefreshStats call
     * so the HUD shows the current values right away (e.g. after a reset).
     */
    virtual void NativeConstruct() override;

private:

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