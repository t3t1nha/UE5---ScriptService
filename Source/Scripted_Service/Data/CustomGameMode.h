// ScriptedServiceGameMode.h
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "TableActor.h"
#include "CustomGameMode.generated.h"

class ATableManager;

//  @param Score            Current cumulative score (points)
//  @param TotalTips        Total tips earned so far (dollars, float)
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
    FOnStatsUpdated,
    int32,  Score,
    float,  TotalTips
);


UCLASS()
class SCRIPTED_SERVICE_API ACustomGameMode : public AGameModeBase
{
    GENERATED_BODY()

public:

    ACustomGameMode();

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Scoring",
        meta = (ClampMin = "0"))
    int32 PointsPerCorrectDelivery = 100;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Scoring",
        meta = (ClampMin = "0"))
    int32 PenaltyPerExpiredOrder = 25;
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Stats")
    int32 Score = 0;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Stats")
    float TotalTips = 20.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    int MaxNumberOfOrders = 5;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    int ActiveOrders = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    int NumberOfTables;
    
    UPROPERTY(BlueprintAssignable, Category = "Stats|Events")
    FOnStatsUpdated OnStatsUpdated;

    UFUNCTION(BlueprintCallable)
    void ReorderTables();
    
    UFUNCTION(BlueprintCallable, Category = "Stats")
    void ResetStats();
    UFUNCTION(BlueprintCallable)
    bool IncreaseOrderCount();

    /**
    * Attempts to spend the given amount from TotalTips.
    * @param Amount - Amount to spend
    * @return true if player had enough money and it was spent, false otherwise
    */
    UFUNCTION(BlueprintCallable, Category = "Stats")
    bool SpendMoney(float Amount);

    UFUNCTION(BlueprintCallable, Category = "Stats")
    void UpdateMaxNumberOfOrder(int value);

protected:

    virtual void BeginPlay() override;

private:

    ATableManager* TableManager;
    
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
    void HandleOrderDelivered(int32 TableNumber, float IngredientPrice);

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