# VR Lesson System — Technical Documentation (Part 2)
### Lesson Steps: Class Reference & Usage Guide

> This document is a continuation of the main VR Lesson System documentation.  
> It covers the `LessonStep` base class, all concrete step subclasses, and the supporting types modules revealed in the second UML diagram.

---

## Table of Contents

1. [LessonStep (Base Class)](#1-lessonstep-base-class)
2. [Concrete Step Subclasses](#2-concrete-step-subclasses)
   - [AssembleStep](#21-assemblestep)
   - [FocusStep](#22-focusstep)
   - [InteractionStep](#23-interactionstep)
   - [QuizLessonStep](#24-quizlessonstep)
3. [Types Modules](#3-types-modules)
   - [LessonTypes](#31-lessontypes)
   - [QuizTypes](#32-quiztypes)
4. [Step Lifecycle In Detail](#4-step-lifecycle-in-detail)
5. [Adding a New Step Type](#5-adding-a-new-step-type)
6. [Relationship Summary (Part 2)](#6-relationship-summary-part-2)

---

## 1. LessonStep (Base Class)

`LessonStep` is the abstract base class for all lesson steps. It is instantiated at runtime from `FLessonStepData` (held in a `LessonDataAsset`) and managed by the `LessonManagerComponent`.

Every concrete step type inherits from this class and overrides `StartStep()`, `CheckCompletion()`, and `ResetStep()` to implement its own logic.

### Properties

| Property | Type | Description |
|---|---|---|
| `InstructionText` | `FString` | The instruction shown to the player on the `LessonUIActor` for this step. |
| `bStepCompleted` | `bool` | Flag set to `true` once the step has been successfully completed. |
| `PreviousStep` | `ULessonStep*` | Pointer to the preceding step; used for back-navigation or context. |
| `NextStep` | `ULessonStep*` | Pointer to the following step; set by `LessonManagerComponent` during initialization. |
| `OnStepCompleted` | delegate | Broadcast when this step finishes. `LessonManagerComponent` binds to this to call `HandleStepCompleted()`. |

### Methods

| Method | Returns | Description |
|---|---|---|
| `StartStep()` | `void` | Called by the manager when this step becomes active. Subclasses set up listeners and timers here. |
| `CompleteStep()` | `void` | Marks `bStepCompleted = true` and broadcasts `OnStepCompleted`. Call this from subclass logic when the condition is met. |
| `ResetStep()` | `void` | Resets state back to the beginning — useful if a step needs to be replayed or the lesson is restarted. |
| `CheckCompletion()` | `bool` | Returns whether the completion condition has been satisfied. Override in subclasses for custom logic. |

---

## 2. Concrete Step Subclasses

All four subclasses below inherit from `LessonStep` and add their own properties. They do **not** add new public methods beyond what the base class defines — completion is handled internally and surfaced through `OnStepCompleted`.

---

### 2.1 AssembleStep

Requires the player to physically assemble parts onto a target assembly actor in VR. Completion is triggered when the required parts are attached (and optionally fully assembled).

#### Properties

| Property | Type | Description |
|---|---|---|
| `AssemblyActorClass` | `TSubclassOf<AAssemblyActor>` | The class of the assembly actor the player must interact with. |
| `TargetPartClasses` | `TArray<TSubclassOf<APartActor>>` | The list of part actor classes that must be connected to satisfy this step. |
| `bRequireTargetConnected` | `bool` | If `true`, each part in `TargetPartClasses` must be physically connected to the assembly actor. |
| `bRequireFullyAssembled` | `bool` | If `true`, the assembly actor must report itself as fully assembled — not just partially. |

#### Usage Notes

- Use `bRequireTargetConnected` alone when only a subset of parts matters.
- Use `bRequireFullyAssembled` when the entire assembly must be complete before advancing.
- Both flags can be enabled together for stricter completion requirements.

---

### 2.2 FocusStep

Requires the player to look at (focus on) a specific actor for a minimum duration. Useful for directing attention to objects or areas in the VR environment.

#### Properties

| Property | Type | Description |
|---|---|---|
| `TargetActor` | `AActor*` | The actor the player must look at to complete the step. |
| `FocusAngleDegrees` | `float` | The maximum angle (in degrees) between the player's gaze direction and the target for it to count as focused. |
| `MinFocusTimeSeconds` | `float` | How long (in seconds) the player must continuously hold focus to complete the step. |
| `bRequireLineOfSight` | `bool` | If `true`, a line trace is performed to ensure no objects are occluding the target. |

#### Usage Notes

- Lower `FocusAngleDegrees` values require more precise head orientation.
- `bRequireLineOfSight` should generally be `true` in environments with walls or equipment that could block the view.
- Focus time resets if the player looks away before `MinFocusTimeSeconds` is reached.

---

### 2.3 InteractionStep

Requires the player to physically interact with a target actor a specified number of times. The interaction type (grab, press, activate, etc.) is defined by an `EInteractionType` enum value from `LessonTypes`.

#### Properties

| Property | Type | Description |
|---|---|---|
| `TargetActor` | `AActor*` | The actor the player must interact with. |
| `InteractionType` | `EInteractionType` | The kind of interaction required (e.g., Grab, Press, Activate). Defined in `LessonTypes`. |
| `RequiredCount` | `int32` | How many times the interaction must be performed to complete the step. |

#### Usage Notes

- Set `RequiredCount` to `1` for a single interaction (e.g., press a button once).
- For repeated actions (e.g., press a lever 3 times), increase `RequiredCount` accordingly.
- The target actor must implement the expected interaction interface for the step to detect it correctly.

---

### 2.4 QuizLessonStep

Presents the player with one or more quiz questions and evaluates their answers. This step uses the `QuizTypes` module for its data structures and tracks a running score.

> ⚠️ **Note:** `QuizLessonStep` is currently under active development and is **not yet merged into the main branch**. Do not use it in production lessons until it has been merged and tested.

#### Properties

| Property | Type | Description |
|---|---|---|
| `Questions` | `TArray` | The list of `FQuizQuestion` structs to present to the player. |
| `bRequirePassingScore` | `bool` | If `true`, the player must meet or exceed `PassingScorePercent` to complete the step. |
| `PassingScorePercent` | `float` | The minimum score percentage (0.0–1.0) required when `bRequirePassingScore` is enabled. |
| `QuizState` | `EQuizState` | Tracks the current state of the quiz (e.g., InProgress, Passed, Failed). Defined in `QuizTypes`. |
| `Score` | `int32` | The player's current running score across all answered questions. |

#### Methods

| Method | Returns | Description |
|---|---|---|
| `SubmitAnswer(int32 AnswerIndex)` | `void` | Called when the player selects an answer. Updates `Score` and advances to the next question. |
| `GetCurrentQuestion()` | `FQuizQuestion` | Returns the `FQuizQuestion` struct for whichever question is currently active. |

---

## 3. Types Modules

Two `«types»` modules appear in the diagram. These are not Blueprint classes — they are **C++ header files** (or a single combined header) that define the enums and structs shared across the lesson system.

---

### 3.1 LessonTypes

Defines the core enums and the step data struct used throughout the lesson system.

| Type | Kind | Description |
|---|---|---|
| `ELessonStepType` | `enum` | Identifies which concrete `LessonStep` subclass to instantiate (e.g., `Assemble`, `Focus`, `Interaction`, `Quiz`). |
| `EInteractionType` | `enum` | The kind of physical interaction required by an `InteractionStep` (e.g., `Grab`, `Press`, `Activate`). |
| `FLessonStepData` | `struct` | The data-only representation of a step stored inside a `LessonDataAsset`. Passed to `LessonManagerComponent::InitializeLessonFromDataAsset()` to construct runtime `ULessonStep` objects. |

#### FLessonStepData Fields (Recommended)

```cpp
USTRUCT(BlueprintType)
struct FLessonStepData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere) FString InstructionText;
    UPROPERTY(EditAnywhere) ELessonStepType StepType;

    // AssembleStep
    UPROPERTY(EditAnywhere) TSubclassOf<AAssemblyActor> AssemblyActorClass;
    UPROPERTY(EditAnywhere) TArray<TSubclassOf<APartActor>> TargetPartClasses;
    UPROPERTY(EditAnywhere) bool bRequireTargetConnected = false;
    UPROPERTY(EditAnywhere) bool bRequireFullyAssembled  = false;

    // FocusStep
    UPROPERTY(EditAnywhere) AActor* FocusTargetActor    = nullptr;
    UPROPERTY(EditAnywhere) float   FocusAngleDegrees   = 30.f;
    UPROPERTY(EditAnywhere) float   MinFocusTimeSeconds = 2.f;
    UPROPERTY(EditAnywhere) bool    bRequireLineOfSight = true;

    // InteractionStep
    UPROPERTY(EditAnywhere) AActor*           InteractionTargetActor = nullptr;
    UPROPERTY(EditAnywhere) EInteractionType  InteractionType;
    UPROPERTY(EditAnywhere) int32             RequiredCount = 1;

    // QuizLessonStep (not yet on main branch)
    UPROPERTY(EditAnywhere) TArray<FQuizQuestion> Questions;
    UPROPERTY(EditAnywhere) bool  bRequirePassingScore  = false;
    UPROPERTY(EditAnywhere) float PassingScorePercent   = 0.7f;
};
```

---

### 3.2 QuizTypes

Defines the data structures used exclusively by `QuizLessonStep`.

| Type | Kind | Description |
|---|---|---|
| `EQuizState` | `enum` | Current state of a quiz step: e.g., `NotStarted`, `InProgress`, `Passed`, `Failed`. |
| `FQuizQuestion` | `struct` | A single question, its answer choices, and the index of the correct answer. |
| `FQuizChoice` | `struct` | One answer choice within an `FQuizQuestion` — typically an index and display text. |
| `FQuizQuestionResult` | `struct` | The result of a single answered question: which answer was chosen and whether it was correct. |

#### Struct Relationships

```
FQuizQuestion
  └── TArray<FQuizChoice>   (the available answers)

QuizLessonStep
  ├── TArray<FQuizQuestion>        (all questions)
  └── TArray<FQuizQuestionResult>  (recorded answers, one per question answered)
```

---

## 4. Step Lifecycle In Detail

```
LessonManagerComponent
  │
  ├── InitializeLessonFromDataAsset()
  │     Reads each FLessonStepData, checks ELessonStepType,
  │     and instantiates the matching ULessonStep subclass.
  │     Sets PreviousStep / NextStep pointers to form a linked list.
  │
  └── StartLesson()
        │
        ▼
      [Active Step]::StartStep()
        │   Sets up interaction listeners, timers, gaze tracking, etc.
        │
        ▼
      [Active Step]::CheckCompletion()  ← polled each frame or event-driven
        │
        └── when true → CompleteStep()
                          │
                          ├── bStepCompleted = true
                          ├── OnStepCompleted.Broadcast()
                          │
                          ▼
                    LessonManagerComponent::HandleStepCompleted()
                          │
                          ├── LessonUIManagerComponent::UpdateProgress()
                          │
                          └── AdvanceStep()
                                │
                                ├── more steps → [Next Step]::StartStep()
                                └── no more   → OnLessonCompleted.Broadcast()
```

---

## 5. Adding a New Step Type

1. **Add an enum value** to `ELessonStepType` in `LessonTypes.h`.
2. **Add any new fields** to `FLessonStepData` for the data this step needs.
3. **Create a new C++ class** (e.g., `UMyNewStep`) that inherits from `ULessonStep` and overrides:
   - `StartStep()` — set up any listeners or timers
   - `CheckCompletion()` — return `true` when the condition is met
   - `ResetStep()` — tear down listeners and reset state
4. **Update `InitializeLessonFromDataAsset()`** in `LessonManagerComponent.cpp` to handle the new enum value and instantiate `UMyNewStep`.
5. **Populate the data asset** — open any `LessonDataAsset`, add a step entry, and select your new step type from the dropdown.

---

## 6. Relationship Summary (Part 2)

| From | To | Relationship |
|---|---|---|
| `LessonStep` | `AssembleStep` / `FocusStep` / `InteractionStep` / `QuizLessonStep` | base class — all four inherit from `LessonStep` |
| `LessonStep` | `LessonUIActor` | uses — calls `ShowInstruction()` to display `InstructionText` |
| `QuizLessonStep` | `QuizTypes` | uses — depends on `EQuizState`, `FQuizQuestion`, `FQuizChoice`, `FQuizQuestionResult` |
| `LessonStep` / subclasses | `LessonTypes` | uses — `ELessonStepType`, `EInteractionType`, and `FLessonStepData` are referenced across all step classes |
| `LessonDataAsset` | `FLessonStepData` | instances via — the asset owns a `TArray<FLessonStepData>` from which runtime steps are built |
