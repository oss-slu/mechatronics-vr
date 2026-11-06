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
