# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

VanguardArena is an **Unreal Engine 5.7** C++ game project (single `VanguardArena` runtime module). It is structured as a multi-variant game template with three independent gameplay variants sharing a single module:

- **Variant_Combat** — Third-person melee combat with combos, charged attacks, AI enemies, and a health system
- **Variant_Platforming** — Third-person platformer with double jump, wall jump, coyote time, and dash
- **Variant_SideScrolling** — 2.5D side-scroller with wall jump, soft platforms (drop-through), NPC AI, and pickups

Each variant has its own `Character`, `GameMode`, and `PlayerController` classes that are **not** derived from the base classes in the root (`VanguardArenaCharacter`, `VanguardArenaGameMode`, `VanguardArenaPlayerController`). The base classes in the root serve as a simple standalone third-person template.

## Building

Build via **Unreal Build Tool** from the Unreal Editor or Visual Studio solution (`VanguardArena.sln`). There is no standalone CLI build command — use the Editor's **Build** button or **Development Editor** configuration in Visual Studio.

Key modules declared in `VanguardArena.Build.cs`:
- `EnhancedInput` — all input handling uses Enhanced Input
- `AIModule`, `StateTreeModule`, `GameplayStateTreeModule` — AI and StateTree
- `UMG`, `Slate` — UI widgets

All `Source/VanguardArena/Variant_*` subdirectories are added to `PublicIncludePaths`, so headers can be included without path prefixes within the module.

## Architecture

### Base Layer (`Source/VanguardArena/`)
- `AVanguardArenaCharacter` — abstract base; third-person character with SpringArm/Camera and Enhanced Input (Move, Look, Jump). Exposes `DoMove`, `DoLook`, `DoJumpStart/End` as `BlueprintCallable` for mobile UI bridging.
- `AVanguardArenaPlayerController` — abstract base; manages `DefaultMappingContexts` and `MobileExcludedMappingContexts`. Optionally spawns a UMG touch controls widget (`bForceTouchControls`).
- `AVanguardArenaGameMode` — minimal abstract base.

### Variant_Combat
The combat variant's `ACombatCharacter` does **not** extend `AVanguardArenaCharacter` — it inherits directly from `ACharacter` and implements two interfaces:
- `ICombatAttacker` — `DoAttackTrace`, `CheckCombo`, `CheckChargedAttack` (called via AnimNotifies)
- `ICombatDamageable` — `ApplyDamage`, `HandleDeath`, `ApplyHealing`, `NotifyDanger`

`ACombatEnemy` is a separate character class also implementing both interfaces, controlled by `ACombatAIController` which runs a `UStateTreeAIComponent`. Custom StateTree tasks/conditions are defined in `CombatStateTreeUtility.h`:
- Tasks: `FStateTreeComboAttackTask`, `FStateTreeChargedAttackTask`, `FStateTreeWaitForLandingTask`, `FStateTreeFaceActorTask`, `FStateTreeFaceLocationTask`, `FStateTreeSetCharacterSpeedTask`, `FStateTreeGetPlayerInfoTask`
- Conditions: `FStateTreeCharacterGroundedCondition`, `FStateTreeIsInDangerCondition`

Combat mechanics use **AnimNotifies** to trigger attack logic at the right frame:
- `AnimNotify_DoAttackTrace` — fires sphere trace for hit detection
- `AnimNotify_CheckCombo` — advances or ends the combo string
- `AnimNotify_CheckChargedAttack` — determines whether to loop the charge or release

The `ICombatActivatable` interface (`ToggleInteraction`, `ActivateInteraction`, `DeactivateInteraction`) is used by gameplay actors like `ACombatActivationVolume`, `ACombatCheckpointVolume`, `ACombatDamageableBox`, `ACombatLavaFloor`, and `ACombatDummy`.

`ACombatEnemySpawner` manages enemy spawning. `UCombatLifeBar` is a UMG `UserWidget` subclass that exposes `SetLifePercentage` and `SetBarColor` as `BlueprintImplementableEvent`s.

EQS context classes (`EnvQueryContext_Player`, `EnvQueryContext_Danger`) are provided for use in EQS queries from StateTree.

### Variant_Platforming
`APlatformingCharacter` extends `ACharacter` directly. Features:
- Wall jump — sphere trace forward, impulse away from wall, reset timer between wall jumps
- Double jump — tracked with `bHasDoubleJumped`
- Coyote time — tracks `LastFallTime` and allows a regular jump within `MaxCoyoteTime` after walking off a ledge
- Dash — plays `DashMontage`, ended via `AnimNotify_EndDash` which calls `EndDash()`

All state flags are packed in a `uint8` bitfield. Landing resets all flags.

### Variant_SideScrolling
`ASideScrollingCharacter` constrains movement to a single horizontal axis. Features:
- Wall jump, double jump, coyote time (same patterns as Platforming variant)
- Soft platforms — custom collision channel (`SoftCollisionObjectType`) toggled via `SetSoftCollision()` to enable drop-through
- Interaction system via `ISideScrollingInteractable` interface

`ASideScrollingCameraManager` extends `APlayerCameraManager` and overrides `UpdateViewTarget` to implement smooth horizontal scrolling with world-space X bounds (`CameraXMinBounds`/`CameraXMaxBounds`) and vertical adjustment only when necessary.

`ASideScrollingGameMode` tracks pickup count and manages the `USideScrollingUI` widget. `ASideScrollingNPC` implements `ISideScrollingInteractable` and can be temporarily stunned/deactivated. AI uses `ASideScrollingAIController` with StateTree tasks in `SideScrollingStateTreeUtility.h`.

## Key Patterns

- **Mobile input bridging**: All character classes expose `Do*` methods as `BlueprintCallable` so UMG touch control widgets can drive the same input paths as hardware input.
- **Abstract base classes**: All C++ classes that need Blueprint subclasses use `UCLASS(abstract)` — Blueprint subclasses provide mesh, animations, and montage assignments.
- **StateTree for AI**: AI behavior is driven entirely by StateTree (`UStateTreeAIComponent` on the AIController). Custom tasks and conditions follow the pattern: `FStateTreeXxxInstanceData` struct + `FStateTreeXxxTask`/`FStateTreeXxxCondition` struct with `using FInstanceDataType = ...` and `GetInstanceDataType()` override.
- **AnimNotify-driven attack timing**: Attack collision, combo chaining, and charge loop checks happen via AnimNotifies on montages rather than tick-based logic.