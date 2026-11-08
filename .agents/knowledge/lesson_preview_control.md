# Lesson Manager Preview Control

## Implementation: AssembleStep Preview Management

When an AssembleStep is active, the lesson manager controls which parts show snap previews (arrows and ghost outlines).

### How It Works

**AssembleStep** (in `OnStarted()` and `OnStopped()`):
1. `UpdateTargetPartVisuals()` - On step start:
   - Disables `bAllowGhostOutline` and `bAllowSnapArrow` for ALL parts in the world
   - Enables them only for parts matching `TargetPartClasses`
   
2. `ClearTargetPartVisuals()` - On step stop:
   - Disables previews for all parts

**PartActor** (in `Tick()`):
1. Calls `UpdatePreviewState()` every frame - finds the best snap point target
2. Calls `ShowGhostOutline()` every frame:
   - If `bAllowGhostOutline` is false, hides outline and returns
   - If true and a valid target exists, shows the ghost outline
3. Calls `ShowSnapArrow()` every frame:
   - If `bAllowSnapArrow` is false, hides arrow and returns
   - If true and snap points exist, shows the arrow

### Complete Flow

1. AssembleStep starts
2. All parts have their bools disabled
3. Target parts have their bools enabled
4. PartActor Tick runs every frame:
   - Finds best snap point target
   - Shows outline only if bool is true AND target exists
   - Shows arrow only if bool is true AND snap points exist
5. AssembleStep stops
6. All parts have their bools disabled

### Result

Only parts that need to be assembled in the current step show visual previews, because only they have `bAllowGhostOutline` and `bAllowSnapArrow` enabled.

## Ghost Outline Not Displaying When Enabled

**Symptom:** Parts with `bAllowGhostOutline = true` don't show the preview mesh when grabbed.

**Root Cause:** The call to `ShowGhostOutline()` wasn't being executed. It was commented out in `OnPartGrabbed()` and wasn't being called in `Tick()`.

**Fix:** Call `UpdatePreviewState()`, `ShowGhostOutline()`, and `ShowSnapArrow()` every frame in `Tick()`:
```cpp
void APartActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Update preview state every frame
	UpdatePreviewState();
	ShowGhostOutline();
	ShowSnapArrow();
	
	// ... rest of Tick
}
```

Additionally, `ShowGhostOutline()` and `ShowSnapArrow()` now immediately hide their visuals when their respective bools are false, ensuring immediate feedback when toggling the bools.

## Several Parts Previewing the Same Snap Point (One Arrow at a Time)

**Symptom:** both wires (Contact1, Contact2) drew arrows/ghosts to the battery's positive terminal at once. Each part independently takes the *first* free compatible snap point from `FindAssemblySnapTarget()`, so identical parts all pick the same one.

**Fix (Nov 2026):** `APartActor::FindBestPreviewTarget` = `IsPreviewAllowedByLesson()` (the old lesson filter) + `FindAssemblySnapTarget()` + `IsOutrankedForPreviewTarget()`. When other unplaced parts with previews enabled resolve to the same snap point, only one previews it: the part held in a motion controller wins, otherwise the lowest object name (`FName::LexicalLess`). After it snaps, the next part's first free target moves on (Contact2 -> negative terminal). Works with or without a lesson running.

**Second bug fixed with it:** `ShowSnapArrow` and `Tick` redraw from the cached `MySnapPoint`/`CandidateSnapPoint`, so a part that lost its target kept drawing last frame's arrow. `ShowGhostOutline` now hides and clears those when `CurrentTargetSnapPoint` is null, and `ShowSnapArrow` requires a current target.

**Testing without VR:** MCP `/api/editor/pie_control` `{"operation":"start","simulate":true}` runs Simulate In Editor (BeginPlay/Tick, no player). Query `get_editor_subsystem(UnrealEditorSubsystem).get_game_world()`, call `find_best_preview_target()` / `try_snap_to_preview()` from Python, then `{"operation":"stop"}`. Status JSON has spaces (`"pie_active": true`), so strip whitespace before grepping. In the plain editor world `find_best_preview_target()` returns None because BeginPlay hasn't set `AssemblyActor`/`PartAssembledOnto`.
