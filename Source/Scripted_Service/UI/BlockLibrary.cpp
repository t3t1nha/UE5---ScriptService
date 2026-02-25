// Fill out your copyright notice in the Description page of Project Settings.


#include "BlockLibrary.h"

TArray<FBlockData> UBlockLibrary::GetDefaultBlocks()
{
    TArray<FBlockData> Blocks;
    
    // Move to Table block
    FBlockData MoveToTable;
    MoveToTable.InstructionType = EInstructionType::MoveToTable;
    MoveToTable.DisplayName = FText::FromString("Move to Table");
    MoveToTable.BlockColor = FLinearColor(0.2f, 0.5f, 1.0f);
    MoveToTable.Category = "Movement";
    MoveToTable.bHasTableParameter = true;
    Blocks.Add(MoveToTable);
    
    // Move to Kitchen block
    FBlockData MoveToKitchen;
    MoveToKitchen.InstructionType = EInstructionType::MoveToKitchen;
    MoveToKitchen.DisplayName = FText::FromString("Move to Kitchen");
    MoveToKitchen.BlockColor = FLinearColor(0.2f, 0.5f, 1.0f); 
    MoveToKitchen.Category = "Movement";
    Blocks.Add(MoveToKitchen);
    
    // Take Order block
    FBlockData TakeOrder;
    TakeOrder.InstructionType = EInstructionType::TakeOrder;
    TakeOrder.DisplayName = FText::FromString("Take Order");
    TakeOrder.BlockColor = FLinearColor(1.0f, 0.7f, 0.2f); 
    TakeOrder.Category = "Actions";
    TakeOrder.bHasTableParameter = true;
    Blocks.Add(TakeOrder);
    
    // Pickup Food block
    FBlockData PickupFood;
    PickupFood.InstructionType = EInstructionType::PickupFood;
    PickupFood.DisplayName = FText::FromString("Pickup Food");
    PickupFood.BlockColor = FLinearColor(1.0f, 0.7f, 0.2f); 
    PickupFood.Category = "Actions";
    Blocks.Add(PickupFood);
    
    // Deliver Order block
    FBlockData DeliverOrder;
    DeliverOrder.InstructionType = EInstructionType::DeliverOrder;
    DeliverOrder.DisplayName = FText::FromString("Deliver Order");
    DeliverOrder.BlockColor = FLinearColor(0.3f, 1.0f, 0.3f);
    DeliverOrder.Category = "Actions";
    Blocks.Add(DeliverOrder);
    
    // Wait block
    FBlockData Wait;
    Wait.InstructionType = EInstructionType::Wait;
    Wait.DisplayName = FText::FromString("Wait");
    Wait.BlockColor = FLinearColor(0.7f, 0.7f, 0.7f); 
    Wait.Category = "Flow";
    Wait.bHasWaitParameter = true;
    Blocks.Add(Wait);
    
    return Blocks;
}