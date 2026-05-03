#include "GameHUD.h"
#include "CustomGameMode.h"
#include "Kismet/GameplayStatics.h"
#include "Scripted_Service/Player_Character.h"

void UGameHUD::NativeConstruct()
{
    Super::NativeConstruct();

    ACustomGameMode* GM = Cast<ACustomGameMode>(
        UGameplayStatics::GetGameMode(GetWorld()));

    if (GM)
    {
        GM->OnStatsUpdated.AddDynamic(this, &UGameHUD::RefreshStats);

        // Cache the starting tip value so the delta is correct on first update
        Tips = GM->TotalTips;

        // Push initial values so the HUD shows 0s from the start
        RefreshStats(GM->Score, GM->TotalTips,
                     GM->OrdersCorrect, GM->OrdersWrong, GM->OrdersExpired);
    }
    else
    {
        UE_LOG(LogTemp, Warning,
            TEXT("GameHUD: Could not find ACustomGameMode — stats will not update."));
    }

    APlayer_Character* PC = Cast<APlayer_Character>(
        UGameplayStatics::GetPlayerCharacter(this, 0));

    if (PC)
    {
        PC->OnPlayerGrabItem.AddDynamic(this, &UGameHUD::HandleGrabItem);
        PC->OnRobotFailCommand.AddDynamic(this, &UGameHUD::HandleRobotFailCommand);
    }
    else
    {
        UE_LOG(LogTemp, Warning,
            TEXT("GameHUD: Could not find APlayer_Character — "
                 "grab and robot-error events will not fire."));
    }
}
void UGameHUD::HandleGrabItem(bool bisHoldingItem)
{
    // Forward to the BlueprintImplementableEvent so Blueprint can respond
    OnGrabItem(bisHoldingItem);
}

void UGameHUD::HandleRobotFailCommand(FString ErrorMessageText)
{
    // Forward to the BlueprintImplementableEvent so Blueprint can respond
    OnRobotFailCommand(ErrorMessageText);
}

void UGameHUD::RefreshStats(int32 Score, float TotalTips,
                             int32 OrdersCorrect, int32 OrdersWrong,
                             int32 OrdersExpired)
{
    if (ScoreText)
    {
        ScoreText->SetText(
            FText::FromString(FString::Printf(TEXT("Score: %d"), Score)));
    }

    if (TipsText)
    {
        TipsText->SetText(
            FText::FromString(FString::Printf(TEXT("Tips: $%.2f"), TotalTips)));
    }

    // Fire the tip-animation event only when tips actually increased
    if (TotalTips > Tips)
    {
        OnTipsChanged(TotalTips - Tips);
    }

    Tips = TotalTips;

    if (OrdersCorrectText)
    {
        OrdersCorrectText->SetText(
            FText::FromString(FString::Printf(TEXT("Correct: %d"), OrdersCorrect)));
    }

    if (OrdersWrongText)
    {
        OrdersWrongText->SetText(
            FText::FromString(FString::Printf(TEXT("Wrong: %d"), OrdersWrong)));
    }

    if (OrdersExpiredText)
    {
        OrdersExpiredText->SetText(
            FText::FromString(FString::Printf(TEXT("Expired: %d"), OrdersExpired)));
    }
}