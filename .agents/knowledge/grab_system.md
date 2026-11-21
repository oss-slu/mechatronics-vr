# Grab System

## Rejecting a Grab via TryRelease Re-enables Physics and Drops the Part

`APartActor::OnPartGrabbed` used to reject grabs of already-snapped parts by calling `GrabComponent->TryRelease()`. By then `TryGrab` had already disabled physics and attached the part to the hand, and `TryRelease` detaches it and calls `SetSimulatePhysics(true)` (`bSimulateOnDrop` defaults to true). Grabbing an assembled part therefore pulled it off its snap and let it fall.

**Fix:** reject the grab up front in `UGrabComponent::TryGrab`, before touching physics, attachment, haptics, or events. Never use `TryRelease` to "undo" a grab.

## Use UGrabComponent::bIsGrabbable to Block Grabbing Snapped Parts

`APartActor::TrySnapToPreview` sets `GrabComponent->bIsGrabbable = false` on a successful snap (both the base and part-to-part paths), and `TryGrab` returns false early when it is false.

Do not use `SetActive(false)` / `IsActive()` for this: `UActorComponent::bAutoActivate` defaults to false, so `IsActive()` is false on a fresh `UGrabComponent` and gating on it blocks every grab. Do not gate on `bIsSnapped` either; it is only set on the base-snap path, not part-to-part.

## Snapping Used to Copy the Target's Scale onto the Part

`APartActor::CalculateSnapTransform` returned `SourceRelToActor.Inverse() * TargetSnapWorld`, a full-transform multiply, so the part's resulting scale became the target snap point's world scale (e.g. the scaled battery). Parts resized when snapped, and the ghost preview showed the wrong size.

**Fix (Nov 2026):** solve rotation and location only and keep `GetActorScale3D()`:
```cpp
const FQuat NewRot = TargetSnapWorld.GetRotation() * SourceRelToActor.GetRotation().Inverse();
const FVector NewLoc = TargetSnapWorld.GetLocation() - NewRot.RotateVector(ActorScale * SourceRelToActor.GetLocation());
return FTransform(NewRot, NewLoc, ActorScale);
```
`SourceRelToActor`'s location is in unscaled actor space, which is why `ActorScale` is applied before rotating. `Mesh` is the root, so the preview mesh can use the same transform.

## Detachable Parts: bCanDetachWhenSnapped + AAssemblyActor::DetachPart (Nov 2026)

Snapped parts used to be permanently locked (`bIsGrabbable = false`), and nothing ever reset `bIsAssembled`/`bIsSnapped`. Now `APartActor::bCanDetachWhenSnapped` (default **false**) is the opt-in:
- `TrySnapToPreview` sets `GrabComponent->bIsGrabbable = bCanDetachWhenSnapped` (both snap paths), so default parts stay locked exactly as before.
- With it on, the next `TryGrab` succeeds and `OnPartGrabbed` calls `AssemblyActor->DetachPart(this)` and clears `bIsSnapped`. `TryGrab` has already turned physics off and attached to the hand, so nothing to undo (still never use `TryRelease` to undo a grab).
- `DetachPart` works from `USnapPointComponent::ConnectedSnapPoint` / `bIsAssembled`, NOT `Connections` (wiped every tick) and not `DisconnectParts` (can't handle base connections where `PartA` is null).
- The VR pawn calls `TryGrab` only on `IA_Grab_*` **Started** and `TryRelease` on **Completed**, and the snap happens in `OnPartReleased`, so a placed part can only be re-grabbed by a fresh trigger press.
- `UGrabRayCaster` (`SuckObjectToSource` and the Check Reach hit test) now skips parts where `IsPlacedInAssembly()` is true; otherwise a distance-grab impulse would pull a detachable part out of the assembly without `DetachPart`.

## FindAssemblySnapTarget Returns the Nearest Free Target, Not the First

It used to return the first free compatible snap point in list order, so with both battery terminals free a contact could only ever preview/snap to the positive one (the negative terminal was unreachable until the positive was taken). It now returns the free compatible target nearest to the part's own matching snap point, in both the `PartAssembledOnto` and base searches. `IsOutrankedForPreviewTarget` still resolves two parts wanting the same target (held part wins).

## Snapped Parts Are Invisible to the Pawn's Hand Trace Unless Pinned to PhysicsBody (Nov 2026)

`VRPawn::GetGrabComponentNearMotionController` finds what the hand can grab with a **Sphere Trace For Objects** for `ObjectTypeQuery4` (**PhysicsBody**) only. A body reports PhysicsBody only while it simulates physics; with physics off its Blueprint-set object type (WorldDynamic) applies. `TrySnapToPreview` turns physics off, so a snapped part returned **0 hits** and could not be grabbed in VR even though `bIsGrabbable` was true. Calling `GrabComponent->TryGrab` directly in a test passes anyway, so it hid the bug.

**Fix:** when `bCanDetachWhenSnapped`, `TrySnapToPreview` calls `Mesh->SetCollisionObjectType(ECC_PhysicsBody)` after disabling physics (QueryOnly collision is still found by object traces). Verify any "can't grab X" report by running `SystemLibrary.sphere_trace_multi_for_objects(world, start, end, radius, [ObjectTypeQuery.OBJECT_TYPE_QUERY4], False, [], DrawDebugTrace.NONE)` at the part (it returns an Array of hits, not a tuple) and checking the part shows up, not just by calling `try_grab`.

Applied with Live Coding (body-only change); do a full `Build.bat` the next time the editor is closed.

## A Per-Instance Setting Must Differ From the Class Default to Be Saved in the Map (Nov 2026)

Setting `bCanDetachWhenSnapped = true` on the `BP_ContactPart`/`BP_ContactPart2` **class defaults** (via Python on the CDO) looked fine in the editor but fresh sessions read the placed contacts as `False` (`inst=False`, `cdo=True`), so snapped contacts stayed locked and the grab never worked in VR. Unreal saves a level instance's property only when it differs from the class default, and the stale in-memory copies hid this. **Fix:** keep the Blueprint default `False` and set an explicit `True` on each placed contact in the map (the name then appears in the `.umap`: `grep -a bCanDetachWhenSnapped`). Apply it via the load-the-real-map recipe in `lesson_level_setup.md`, then restart the editor (or reload Demo_LVL) so the Demo_LVL level-instance copy refreshes.
