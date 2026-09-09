# Snap Guide and 3D Indicator Systems

## 3D World-Space Indicators for VR Assembly Guidance

In VR environments, 2D screen-space projections (`ProjectWorldLocationToScreen`) do not render well in stereo head-mounted displays and can cause visual confusion or depth distortion.

**Design Pattern:**
- Use a dedicated 3D actor (`ASnapGuideArrowActor`) positioned directly in world space.
- Align the indicator with the target `USnapPointComponent`'s outward normal (`GetForwardVector()`) offset by a configurable `HoverDistance` (e.g. 15 cm) so it hovers in front of the target socket without occluding hands or parts.
- Rotate the actor inward (`(-GetForwardVector()).Rotation()`) to point directly into the socket.
- Animate along the approach normal with a gentle sine-wave bobbing effect in `Tick`.
- Lifecycle is managed cleanly in `UAssembleStep`: spawned/shown on `OnStarted()`, hidden on step completion (`CheckCompletion()`), step stop (`OnStopped()`), or reset (`OnReset()`).

## Hierarchical Settings for Visual Guides
To support different player preferences and lesson difficulty modes, indicators should be toggleable at three levels:
1. **Cross-level / Persistent User Settings**: `UMechatronicsGameInstance::bEnableGuideArrows` and `SetGuideArrowsEnabled(bool)`.
2. **Session / Lesson Level**: `ULessonManagerComponent::bEnableGuideArrows` and `SetGuideArrowsEnabled(bool)`.
3. **Step Level Override**: `UAssembleStep::bShowGuideArrow` and `SetGuideArrowEnabled(bool)`.
The step checks `ShouldShowGuideArrow()`, resolving step override, game instance settings, and lesson manager settings before showing the indicator.
