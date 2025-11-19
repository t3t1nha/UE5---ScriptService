# WARP.md

This file provides guidance to WARP (warp.dev) when working with code in this repository.

## Project overview

- Unreal Engine 5 project named `Scripted_Service` (`Scripted_Service.uproject`, `EngineAssociation` 5.6).
- Primary runtime C++ module: `Scripted_Service` (in `Source/Scripted_Service`).
- Custom plugin: `CookingSystemTypes` (in `Plugins/CookingSystemTypes`), providing shared enums and data structs for the cooking system.
- Gameplay is a first-person experience built on Enhanced Input and a data-driven cooking/recipe system.

## Common workflows & commands

### Open the project in Unreal Editor (Windows)

Use your local UE5 install path and the `.uproject` file. From PowerShell:

```powershell
& "<UE5_INSTALL_ROOT>\Engine\Binaries\Win64\UnrealEditor.exe" "C:\\Users\\IcePa\\Desktop\\UE5---ScriptService\\Scripted_Service.uproject"
```

Replace `<UE5_INSTALL_ROOT>` with your actual UE5.6 installation directory.

### Open the project in Rider

The primary workflow is to open the `.uproject` directly in Rider:
- In Rider, choose **Open** and select `Scripted_Service.uproject`.
- Rider will generate and manage the underlying project files as needed.

### Build the C++ code

**Via Rider (preferred):**
- Open `Scripted_Service.uproject` in Rider.
- Use Rider’s Unreal toolbar or build configuration dropdown to build the `Scripted_ServiceEditor` target in `Development Editor | Win64`.

**Optional: generate Visual Studio project files**

If you ever want to work in Visual Studio or need a `.sln` for tooling:

```powershell
& "<UE5_INSTALL_ROOT>\Engine\Build\BatchFiles\GenerateProjectFiles.bat" `
  -project="C:\\Users\\IcePa\\Desktop\\UE5---ScriptService\\Scripted_Service.uproject" `
  -game -engine
```

This creates a `.sln` and associated project files for editing and building in Visual Studio. Once generated, you can also build from the command line:

```powershell
& "<MSBUILD_PATH>\MSBuild.exe" "Scripted_Service.sln" `
  /p:Configuration="Development Editor" `
  /p:Platform="Win64"
```

Substitute `<MSBUILD_PATH>` with your local MSBuild location (for example from a Visual Studio Developer Prompt).

### Running the game

The typical flow is to run PIE (Play In Editor) from within Unreal Editor after building:
- Open the project in Unreal Editor.
- Press the Play button in the toolbar to start the first-person experience using `APlayer_Character`.

### Linting / formatting

There is no explicit linter or formatter configuration checked into this repository (no `clang-format`, `cpplint` config, etc.). Use your IDE/Unreal defaults or project-specific conventions when modifying C++ files.

### Tests

There are currently no automated test targets or test harnesses defined in this repository (no `AutomationSpec` C++ tests or dedicated test modules). Running a “single test” is not applicable yet; testing is done manually via PIE in Unreal Editor.

## High-level architecture

### Modules and folders

- `Source/Scripted_Service`
  - Root C++ game module (`Scripted_Service.Build.cs`).
  - Implements the primary game module in `Scripted_Service.cpp`.
  - Contains gameplay classes such as the player character and cooking-system actors.
- `Plugins/CookingSystemTypes`
  - Standalone module defining shared gameplay types used by the main game module.
  - Designed so that item data and recipes live in data tables and are consumed by the runtime code.
- `Content`
  - Blueprint, level, and data assets, including:
    - First-person template blueprints (`Content/FirstPerson/Blueprints`).
    - Cooking-related blueprints and data (`Content/Kitchen`, `Content/Kitchen1/Interactables`, `Content/Kitchen/Data`).
- `Config`
  - Engine and input configuration (`DefaultInput.ini`, `DefaultEngine.ini`, etc.).
  - `DefaultInput.ini` is configured for Enhanced Input (e.g., `DefaultPlayerInputClass=/Script/EnhancedInput.EnhancedPlayerInput`).

### CookingSystemTypes plugin (data model)

Located in `Plugins/CookingSystemTypes/Source/CookingSystemTypes`:

- `CookingSystemTypes.h` defines the fundamental data model:
  - `ECookState`: cooking state progression for items (Raw, Chopped, Cooked, Boiled, Fried, Burnt, etc.).
  - `EApparatusType`: types of cooking apparatus (Grill, Stove, Oven, CuttingBoard, Plate, WaterPot).
  - `EItemType`: high-level item categories (Vegetable, Meat, Condiment, Liquid, CookedDish, Apparatus).
  - `FItemBaseData` (`USTRUCT` deriving from `FTableRowBase`):
    - Holds static data per item: type, display name/description, mesh, default cook state.
  - `FRecipeData` (`USTRUCT` deriving from `FTableRowBase`):
    - `RequiredApparatus` (`EApparatusType`).
    - `RequiredIngredients` (`TMap<FName, int32>`): item IDs + counts.
    - `RequiredCookState` (`TMap<FName, ECookState>`): optional cook states per ingredient.
    - `OutputItemID` and `BaseCookTime` for generated results.

These structs are intended to be referenced from data tables (`DT_ItemBaseData`, `DT_RecipeData` in `Content/Kitchen/Data`). The runtime game code in `Scripted_Service` uses these tables to drive behavior.

### Scripted_Service game module (runtime)

#### Player character and interaction

- `APlayer_Character` (`Player_Character.h/.cpp`):
  - Extends `ACharacter` and is set up as a first-person pawn.
  - Components:
    - `FirstPersonMeshComponent` (`USkeletalMeshComponent`) used for the player’s first-person body/arms.
    - `FirstPersonCameraComponent` (`UCameraComponent`) attached to the `head` bone of the first-person mesh, with first-person FOV/scale enabled.
    - `PhysicsHandleComponent` (`UPhysicsHandleComponent`) used for grabbing and holding physics-simulated objects in front of the camera.
  - Input (Enhanced Input):
    - Uses `UInputMappingContext` and several `UInputAction` instances (Move, Look, Jump, Interact).
    - In `BeginPlay`, retrieves the local `UEnhancedInputLocalPlayerSubsystem` and adds the `InputMappingContext`.
    - `SetupPlayerInputComponent` binds actions via `UEnhancedInputComponent`:
      - `MoveAction` → `Move(const FInputActionValue&)` (WASD/gamepad movement using `AddMovementInput`).
      - `LookAction` → `Look(const FInputActionValue&)` (mouse/gamepad look using `AddControllerYawInput`/`AddControllerPitchInput`).
      - `JumpAction` → `ACharacter::Jump` / `StopJumping`.
      - `InteractAction` → `Interact()`.
  - Interaction flow in `Interact()`:
    - Performs a line trace from the camera forward up to `InteractDistance`.
    - If already holding an item (`bIsHoldingItem`), releases it via `PhysicsHandleComponent->ReleaseComponent()`.
    - Otherwise, if the hit actor implements `UGrabableInterface`, it:
      - Uses `PhysicsHandleComponent->GrabComponentAtLocationWithRotation` to attach the hit component.
      - Sets `bIsHoldingItem = true` and, on each `Tick`, keeps the held object at `GrabDistance` in front of the camera.
    - If the hit actor instead implements `UInteractInterface`, it casts to `AAApparatusActor` and calls `Interact_Implementation()` on the apparatus.

This creates a clear separation between “grab anything physically grabable” (via `GrabableInterface`) and “trigger scripted interaction” (via `InteractInterface`).

#### Cooking apparatus and ingredients

- `ABaseIngredient` (`Kitchen/BaseIngredient.h/.cpp`):
  - `AActor` implementing `IGrabableInterface`.
  - Owns `ItemMeshComponent` (`UStaticMeshComponent`) as root, with physics simulation and `PhysicsActor` collision.
  - Exposes `ItemID` (`FName`) and `ItemBaseDataTable` (`UDataTable*`, with row type `FItemBaseData`).
  - On construction (`OnConstruction`), calls `UpdateVisuals()`:
    - Looks up the `FItemBaseData` row for `ItemID` and, if found, sets the static mesh on `ItemMeshComponent`.
  - This makes ingredients fully data-driven: visuals and metadata come from `DT_ItemBaseData`.

- `AAApparatusActor` (`Kitchen/AApparatusActor.h/.cpp`):
  - `AActor` implementing `IInteractInterface`.
  - Components:
    - `ApparatusMeshComponent` (`UStaticMeshComponent`) as root.
    - `DropZoneComponent` (`UBoxComponent`) attached to the mesh, with `OverlapAll` collision; used as the trigger zone for ingredient placement.
  - Core state:
    - `ApparatusType` (`EApparatusType`): what kind of cooking station this is (grill, stove, etc.).
    - `CurrentIngredients` (`TMap<FName, int32>`): counts of ingredient IDs currently in the drop zone.
    - `CurrentIngredientActors` (`TArray<ABaseIngredient*>`): live ingredient actors inside the apparatus.
    - `RecipeDataTable` (`UDataTable*`, row type `FRecipeData`) and `ItemBaseDataTable` (`UDataTable*`, row type `FItemBaseData`).
    - `IngredientBPClass`: generic `ABaseIngredient` subclass to spawn for outputs.
    - `CurrentRecipeData` (`FRecipeData`) and `CookingTimerHandle` (`FTimerHandle`).
  - Overlap handling:
    - `OnDropZoneOverlapBegin` / `OnDropZoneOverlapEnd` are bound in `BeginPlay`.
    - When an `ABaseIngredient` enters the drop zone:
      - Validates `ItemID`.
      - Calls `AddIngredient`, which increments the corresponding entry in `CurrentIngredients`, appends to `CurrentIngredientActors`, and calls `CheckForRecipe()`.
    - When an `ABaseIngredient` leaves the drop zone:
      - Calls `RemoveIngredient`, which decrements/removes the map entry, updates `CurrentIngredientActors`, and calls `CheckForRecipe()`.
  - Recipe matching in `CheckForRecipe()`:
    - Iterates `RecipeDataTable` rows.
    - Filters out recipes that:
      - Require a different `ApparatusType`.
      - Have a different ingredient-count size than `CurrentIngredients`.
    - For each candidate recipe, verifies:
      - Every required ingredient key is present in `CurrentIngredients` with exactly the required quantity.
      - `CurrentIngredients` does not contain any extra keys beyond those in `RequiredIngredients`.
    - On match, sets `CurrentRecipeData` and logs the recipe name; otherwise logs “No Recipe Found”.
  - Cooking flow:
    - `Interact_Implementation()` (called via `IInteractInterface`) simply calls `StartCookingProcess()` and logs an on-screen message.
    - `StartCookingProcess()`:
      - If `CurrentRecipeData` has a valid `OutputItemID` and positive `BaseCookTime`, sets a one-shot timer on `CookingTimerHandle` to call `FinishCooking()` after `BaseCookTime` seconds.
      - If a timer is already active, logs that cooking is already in progress.
      - Otherwise, logs that cooking did not start.
    - `FinishCooking()`:
      - Clears the timer.
      - Looks up `FItemBaseData` for `CurrentRecipeData.OutputItemID` from `ItemBaseDataTable`.
      - Spawns a new `ABaseIngredient` using `IngredientBPClass` slightly above the drop zone, sets its `ItemID` to `OutputItemID`, and calls `UpdateVisuals()` to configure the mesh.
      - Destroys all `CurrentIngredientActors` and clears both ingredient collections and `CurrentRecipeData`.

This creates a tightly coupled but clear pipeline:

`ABaseIngredient` (data-driven physical items) → Overlap with `AAApparatusActor` drop zone → `CurrentIngredients`/recipe selection → `StartCookingProcess` timer → `FinishCooking` spawns result and consumes inputs.

### Interfaces

- `GrabableInterface` (`Kitchen/Interface/GrabableInterface.h/.cpp`):
  - Marker interface used to indicate actors can be grabbed by the player via `PhysicsHandleComponent`.
  - `ABaseIngredient` implements this.
- `InteractInterface` (`Kitchen/Interface/InteractInterface.h/.cpp`):
  - Declares a Blueprint-native event `Interact()`.
  - `AAApparatusActor` implements `Interact_Implementation()` to trigger the cooking process when the player interacts.

### Input configuration and assets

- `Config/DefaultInput.ini`:
  - Sets Enhanced Input-related defaults (`DefaultPlayerInputClass`, `DefaultInputComponentClass`).
  - Defines axis configs for mouse/gamepad/VR devices.
- `Content/Input/Actions` and `Content/Input/IMC_Default.uasset`:
  - Contain specific `UInputAction` and input mapping context assets referenced by `APlayer_Character`.

### Blueprint layer

- First-person character/game mode/controller blueprints under `Content/FirstPerson/Blueprints`:
  - Wrap or extend the C++ `APlayer_Character` and configure visuals, animations, and game mode behavior.
- Cooking-specific blueprints under `Content/Kitchen` and `Content/Kitchen1/Interactables`:
  - Placeable instances of `ABaseIngredient` and `AAApparatusActor` (e.g., grill, hotdog, raw/cooked beef).
  - Associated data tables (`DT_ItemBaseData`, `DT_RecipeData`) that plug into the `CookingSystemTypes` structs.

These blueprints and data assets are where level designers wire up new recipes, items, and apparatuses without requiring C++ changes, as long as new content fits within the existing `CookingSystemTypes` data model.
