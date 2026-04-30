// GameHUDWidget.cpp
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

        Tips = GM->TotalTips;
        
        RefreshStats(GM->Score, GM->TotalTips,
                     GM->OrdersCorrect, GM->OrdersWrong, GM->OrdersExpired);
    }
    else
    {
        UE_LOG(LogTemp, Warning,
            TEXT("GameHUDWidget: Could not find AScriptedServiceGameMode"));
    }

    APlayer_Character* PC = Cast<APlayer_Character>(UGameplayStatics::GetPlayerCharacter(this, 0));

    if (PC)
    {
        PC->OnPlayerGrabItem.AddDynamic(this, &UGameHUD::OnGrabItem);
        PC->OnRobotFailCommand.AddDynamic(this, &UGameHUD::OnRobotFailCommand);
    }
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