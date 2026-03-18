// GameHUDWidget.cpp
#include "GameHUD.h"
#include "CustomGameMode.h"
#include "Kismet/GameplayStatics.h"

void UGameHUD::NativeConstruct()
{
    Super::NativeConstruct();

    // Retrieve the game mode and cast it to our custom type.
    // UGameplayStatics::GetGameMode is safe to call on the client in
    // single-player — it always returns the authority game mode.
    ACustomGameMode* GM = Cast<ACustomGameMode>(
        UGameplayStatics::GetGameMode(GetWorld()));

    if (GM)
    {
        // Bind to the delegate so RefreshStats is called every time any stat changes.
        // AddDynamic is the correct macro for DYNAMIC multicast delegates.
        GM->OnStatsUpdated.AddDynamic(this, &UGameHUD::RefreshStats);

        // Pull the current values immediately — the GameMode may have already
        // broadcast an initial update before this widget was created.
        RefreshStats(GM->Score, GM->TotalTips,
                     GM->OrdersCorrect, GM->OrdersWrong, GM->OrdersExpired);

        UE_LOG(LogTemp, Log,
            TEXT("GameHUDWidget: Bound to ScriptedServiceGameMode::OnStatsUpdated."));
    }
    else
    {
        UE_LOG(LogTemp, Warning,
            TEXT("GameHUDWidget: Could not find AScriptedServiceGameMode. "
                 "Make sure the project's Game Mode is set to AScriptedServiceGameMode "
                 "(Project Settings → Maps & Modes  or  World Settings → Game Mode Override)."));
    }
}
void UGameHUD::RefreshStats(int32 Score, float TotalTips,
                                   int32 OrdersCorrect, int32 OrdersWrong,
                                   int32 OrdersExpired)
{
    // ── Score ─────────────────────────────────────────────────────────────────
    // FText::FromString + FString::Printf gives us full formatting control.
    if (ScoreText)
    {
        ScoreText->SetText(
            FText::FromString(FString::Printf(TEXT("Score: %d"), Score)));
    }

    // ── Tips (formatted as dollars with two decimal places) ───────────────────
    if (TipsText)
    {
        TipsText->SetText(
            FText::FromString(FString::Printf(TEXT("Tips: $%.2f"), TotalTips)));
    }

    // ── Correct order count ───────────────────────────────────────────────────
    if (OrdersCorrectText)
    {
        OrdersCorrectText->SetText(
            FText::FromString(FString::Printf(TEXT("Correct: %d"), OrdersCorrect)));
    }

    // ── Wrong order count ─────────────────────────────────────────────────────
    if (OrdersWrongText)
    {
        OrdersWrongText->SetText(
            FText::FromString(FString::Printf(TEXT("Wrong: %d"), OrdersWrong)));
    }

    // ── Expired order count ───────────────────────────────────────────────────
    if (OrdersExpiredText)
    {
        OrdersExpiredText->SetText(
            FText::FromString(FString::Printf(TEXT("Expired: %d"), OrdersExpired)));
    }
}