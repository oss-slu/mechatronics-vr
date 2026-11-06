# Lesson Level Setup

## Which Lesson Runs Is Decided by the Level's GameMode Override, Not the Map Name

`AMechatronicsGameMode::BeginPlay` auto-starts whatever `ULessonDataAsset` is in the GameMode's `AutoStartLessonData` (after a 0.5s timer). `ULessonManagerComponent` and `UMechatronicsGameInstance` do not pick a lesson on their own. So the lesson that runs is: level World Settings -> GameMode Override -> that GameMode's `AutoStartLessonData`.

**What went wrong (QuizExample, Oct 2026):** the Quiz Example played the DC motor demo lesson because:
- `GM_QuizExample` was duplicated from `GM_TestLesson` and still had `AutoStartLessonData = DA_DemoLessonFull`. `DA_QuizExample` was never assigned.
- `Content/LessonExamples/QuizExample.umap` and `Content/LessonExamples/QuizExample/QuizExample.umap` are both ~1.3 KB **ObjectRedirectors** (left from renames) that point to `QuizRoom.umap`. `QuizRoom` overrides the GameMode to `GM_TestLesson`, which also auto-starts `DA_DemoLessonFull`.
- Only `QuizWorld.umap` uses `GM_QuizExample`.

**Fix / checklist when creating a lesson level from a template:**
1. In the new GameMode BP's Class Defaults, set `AutoStartLessonData` (and `LevelLessonData`) to the new lesson's data asset.
2. In the level's World Settings, set GameMode Override to that GameMode.
3. After renaming/moving maps, right-click the folder -> **Fix Up Redirectors** so you don't open a stale redirector that leads to a different map.
4. Quick check without the editor: `strings -n 6 GM_X.uasset | grep DA_` shows which data asset a GameMode references; a .umap of only ~1 KB is a redirector.

## AssembleStep Uses the LessonManager's AssemblyActorClass, Not the Step's

`ULessonManagerComponent::UpdateStepReferences` assigns an AssembleStep's `AssemblyActor` using the **LessonManager component's** `AssemblyActorClass` (set in the GameMode BP's LessonManager component defaults). The step's own `AssemblyActorClass` field is not used for this lookup. If the manager's class doesn't match an assembly in the level, the step never completes. When copying `GM_TestLesson`, change `LessonManager -> AssemblyActorClass` (it is `BP_DCAssembly`).

## A Part Only Snaps onto Its PartAssembledOntoClass or the Assembly Base

`APartActor::FindAssemblySnapTarget` only searches (1) the snap points of the first level actor of `PartAssembledOntoClass`, then (2) the `AssemblyActor`'s base snap points. Snap points must be children of the part's `Assembly` component (`UAssemblyComponent::RegisterSnapPoints` only scans its children), and compatibility is by `SnapID` in both directions (`CompatibleSnapIDs`).

**Example (Electromagnetic Rotation lesson):** `BP_ContactPart` has `PartAssembledOntoClass = BP_Battery`, `ContactSnapPoint` (SnapID `ContactSnap`, accepts `BatteryPositiveTerminal`); the battery's `BatteryPositiveSnap` has SnapID `BatteryPositiveTerminal` and accepts `ContactSnap`. A second wire, `Contact2` (cable end -> motor `Magnet2`; `Contact1` goes to `Magnet1`), uses the battery's `BatteryNegativeSnapPoint` (SnapID `BatteryNegativeTerminal`, accepts `ContactSnap`). `ContactSnapPoint` accepts both terminal IDs, so either wire fits either terminal (reversing the leads only reverses the spin). `BP_MotorAssembly` gates its spin in Blueprint: Event Tick -> Branch(AND(`Contact.ContactSnapPoint.bIsAssembled`, `Contact2.ContactSnapPoint.bIsAssembled`)) -> Spin, so both wires must be on the battery (a closed circuit). `Contact`/`Contact2` are instance-editable variables set to the level's Contact1/Contact2. It uses `bIsAssembled` rather than `ArePartsConnected` because of the connection-wiping bug below. Lesson-specific logic like this goes in the lesson's Blueprints, not in shared C++.

## AssemblyActor Drops Every Connection One Frame After It Is Made

`AAssemblyActor::Tick` -> `CleanupInvalidConstraints()` removes any `FPartConnection` whose `Constraint` is invalid or whose `PartA`/`PartB` is null. Constraint creation in `ConnectParts` is commented out, so `Constraint` is always null: every connection (and every base connection, where `PartA` is null) is erased on the next tick. `SnapPointComponent::bIsAssembled` and `AssemblyActor->Parts` are NOT reset, so they still reflect what's snapped.

**Symptoms:** `ArePartsConnected` / `HasConnection` / `GetConnectedParts` return false/empty a frame after a snap; AssembleSteps only complete because they evaluate inside the `OnPartsConnected` broadcast. Don't poll `Connections` from gameplay code until this is fixed — use `bIsAssembled` on the snap point, or react to `OnPartsConnected`.

**Found:** Electromagnetic Rotation lesson PIE test (Nov 2026) — motor Branch on `ArePartsConnected(Contact, Battery)` was true right after `TrySnapToPreview` and false the next frame.

## AssembleStep Can Target Named Instances (TargetPartNames) or Classes

`TargetPartClasses` is satisfied by ANY one instance per class, so with two `BP_ContactPart` wires the step finished after the first. `UAssembleStep::TargetPartNames` (Nov 2026) lists specific parts by **object name** (`AActor::GetFName()`); when non-empty it overrides the classes everywhere, through `UAssembleStep::IsTargetPart` (completion, snap previews, `SnapValidatorComponent`, LessonManager instruction placement), and the step completes only when every named part is in `AssemblyActor->Parts` with an assembled snap point. Class-only steps behave as before, so old lessons need no migration.

- Use object names, not Outliner labels: labels are editor-only. Rename the actor object to match its label from Python: `actor.modify(); actor.rename("Contact1")` (pointer references to it survive).
- A data asset can't hard-reference level actors, and soft refs resolve to the editor world in PIE (needs `FSoftObjectPath::FixupForPIE`), which is why names were used.
- Electromagnetic Rotation lesson: wires are objects `Contact1`/`Contact2`.

## A Level Without an AssemblyActor Crashes on Snap

`APartActor::TrySnapToPreview` dereferences `AssemblyActor` without a null check, and nothing spawns one (parts and the LessonManager only search the level). The Electromagnetic Rotation map lost its `AssemblyActor_0` in uncommitted edits (Nov 2026); one was re-placed at the battery (-160, 310, 0). If snapping crashes, check the level has an `AAssemblyActor` matching the parts' and LessonManager's `AssemblyActorClass`.

## Electromagnetic Rotation Is a Level Instance Inside Demo_LVL

`ElectroMagneticRotationLesson.umap` is placed in `/Game/Demo_LVL` as a Level Instance (at -30, 900, 40), next to a `Classroom` instance. Consequences:
- **Only Demo_LVL's GameMode runs.** Demo_LVL overrides to `GM_TestLesson` -> `DA_DemoLessonFull` (LessonManager `AssemblyActorClass = BP_DCAssembly`). The instance map's own GameMode override and `DA_ElectromagneticRotationLesson` are not used when playing Demo_LVL.
- **Finding actors from Python:** `EditorActorSubsystem.get_all_level_actors()` doesn't return actors inside level instances. Use `unreal.GameplayStatics.get_all_actors_of_class(editor_world, cls)`, which walks every loaded level. Edit/save those actors via the instance's map (or Level Instance edit mode).
- Parts find their AssemblyActor with `IsA(AssemblyActorClass)` over the whole world, so a `BP_DCAssembly` (a subclass) elsewhere in Demo_LVL could be picked by parts whose class is plain `AssemblyActor`.
