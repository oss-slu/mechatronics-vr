# MechatronicsVR Architecture

## Overview

MechatronicsVR is a Windows-focused virtual-reality training application built with Unreal Engine 5.7, C++, and Blueprint-authored content. Students manipulate physics-enabled mechatronics parts, connect compatible snap points, and follow step-by-step lessons in Unreal levels.

The [architecture diagrams](.github/modernize/assessment/engines/facts/architecture-diagram.md) show the application-level flow and the principal component relationships. This document describes the current implementation; planned or incomplete capabilities are identified separately.

## Runtime Architecture

### Level and lesson startup

`AMechatronicsGameMode` creates a `ULessonManagerComponent` and a `ULessonUIManagerComponent`. Its default pawn class is configured to the VR Template pawn Blueprint. A level can configure an `AutoStartLessonData` asset; when auto-start is enabled, the GameMode starts that lesson after a short delay.

`ULessonManagerComponent` initializes from the Data Asset's `StepDefinitions`. Each entry supplies a configured, instanced `ULessonStep`; the manager binds to step-completion events, links the steps in order, and activates them. Steps can complete in response to events or manager polling, depending on their configuration. When configured to auto-advance, the manager waits for the transition delay before starting the next step.

### Lesson steps and presentation

The step base class provides lifecycle, reset, completion, and update events. C++ step implementations in the project include:

- `UAssembleStep` — checks target-part presence and connections, or full assembly when configured.
- `UFocusStep` — checks the player's focus against a target.
- `UInteractionStep` — checks a configured interaction with a target actor.
- `UQuizLessonStep` and `UMetahumanLessonStep` — additional step implementations present in source.

`ULessonUIManagerComponent` manages Unreal widget components and lesson UI presentation, including instructions, progress, and part highlighting. The level GameMode supplies the UI manager to the lesson manager.

### VR interaction and assembly

The VR pawn and motion-controller components provide tracked input. `UGrabComponent` handles grabbing and releasing parts; `UGrabRayCaster` supports controller-directed interaction. `APartActor` contains a physics-enabled mesh, grab component, assembly component, snap validator, and snap-preview behavior.

`USnapPointComponent` defines snap IDs and compatible parts or snap IDs. `USnapValidatorComponent` checks compatibility and, during an assembly step, can restrict valid connections to the step's target part classes. `AAssemblyActor` records parts and logical connections, updates its assembly state, and broadcasts connection and state-change events used by lesson steps.

### Part behavior and visual feedback

Part actors support configurable motor parameters and electrical-flow visualization. Electrical flow is represented by animated arrows rendered on the part mesh; direction can be linear or circular and can be reversed. This is a visual teaching aid, not an electrical-circuit simulation.

Snap previews provide ghost outlines and placement arrows. Assembly steps can enable previews for their target parts and clear them when the step stops.

## Content and Technology

| Area | Current implementation |
|---|---|
| Engine | Unreal Engine 5.7 (`MechatronicsVR.uproject`) |
| Application code | C++ Unreal module with Blueprint integration |
| Input and VR | Enhanced Input and HeadMountedDisplay modules; OpenXR and OculusXR plugins enabled |
| User interface | Unreal Motion Graphics (UMG), Widget Components, and Blueprint-configured widgets |
| Lesson content | Unreal maps and Data Assets with instanced lesson steps |
| Interactive content | Blueprint actor assets, meshes, materials, audio, and other Unreal assets |
| External services | No required application database, cache, message broker, or web API |

The project includes a DC motor asset collection under `Content/DCMotorAssets`, including battery, horseshoe magnet, stator, and test-map content. Runtime lesson and assembly state is held in memory; authored maps, Data Assets, and other content are stored as Unreal project assets.

## Runtime Flow

1. Unreal loads a level and selects its configured GameMode.
2. The GameMode initializes the lesson and UI manager components.
3. If configured, the GameMode starts its lesson Data Asset.
4. The lesson manager activates the first configured step and updates lesson UI.
5. The student uses VR controls to manipulate parts and interact with the assembly.
6. Snap validation checks compatible points and current assembly-step targets.
7. The assembly actor records valid logical connections and broadcasts state events.
8. The active step checks its completion condition; the lesson manager updates progress and advances or completes the lesson.

## Current Limitations and Design Considerations

- **Logical connections are not guaranteed physical joints.** Physics-constraint creation methods exist in `AAssemblyActor`, but `ConnectParts` currently has the constraint-creation call commented out. It records the connection and updates assembly state without creating that constraint.
- **Lesson progress is not persisted to disk.** `UMechatronicsGameInstance` has save/load entry points, but `SaveLessonProgress` and `LoadLessonProgress` are placeholders.
- **Cross-level lesson navigation needs validation.** Lesson-library and level-loading APIs exist on the GameInstance, but the current `LoadNextLesson` implementation uses the previous-lesson guard and previous index. Do not rely on it for next-lesson navigation until corrected.
- **Asset configuration is part of the runtime contract.** Each lesson level must use a GameMode whose lesson Data Asset and assembly references match that level's content.
- **Electrical-flow arrows are illustrative.** They do not calculate current, voltage, or circuit connectivity.

## Key Source Locations

- GameMode and level lesson startup: [MechatronicsGameMode](Source/MechatronicsVR/Private/MechatronicsGameMode.cpp)
- Lesson sequencing and step activation: [LessonManagerComponent](Source/MechatronicsVR/Lesson/LessonManagerComponent.cpp)
- Lesson step data: [LessonTypes](Source/MechatronicsVR/Lesson/LessonTypes.h) and [LessonDataAsset](Source/MechatronicsVR/Lesson/LessonDataAsset.h)
- Parts and visual behavior: [PartActor](Source/MechatronicsVR/Public/PartActor.h) and [PartActor implementation](Source/MechatronicsVR/Private/PartActor.cpp)
- Snap compatibility: [SnapPointComponent](Source/MechatronicsVR/Public/SnapPointComponent.h) and [SnapValidatorComponent](Source/MechatronicsVR/Private/SnapValidatorComponent.cpp)
- Assembly state and connections: [AssemblyActor](Source/MechatronicsVR/Private/AssemblyActor.cpp)
- Cross-level lesson state: [MechatronicsGameInstance](Source/MechatronicsVR/Private/MechatronicsGameInstance.cpp)
