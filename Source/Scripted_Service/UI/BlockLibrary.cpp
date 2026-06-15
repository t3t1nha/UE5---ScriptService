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
    MoveToTable.bCanReadTableFromSlot = true;
    MoveToTable.bCanSaveToSlot        = true; 
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
    MoveToTable.bCanReadTableFromSlot = true;
    MoveToTable.bCanSaveToSlot        = true; 
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

    FBlockData IfTableHasOrder;
    IfTableHasOrder.InstructionType    = EInstructionType::IfTableHasOrder;
    IfTableHasOrder.DisplayName        = FText::FromString("If Table Has Order");
    IfTableHasOrder.BlockColor         = FLinearColor(1.0f, 0.85f, 0.1f);
    IfTableHasOrder.Category           = "Logic";
    IfTableHasOrder.bIsContainerBlock  = true;
    IfTableHasOrder.bHasTableParameter = true;
    IfTableHasOrder.bCanReadTableFromSlot = true;
    Blocks.Add(IfTableHasOrder);

    FBlockData IfCarrying;
    IfCarrying.InstructionType   = EInstructionType::IfCarryingDish;
    IfCarrying.DisplayName       = FText::FromString("If Carrying Dish");
    IfCarrying.BlockColor        = FLinearColor(1.0f, 0.85f, 0.1f);
    IfCarrying.Category          = "Logic";
    IfCarrying.bIsContainerBlock = true;
    Blocks.Add(IfCarrying);

    // FBlockData IfNotCarrying;
    // IfNotCarrying.InstructionType   = EInstructionType::IfNotCarryingDish;
    // IfNotCarrying.DisplayName       = FText::FromString("If NOT Carrying Dish");
    // IfNotCarrying.BlockColor        = FLinearColor(1.0f, 0.85f, 0.1f);
    // IfNotCarrying.Category          = "Logic";
    // IfNotCarrying.bIsContainerBlock = true;
    // Blocks.Add(IfNotCarrying);

    FBlockData RepeatLoop;
    RepeatLoop.InstructionType        = EInstructionType::RepeatLoop;
    RepeatLoop.DisplayName            = FText::FromString("Repeat");
    RepeatLoop.BlockColor             = FLinearColor(0.8f, 0.3f, 1.0f);
    RepeatLoop.Category               = "Logic";
    RepeatLoop.bIsContainerBlock      = true;
    RepeatLoop.bHasLoopCountParameter = true;
    Blocks.Add(RepeatLoop);

    FBlockData LoopForever;
    LoopForever.InstructionType   = EInstructionType::LoopForever;
    LoopForever.DisplayName       = FText::FromString("Loop Forever");
    LoopForever.BlockColor        = FLinearColor(0.8f, 0.3f, 1.0f);
    LoopForever.Category          = "Logic";
    LoopForever.bIsContainerBlock = true;
    Blocks.Add(LoopForever);

    // FBlockData SetSlotBlock;
    // SetSlotBlock.InstructionType       = EInstructionType::SetSlot;
    // SetSlotBlock.DisplayName           = FText::FromString(TEXT("Set Slot"));
    // SetSlotBlock.BlockColor            = FLinearColor(0.3f, 0.85f, 0.55f); 
    // SetSlotBlock.Category              = "Memory";
    // SetSlotBlock.bHasSlotIndexParameter = true; 
    // SetSlotBlock.bHasSlotValueParameter = true; 
    // Blocks.Add(SetSlotBlock);
    // 
    // FBlockData IncrSlotBlock;
    // IncrSlotBlock.InstructionType        = EInstructionType::IncrementSlot;
    // IncrSlotBlock.DisplayName            = FText::FromString(TEXT("Increment Slot"));
    // IncrSlotBlock.BlockColor             = FLinearColor(0.3f, 0.85f, 0.55f);
    // IncrSlotBlock.Category               = "Memory";
    // IncrSlotBlock.bHasSlotIndexParameter = true;
    // Blocks.Add(IncrSlotBlock);
    // 
    // FBlockData DecrSlotBlock;
    // DecrSlotBlock.InstructionType        = EInstructionType::DecrementSlot;
    // DecrSlotBlock.DisplayName            = FText::FromString(TEXT("Decrement Slot"));
    // DecrSlotBlock.BlockColor             = FLinearColor(0.3f, 0.85f, 0.55f);
    // DecrSlotBlock.Category               = "Memory";
    // DecrSlotBlock.bHasSlotIndexParameter = true;
    // Blocks.Add(DecrSlotBlock);
    // 
    // FBlockData IfSlotEqBlock;
    // IfSlotEqBlock.InstructionType        = EInstructionType::IfSlotEquals;
    // IfSlotEqBlock.DisplayName            = FText::FromString(TEXT("If Slot = Value"));
    // IfSlotEqBlock.BlockColor             = FLinearColor(1.0f, 0.85f, 0.1f);
    // IfSlotEqBlock.Category               = "Memory";
    // IfSlotEqBlock.bIsContainerBlock      = true;
    // IfSlotEqBlock.bHasSlotIndexParameter = true;
    // IfSlotEqBlock.bHasSlotValueParameter = true;
    // Blocks.Add(IfSlotEqBlock);
    // 
    // FBlockData IfSlotGtBlock;
    // IfSlotGtBlock.InstructionType        = EInstructionType::IfSlotGreaterThan;
    // IfSlotGtBlock.DisplayName            = FText::FromString(TEXT("If Slot > Value"));
    // IfSlotGtBlock.BlockColor             = FLinearColor(1.0f, 0.85f, 0.1f);
    // IfSlotGtBlock.Category               = "Memory";
    // IfSlotGtBlock.bIsContainerBlock      = true;
    // IfSlotGtBlock.bHasSlotIndexParameter = true;
    // IfSlotGtBlock.bHasSlotValueParameter = true;
    // Blocks.Add(IfSlotGtBlock);
    // 
    // FBlockData IfSlotLtBlock;
    // IfSlotLtBlock.InstructionType        = EInstructionType::IfSlotLessThan;
    // IfSlotLtBlock.DisplayName            = FText::FromString(TEXT("If Slot < Value"));
    // IfSlotLtBlock.BlockColor             = FLinearColor(1.0f, 0.85f, 0.1f);
    // IfSlotLtBlock.Category               = "Memory";
    // IfSlotLtBlock.bIsContainerBlock      = true;
    // IfSlotLtBlock.bHasSlotIndexParameter = true;
    // IfSlotLtBlock.bHasSlotValueParameter = true;
    // Blocks.Add(IfSlotLtBlock);
     
    FBlockData IfKitchenBlock;
    IfKitchenBlock.InstructionType   = EInstructionType::IfKitchenHasOrder;
    IfKitchenBlock.DisplayName       = FText::FromString(TEXT("If Kitchen Ready"));
    IfKitchenBlock.BlockColor        = FLinearColor(1.0f, 0.5f, 0.15f);
    IfKitchenBlock.Category          = "Logic";
    IfKitchenBlock.bIsContainerBlock = true;
    Blocks.Add(IfKitchenBlock);
    
    FBlockData IfWaitBlock;
    IfWaitBlock.InstructionType        = EInstructionType::IfTableWaitingTooLong;
    IfWaitBlock.DisplayName            = FText::FromString(TEXT("If Table Waiting Too Long"));
    IfWaitBlock.BlockColor             = FLinearColor(1.0f, 0.5f, 0.15f);
    IfWaitBlock.Category               = "Logic";
    IfWaitBlock.bIsContainerBlock      = true;
    IfWaitBlock.bHasTableParameter     = true;
    IfWaitBlock.bCanReadTableFromSlot  = true; 
    IfWaitBlock.bHasSlotValueParameter = true;  
    Blocks.Add(IfWaitBlock);
    
    return Blocks;
}