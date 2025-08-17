# 🎓 VR Mechatronics Interactive Lesson System

**Author**: [Your Name]  
**Engine**: Unreal Engine 5.x (C++ + Blueprints)  
**Purpose**: A fully physics-based VR educational environment where students learn mechatronics through interactive, step-by-step device assembly.

---

## 📘 High-Level Summary

This project simulates mechanical and electrical components of real-world devices (e.g., a DC Motor) in VR, allowing students to physically assemble parts, visualize behavior, and complete guided lessons. The goal is to create a modular, interactive, and educationally valuable experience.

---

## 🔧 Core Design Principles

- **Physics-Based**: All parts simulate physics, collisions, and constraints.
- **Modular**: Parts, behaviors, and lessons are reusable and data-driven.
- **VR-Interactive**: Users interact via motion controllers (grab, assemble, press).
- **Lesson-Driven**: Each level is a standalone educational lesson.
- **Performance-Safe**: Avoids shader stutter and runtime loading hitches.

---

## 📂 Project Structure

### 📁 Levels

Each `.umap` file is a separate lesson:

- `L_Lesson_01_Intro.umap`
- `L_Lesson_02_ArmatureAssembly.umap`
- ...

### 📁 DataAssets

Each lesson is configured via:

- `DA_Lesson_01.uasset`
- `DA_Lesson_02.uasset`
- ...

---

## ⚙️ System Architecture

### 1. GameInstance

Global state and flow manager:

- `ActiveLessonData` (UDataAsset)
- `LoadNextLesson()`, `StartLesson(FName)`
- Persistent across level transitions

---

### 2. LessonManager (`ALessonManager`)

Spawned per level. Controls:

- `TArray<ULessonStep*>` (steps for current lesson)
- `AdvanceStep()`, `CheckStepCompletion()`

**Step Types:**

- `UAssembleStep`: Waits for part snap
- `UFocusStep`: Waits for player to focus on object
- `UInteractionStep`: Waits for UI or tool use

---

### 3. Assembly System

**`AAssemblyActor`**

- Contains `TArray<APartActor*>`
- Handles physics constraints
- Validates assembled state

**`APartActor`**

- `UStaticMeshComponent` (SimulatePhysics = true)
- `UAssemblyComponent` (logical metadata)
  - `SnapPoints[]` (`USnapPointComponent`)
  - `BehaviorComponents[]` (`UComponentBehavior`)

**Constraints:**

- Created using `UPhysicsConstraintComponent`
- Types: Fixed, Hinge, Prismatic
- Configured based on SnapPoint metadata

---

### 4. Behavior System

**`UComponentBehavior` (ActorComponent)**

Attached to parts to trigger logic such as:

- Spinning motors
- Visualizing magnetic fields
- Showing educational overlays

Functions:

- `OnAssembled()`
- `OnActivated()`
- `OnDisassembled()`

---

### 5. Interaction System

- VR motion controllers grab physics-enabled parts
- Snap validation via overlapping `SnapPoints`
- Valid connections trigger constraint creation

---

## 🔄 Runtime Flow

```plaintext
GameInstance
   ↓
Load Lesson Level (.umap)
   ↓
LessonManager reads DA_Lesson_XX
   ↓
Player interacts (grab, assemble, focus)
   ↓
SnapPoints matched → constraint created
   ↓
ComponentBehavior triggers
   ↓
LessonStep marked complete
   ↓
Advance to next step or next level
```

---

## 🔩 Physics Constraint Types

| From       | To         | Type   | Purpose                  |
|------------|------------|--------|--------------------------|
| Armature   | Shaft      | Hinge  | Allows motor rotation    |
| Shaft      | Housing    | Fixed  | Holds motor in place     |
| Commutator | Armature   | Fixed  | Rotates with motor       |
| Brush      | Commutator | None   | Overlap triggers logic   |
| Magnet     | Housing    | Fixed  | Static field component   |

---

## 🧪 Shader Precompilation & Stutter Avoidance

### Problems to Avoid

- Shader hitches on first use
- Runtime loading stutter

### Solutions

1. **Warm-Up Level (`L_WarmupShaders`)**
   - Place every material and mesh in-scene

2. **Enable Shader Code Libraries**
   - Project Settings → Rendering
   - Run: `RunUAT.bat BuildShaderArchive`

3. **Preload Assets**
   - Avoid runtime loading
   - Use `StreamableManager` for soft references

4. **Use Fade or Loading Screen**
   - Prevents user interaction until everything’s ready

---

## 🎓 Lesson = Level Design

Each `.umap` is its own lesson.

- Contains its own `AAssemblyActor`, parts, and `ALessonManager`
- `UGameInstance` manages flow between lessons
- Each level uses a `DA_Lesson_XX` to define:
  - Objectives
  - Step sequence
  - Instruction content

---

## 🧱 Class Overview

```plaintext
UGameInstance
 └─ ActiveLessonData : UDataAsset
 └─ LoadNextLesson()

ALessonManager
 └─ Steps : TArray<ULessonStep>
     ├─ UAssembleStep
     ├─ UFocusStep
     └─ UInteractionStep

AAssemblyActor
 └─ APartActor[]
     └─ UAssemblyComponent
         ├─ SnapPoints : USnapPointComponent[]
         └─ Behaviors : UComponentBehavior[]
```

---

## 📦 Naming Conventions

| Asset Type         | Prefix | Example             |
|--------------------|--------|---------------------|
| Level              | `L_`   | `L_Lesson_01_Intro` |
| Lesson Data        | `DA_`  | `DA_Lesson_01`      |
| Part Actor         | `BP_`  | `BP_Commutator`     |
| Snap Component     | `SPC_` | `SPC_BrushMount`    |
| Behavior Component | `CB_`  | `CB_SpinMotor`      |

---

## ✅ Future Roadmap

- Blueprint lesson step system
- Behavior template expansion (fields, forces, visuals)
- Save/load student progress
- Performance profiling + hardware scaling

---

## 📌 TL;DR

A modular, VR-based instructional simulation system built in Unreal Engine using:

- Physics-based part assembly
- Modular behaviors
- Blueprint-driven lessons
- Performance-optimized runtime
- Per-level instructional structure

Ready to scale into schools, labs, or interactive demos.
