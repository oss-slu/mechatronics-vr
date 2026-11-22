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

## Which Terminal a Contact Is On: ConnectedSnapPoint (Nov 2026)

`ConnectParts` now sets `SnapPointA->ConnectedSnapPoint = SnapPointB` and vice versa (cleared by `DetachPart`). Because `Connections` is wiped every tick, this is how Blueprints learn what a snap point is plugged into: `Contact.ContactSnapPoint.ConnectedSnapPoint.SnapID` is `BatteryPositiveTerminal` or `BatteryNegativeTerminal`. `BP_MotorAssembly` uses it to pick the spin direction (Contact1 on `+` = original spin, on `-` = reversed). The contacts (`BP_ContactPart`, `BP_ContactPart2`) are the grabbable parts at the cable ends and have `bCanDetachWhenSnapped = true`; cables themselves aren't grabbable.

## Editing Actors of a Level Instance From Python: Load the Real Map, Not the Editor-World Copy (Nov 2026)

`get_all_actors_of_class(editor_world, ...)` returns the level instance's actors from a **temporary copy** (package `/Temp/Game/<Map>_LevelInstance_<id>_0`). `set_editor_property` + `save_asset`/`save_packages` on those never reach the real `.umap` (it stays unmodified, and `save_asset` still returns True). Instead load the real map as an asset and edit its actors by path, then save the loaded world:

```python
P='/Game/ElectromagneticRotationLesson/ElectroMagneticRotationLesson'
w=unreal.EditorAssetLibrary.load_asset(P)
a=unreal.find_object(None, P+'.ElectroMagneticRotationLesson:PersistentLevel.Contact1')
a.modify(); a.set_editor_property('can_detach_when_snapped', True)
unreal.EditorAssetLibrary.save_loaded_asset(w, False)
```
This also fixed instances that read the old default after a Blueprint class-default change (the in-memory map was stale: `inst=False` while `cdo=True`). Resaving rewrote the map from 36 KB to 120 KB because it was stale against the current Blueprints (new BP_Stator/HorseshoeMagnet/magnetic-field instance data); that is expected.

## Don't Call set_editor_property on Placed Blueprint Actors in PIE

It re-runs the actor's construction script, replacing components (old ones appear as `TRASH_SnapPointComponent_0` at world (0,0,0)) while `UAssemblyComponent` keeps its cached snap-point list, so snapping then targets stale snap points and gives wrong results. In PIE tests use function calls only (`set_actor_location`, `try_snap_to_preview`, `detach_part`, `grab_component.try_grab(mc)` with `unreal.new_object(unreal.MotionControllerComponent, outer=actor)` as a stand-in hand) and fix property values before starting PIE.

## Measuring Motor Spin From Python

`time.sleep` in editor Python blocks the game thread, so sample the stator in two separate `execute_script` calls (store the first quaternion in a file). Euler pitch wraps at these angles, so compare quaternions: signed rotation about the stator's right vector. Unreal's `+Pitch` shows as negative about +Y; compare arrangements against each other rather than trusting the absolute sign.

## Chaining Lessons: ULessonDataAsset::NextLesson (Nov 2026)

To run lesson B after lesson A in the same level, set `A.NextLesson = B` (plus optional `NextLessonDelaySeconds`) and point the level's GameMode `AutoStartLessonData` at A. `AMechatronicsGameMode::HandleLessonCompleted` (bound to `LessonManager->OnLessonCompleted`) starts B on a timer. `UMechatronicsGameInstance::LoadNextLesson` is NOT used: it is buggy (checks `HasPreviousLesson`, loads index-1) and never starts the lesson in the GameMode.

- **Per-lesson assembly:** the manager's `AssemblyActorClass` is one value per GameMode, so lessons that use different assemblies set `ULessonDataAsset::AssemblyActorClassOverride` (Electromagnetic = plain `AssemblyActor`, DC motor = `BP_DCAssembly`). `FindAssemblyOfClass` prefers an exact class match before `IsA`, because `BP_DCAssembly` is also an `AssemblyActor`.
- **Bug fixed:** `CompleteCurrentStep` used to set `bIsLessonCompleted = true` before `EndLesson()`, and `EndLesson()` only broadcasts `OnLessonCompleted` when that flag is false, so the event never fired on a natural finish. Anything bound to it in Blueprints will now start firing.
- `StopLesson()`/`EndPlay` set `bSuppressLessonChaining` so ending a lesson manually never starts the next one.
- New UPROPERTYs on a data asset need an editor-closed build (Live Coding can't add properties).

## Check Demo_LVL's GameMode Override First When No Lesson/Payton Runs (Nov 2026)

Symptom: Payton never explains anything, because no lesson starts. Demo_LVL.umap was re-saved by the editor with its World Settings GameMode override **empty**, so the project default (plain `MechatronicsGameMode`, no `AutoStartLessonData`) ran. Check with Python: `get_editor_subsystem(UnrealEditorSubsystem).get_editor_world().get_world_settings().get_editor_property('default_game_mode')` should be `GM_TestLesson_C`; or `grep -a -c GM_TestLesson Content/Demo_LVL.umap` (0 = missing). Fix: `ws.modify(); ws.set_editor_property('default_game_mode', gm_bp.generated_class())` then `EditorLoadingAndSavingUtils.save_current_level()`.

## Adding a Payton Step to an Existing Lesson From Python (Nov 2026)

Worked for the "try reversing the motor connections" step (`EM_Reverse`, appended after `EM_Spinning`):
- `step_definitions` entries are `LessonStepData` structs (`step_class`, `step_instance`), not step objects. `MetahumanLessonStep`/`LessonStep` entries keep `step_class = None`; build one with `entry = unreal.LessonStepData(); entry.set_editor_property('step_instance', new)`, create the step with `unreal.new_object(unreal.MetahumanLessonStep, outer=da, name=...)`, set `previous_step`/`next_step` on both neighbours, then write the whole list back and `save_asset`.
- The duplicated `SoundWave` (`duplicate_asset`) is **only in memory** until you call `EditorAssetLibrary.save_asset` on it; `git status` won't show it otherwise and the step's `voice_line` would dangle after a restart.
- Read `DT_Subtitles` safely with `get_data_table_column_names(dt)[0]` + `get_data_table_column_as_string(dt, col)` (one ImportText string per row, `((Caption=..,SecondsUntilNext=..),(...))`); split on `"),(Caption="` to get per-cue strings for the replace-everything `fill_data_table_from_json_string`. Verify afterwards by counting `Caption=` per row.
- VRPawn map: `cdo.get_editor_property('SubtitleTrackBySound')` returns a dict-like; copy it to a Python dict, add `{sound: 'RowName'}`, `set_editor_property('SubtitleTrackBySound', d)`, `save_asset('/Game/VRTemplate/Blueprints/VRPawn')`.
- That extra step (`EM_Reverse`) was later **removed on request**: the "try swapping the wires" instruction is now just a cue at the end of the `EM_Intro` row, so it plays up front and never depends on a step completing. Prefer adding narration to an existing row over a new step when the line shouldn't wait for anything. Removing a step is covered in the "Two Wires" section (move the orphan to the transient package; also clear its `voice_line` and the VRPawn map entry before deleting the sound).
- **Two bugs kept Payton's `EM_Spinning` (and its "swap the wires" line) from ever playing (Nov 2026):**
  1. A plain `LessonStep` (`LessonStep_0`, "Now that the circut is complete the motor spins") has `bWantsTickWhileActive = false` and nothing calls `CompleteStep()` on it, so it never advances. It used to be the last step, so nobody noticed. It was removed from `DA_ElectromagneticRotationLesson`. Never put a plain `LessonStep` mid-lesson; use a `MetahumanLessonStep` (completes when Payton finishes) or an `AssembleStep`.
  2. In Demo_LVL, `APartActor` picked its assembly with `IsA(AssemblyActorClass)` over the whole world, and `BP_DCAssembly` is a subclass of `AssemblyActor`. Depending on actor order, the contacts joined the DC assembly while the step watched `AssemblyActor_0`, so both wires connected but `AssembleStep_0` never completed (log: `Parts Contact1/Contact2 ... connected` with no `Completing step`). Fixed in `PartActor.cpp` by preferring `GetClass() == AssemblyActorClass`, then falling back to `IsA` (same rule as the LessonManager). Applied with Live Coding: do a full `Build.bat` when the editor is closed.
  - Repro/verify: PIE `{"operation":"start","simulate":true,"map_path":"/Game/Demo_LVL"}`, `lm.go_to_step(1)`, then for each contact `c.find_best_preview_target()` + `c.set_actor_location(target.get_world_location() + Vector(0,0,5))` in one script call and `c.try_snap_to_preview()` in a **later** call (Tick must set `CurrentTargetSnapPoint` in between). Check `AssemblyActor.Parts` of both assemblies. The result varies run to run, so run it several times.

## Holding Payton Until the Wires Are Swapped: UTerminalSwapStep (Nov 2026)

`UTerminalSwapStep` (`Source/MechatronicsVR/Lesson/TerminalSwapStep.*`) watches one part class (`WatchedPartClass`, e.g. `BP_ContactPart`) and completes when its terminal (`SnapPoint->ConnectedSnapPoint->SnapID`) differs from the first terminal it was seen on during the step; a detached wire just waits. Intended chain in `DA_ElectromagneticRotationLesson`: `MetahumanLessonStep_1` (EM_Spinning, ends on "try swapping the wires") -> `TerminalSwapStep` -> `MetahumanLessonStep` for `EM_Outro` ("Next, we will build a full DC motor"), which is the last step so the chained DC lesson only starts after the swap. The step overrides `TickStep` because the base `ULessonStep::TickStep` calls `StopTicking()` before checking, i.e. its timer only fires once. It's a new UCLASS, so it needs an editor-closed build (Live Coding can't add classes). `EM_Outro` sound key, VRPawn map entry and `DT_Subtitles` row were added ahead of the step.

## Giving Payton Lines in a Lesson (Electromagnetic Rotation, Nov 2026)

Payton only speaks for `MetahumanLessonStep`s, so a lesson with none is silent. Each line needs: (1) a `SoundWave` as the lookup key (duplicate any existing one, it is never played; `/Game/Audio/ElectromagneticRotation/EM_Intro`, `EM_Spinning`); (2) an entry in the **VRPawn CDO** `SubtitleTrackBySound` map (sound -> row name); (3) a `DT_Subtitles` row (`Cues` array of `Caption` + `SecondsUntilNext`). Add rows with `DataTableFunctionLibrary.fill_data_table_from_json_string(dt, json)` which **replaces the whole table**: pass every row as `{"Name":..., "Cues":[ "<cue text>", ...]}` where `Cues` is a real JSON array (a single string gives "Expected Array, got String" and wipes the cues) and each element is a cue in ImportText form (`(Caption=INVTEXT("..."),SecondsUntilNext=5.0)`); reuse existing rows' exported text split at top-level commas to keep their localization keys. It returns a bool, not a problems list. Undo an unsaved bad import with `EditorLoadingAndSavingUtils.reload_packages([pkg], unreal.ReloadPackagesInteractionMode.ASSUME_POSITIVE)` (the default interactive mode pops a modal "Message" dialog that blocks the editor and every REST call).
