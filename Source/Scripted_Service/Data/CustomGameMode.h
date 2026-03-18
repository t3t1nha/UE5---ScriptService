// ScriptedServiceGameMode.h
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "TableActor.h"
#include "CustomGameMode.generated.h"

// ─────────────────────────────────────────────────────────────────────────────
//  Delegate — broadcast whenever any tracked stat changes.
//  The HUD widget binds to this so it never has to poll.
//
//  @param Score            Current cumulative score (points).
//  @param TotalTips        Total tips earned so far (dollars, float).
//  @param OrdersCorrect    Running count of correctly delivered orders.
//  @param OrdersWrong      Running count of wrongly delivered orders.
//  @param OrdersExpired    Running count of orders that timed-out.
// ─────────────────────────────────────────────────────────────────────────────
DECLARE_DYNAMIC_MULTICAST_DELEGATE_FiveParams(
    FOnStatsUpdated,
    int32,  Score,
    float,  TotalTips,
    int32,  OrdersCorrect,
    int32,  OrdersWrong,
    int32,  OrdersExpired
);

UCLASS()
class SCRIPTED_SERVICE_API ACustomGameMode : public AGameModeBase
{
    GENERATED_BODY()

public:

    ACustomGameMode();

    // ── Tuning knobs (editable in Blueprint defaults / Details panel) ─────────

    /**
     * Points awarded for each correctly delivered order.
     * Shown to the player as the "score" counter.
     */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Scoring",
        meta = (ClampMin = "0"))
    int32 PointsPerCorrectDelivery = 100;

    /**
     * Points deducted for each wrong dish delivered.
     * Score is clamped to 0 — it will never go negative.
     */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Scoring",
        meta = (ClampMin = "0"))
    int32 PenaltyPerWrongDelivery = 25;

    /**
     * Points deducted when an order expires before the robot takes it.
     * Score is clamped to 0.
     */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Scoring",
        meta = (ClampMin = "0"))
    int32 PenaltyPerExpiredOrder = 50;

    /**
     * Tip amount (in dollars) added for a correct delivery.
     * Future-proofed: could be made dynamic based on how fast delivery was.
     */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Scoring",
        meta = (ClampMin = "0.0"))
    float TipPerCorrectDelivery = 5.0f;

    // ── Live stats (read-only from outside) ──────────────────────────────────

    /** Accumulated points for this session. Never goes below 0. */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Stats")
    int32 Score = 0;

    /** Total tips earned for this session (in dollars). */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Stats")
    float TotalTips = 0.0f;

    /** How many orders were delivered with the correct dish. */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Stats")
    int32 OrdersCorrect = 0;

    /** How many orders were delivered with the wrong dish. */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Stats")
    int32 OrdersWrong = 0;

    /** How many orders timed-out before the robot reached the table. */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Stats")
    int32 OrdersExpired = 0;

    // ── Event ────────────────────────────────────────────────────────────────

    /**
     * Fired every time any tracked stat changes.
     * Bind your HUD widget to this delegate so it refreshes automatically.
     *
     * Broadcast parameters mirror the five public stat properties above.
     */
    UPROPERTY(BlueprintAssignable, Category = "Stats|Events")
    FOnStatsUpdated OnStatsUpdated;

    // ── Blueprint-callable helpers ────────────────────────────────────────────

    /**
     * Manually reset all stats to zero and broadcast the reset.
     * Useful for a "Restart" or "New Game" button.
     */
    UFUNCTION(BlueprintCallable, Category = "Stats")
    void ResetStats();

    /**
     * Returns the current stats as a formatted debug string.
     * Handy for on-screen debug overlays during development.
     *
     * @return  Multi-line human-readable stats string.
     */
    UFUNCTION(BlueprintPure, Category = "Stats|Debug")
    FString GetStatsDebugString() const;

protected:

    virtual void BeginPlay() override;

private:

    /**
     * Iterates all ATableActor instances in the current level and binds
     * OnOrderDelivered / OnOrderExpired to this GameMode's handlers.
     * Called once from BeginPlay.
     */
    void SubscribeToAllTables();

    /**
     * Called when any table fires OnOrderDelivered.
     *
     * @param TableNumber  Which table the robot delivered to.
     * @param bCorrect     True → correct dish; False → wrong dish.
     */
    UFUNCTION()
    void HandleOrderDelivered(int32 TableNumber, bool bCorrect);

    /**
     * Called when any table fires OnOrderExpired.
     *
     * @param TableNumber  Which table's order timed out.
     */
    UFUNCTION()
    void HandleOrderExpired(int32 TableNumber);

    /**
     * Clamps Score to [0, MAX_int32] and then broadcasts OnStatsUpdated
     * with the current values of all five stat properties.
     * Every stat-mutating path must call this.
     */
    void BroadcastStats();
};