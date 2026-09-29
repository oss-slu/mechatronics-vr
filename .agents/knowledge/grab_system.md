# Grab System

## Rejecting a Grab via TryRelease Re-enables Physics and Drops the Part

`APartActor::OnPartGrabbed` used to reject grabs of already-snapped parts by calling `GrabComponent->TryRelease()`. By then `TryGrab` had already disabled physics and attached the part to the hand, and `TryRelease` detaches it and calls `SetSimulatePhysics(true)` (`bSimulateOnDrop` defaults to true). Grabbing an assembled part therefore pulled it off its snap and let it fall.

**Fix:** reject the grab up front in `UGrabComponent::TryGrab`, before touching physics, attachment, haptics, or events. Never use `TryRelease` to "undo" a grab.

## Use UGrabComponent::bIsGrabbable to Block Grabbing Snapped Parts

`APartActor::TrySnapToPreview` sets `GrabComponent->bIsGrabbable = false` on a successful snap (both the base and part-to-part paths), and `TryGrab` returns false early when it is false.

Do not use `SetActive(false)` / `IsActive()` for this: `UActorComponent::bAutoActivate` defaults to false, so `IsActive()` is false on a fresh `UGrabComponent` and gating on it blocks every grab. Do not gate on `bIsSnapped` either; it is only set on the base-snap path, not part-to-part.
