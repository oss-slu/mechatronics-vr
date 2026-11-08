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

## AssembleStep Targets Classes Only (Named-Instance Targeting Was Reverted)

`TargetPartClasses` is satisfied by ANY one instance per class, so with two `BP_ContactPart` wires a step targeting that class finishes after the first one is placed. A `TargetPartNames` (match by object name) option was added in commit `0ec20f8` and then reverted on request (Nov 2026): `AssembleStep`, `LessonManagerComponent`, `LessonDataAsset` and `SnapValidatorComponent` were restored from `0ec20f8~1`. `DA_ElectromagneticRotationLesson` still has saved `TargetPartNames` data, which the engine ignores now that the property is gone.

If per-instance targeting is needed again:
- Match object names (`AActor::GetFName()`), not Outliner labels, which are editor-only. Rename an actor object from Python with `actor.modify(); actor.rename("Contact1")` (the wires are already named `Contact1`/`Contact2`).
- A data asset can't hard-reference level actors, and soft refs resolve to the editor world in PIE (they need `FSoftObjectPath::FixupForPIE`).
- Check placement with snap state (`bIsAssembled`), not `Connections` (see the connection-wiping bug above).

## A Level Without an AssemblyActor Crashes on Snap

`APartActor::TrySnapToPreview` dereferences `AssemblyActor` without a null check, and nothing spawns one (parts and the LessonManager only search the level). The Electromagnetic Rotation map lost its `AssemblyActor_0` in uncommitted edits (Nov 2026); one was re-placed at the battery (-160, 310, 0). If snapping crashes, check the level has an `AAssemblyActor` matching the parts' and LessonManager's `AssemblyActorClass`.

## Electromagnetic Rotation Is a Level Instance Inside Demo_LVL

`ElectroMagneticRotationLesson.umap` is placed in `/Game/Demo_LVL` as a Level Instance (at -30, 900, 40), next to a `Classroom` instance. Consequences:
- **Only Demo_LVL's GameMode runs.** Demo_LVL overrides to `GM_TestLesson` -> `DA_DemoLessonFull` (LessonManager `AssemblyActorClass = BP_DCAssembly`). The instance map's own GameMode override and `DA_ElectromagneticRotationLesson` are not used when playing Demo_LVL.
- **Finding actors from Python:** `EditorActorSubsystem.get_all_level_actors()` doesn't return actors inside level instances. Use `unreal.GameplayStatics.get_all_actors_of_class(editor_world, cls)`, which walks every loaded level. Edit/save those actors via the instance's map (or Level Instance edit mode).
- Parts find their AssemblyActor with `IsA(AssemblyActorClass)` over the whole world, so a `BP_DCAssembly` (a subclass) elsewhere in Demo_LVL could be picked by parts whose class is plain `AssemblyActor`.

## Two Wires With the Class-Only AssembleStep: Use Sibling Blueprints

To make a class-based step need both wires, each wire has its own Blueprint: `BP_ContactPart` (Contact1) and `BP_ContactPart2` (Contact2, made with `EditorAssetLibrary.duplicate_asset`, so its parent is `PartActor` too). They must be **siblings**: the step's check uses `IsA`, so if `BP_ContactPart2` were a child of `BP_ContactPart`, Contact2 alone would fill both class buckets and finish the step. The step uses `TargetPartClasses = [BP_ContactPart, BP_ContactPart2]`, `bRequireAllTargetsPresent = true`, and **`bRequireTargetConnected = false`**, because connections are wiped one tick after creation (see above), so by the time the second wire lands the first looks unconnected. "Present" means being in `AssemblyActor->Parts`, which only happens on a snap.

- `BP_MotorAssembly.Contact2` is typed `BP_ContactPart2`, and its graph reads `ContactSnapPoint` from `BP_ContactPart2_C`. Python can't change a variable's type, so it was removed and re-added (`add_member_variable` + `get_object_reference_type`), then the two Get nodes were pasted as T3D and rewired.
- To swap a placed actor's class: rename the old object, spawn the new class with the old transform (set scale separately), copy the cable settings, call `CableComponent.set_attach_end_to(actor, 'Magnet2')` (this works from Python, unlike setting `AttachEndTo.OtherActor`), repoint references, then destroy the old actor.
- `DA_ElectromagneticRotationLesson` used to have two identical steps (`AssembleStep_0`, `ConnectContactToBattery`). The second was removed (Nov 2026) because, once step 0 needs both wires, it could never complete (its *connected* check fails after the wipe). Always dump every step and match by name before editing. Don't assume `step_definitions[0]` is the only step.
- Removing a step from Python: write `step_definitions` back without the entry, then move the orphaned instanced step out of the asset (`orphan.rename('X_Removed', unreal.find_object(None, '/Engine/Transient'))`) before saving. Otherwise it's still saved into the .uasset. `unreal.get_transient_package()` doesn't exist.
- Verified Nov 2026 with MCP `pie_control` `{"operation":"start","simulate":true,"map_path":"/Game/ElectromagneticRotationLesson/ElectroMagneticRotationLesson"}`, which runs the lesson's own GameMode: the step stayed incomplete after Contact1 and completed after Contact2.

## Reading an Older Committed Version of an Asset

`git show <commit>:<path>.uasset > Content/__Recover/<SameName>.uasset`, then `AssetRegistry.scan_paths_synchronous(['/Game/__Recover'])` and `load_asset`. The editor then keeps the file locked: it can't delete it (`delete_asset` returns False, and `rm` says "busy"). Delete `Content/__Recover/` after closing the editor, and never commit it.
