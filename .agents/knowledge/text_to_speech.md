# Text-to-Speech (Payton speaks subtitles)

## How Payton Speaks (Nov 2026)

BP_Payton no longer plays the recorded `VoiceLine`. It speaks the `DT_Subtitles` captions with the engine **TextToSpeech** plugin (enabled in `MechatronicsVR.uproject`, plugin lives in `Engine/Plugins/Experimental/TextToSpeech`). All logic is Blueprint, in BP_Payton's EventGraph:

- BeginPlay: `TextToSpeechEngineSubsystem.AddDefaultChannel/ActivateChannel(TTSChannelId)`; `TTSChannelId` defaults to `Professor`.
- Both voice paths (part enters hitbox, `Event TriggerVoiceLine`) -> `Branch(IsSpeakingOnChannel)` -> on False: `Set PendingVoiceLine` -> custom event `SpeakVoiceLine`.
- `SpeakVoiceLine`: VRPawn `SubtitleTrackBySound` Find(PendingVoiceLine) -> `SpeechTrackName`, `SpeechCueIndex=0` -> `SpeakNextCue`.
- `SpeakNextCue`: DT row -> cue[index] -> VRPawn `DisplaySubtitle` + `SpeakOnChannel` -> index++ -> looping 0.1s timer `CheckSpeechFinished`, which calls `SpeakNextCue` again once TTS is silent.
- `FinishSpeaking`: VRPawn `ClearSubtitles` -> existing `On Lesson Step Audio Completed` (completes non-Assemble steps).
- Event Tick sets `Face_AnimBP.bPlayVoiceLine = IsSpeakingOnChannel`, so lips move exactly while TTS talks (looping `CurVoiceSequence`).

VRPawn was not changed; Payton only uses its `DisplaySubtitle`/`ClearSubtitles`/`SubtitleTrackBySound`. `SecondsUntilNext` in the cues is now ignored. **`VoiceLine` sound assets must stay assigned**: they are still the key into `SubtitleTrackBySound`, just not played.

## TTS Plugin Facts

- The Blueprint API has **no "finished speaking" event** (`FOnTextToSpeechFinishSpeaking` is C++-only on `FTextToSpeechBase`, which the subsystem keeps private). Poll `IsSpeakingOnChannel`.
- On Windows the backend is **Flite** (offline, synthetic voice, non-spatial 2D audio via a submix listener).
- `SpeakOnChannel` sets the speaking flag synchronously (`bAllowPlayback = true`), so polling right after `Speak` is safe.
- It is an *engine* subsystem: channels survive between PIE sessions. `AddDefaultChannel` on an existing ID only logs a warning, so calling it every BeginPlay is fine.
- Every "can't find the track/row" path must still call `FinishSpeaking`, or the lesson step never completes.

## Building Big Blueprint Changes Through MCP (Paste, Wire, Refresh)

What worked for adding ~65 nodes:
1. Generate T3D for all nodes with unique `NodePosX/Y`; user pastes once (Ctrl+V). Pasted nodes snap to a 16-unit grid, so match them back by nearest position (greedy, closest pairs first), not exact coordinates.
2. Calls to custom events pasted in the same batch as the events have **no pins** until File -> Refresh All Nodes.
3. `K2Node_GetDataTableRow` pastes as "NONE": the API can't set object pin defaults (`set_pin_value` only writes `DefaultValue` text), so the user picks the DataTable.
4. MCP `connect_pins` uses raw `MakeLinkTo` (no schema notification), so wildcard pins (Cast `Object`, `Map_Find`, `Array_IsValidIndex`, `GET`) stay "undetermined" until a second **Refresh All Nodes**. Links survive the refresh.
5. Python can't read `Blueprint.status` (protected); read the last `Compiling Blueprint` block of the output log for `[Compiler]` messages instead.
6. A custom event's function name is its display name with spaces (`MemberName="On Lesson Step Audio Completed"`).

## Testing Without a Headset

Normal PIE (`pie_control` `{"operation":"start"}`) on `Demo_LVL` spawns VRPawn, and `DA_DemoLessonFull` auto-starts with a MetahumanLessonStep that triggers Payton's Intro line. Run a tiny sampling script via `execute_script` every ~0.25s from outside (a `sleep` inside editor Python blocks the game thread), logging `IsSpeakingOnChannel('Professor')`, `Face.get_anim_instance().bPlayVoiceLine`, `SpeechCueIndex`, and `LessonManager.get_current_step()`.

## bUseTTS Toggle (Nov 2026)

`BP_Payton.bUseTTS` (instance-editable bool, default true) turns the voice off while keeping subtitles. In `SpeakNextCue`: `Display Subtitle -> Branch(bUseTTS)`; True -> `Speak on Channel`, False -> straight to `Set SpeechCueIndex` (skips speaking). The cue timer's `Time` is `SelectFloat(A=0.1, B=cue.SecondsUntilNext, bPickA=bUseTTS)`, so with TTS off each caption stays up for its authored `SecondsUntilNext` before `CheckSpeechFinished` advances (it sees "not speaking" and moves on). Per-instance override: tick/untick `bUseTTS` on the Payton in the level.

## Pasting a Function Call With a Wrong Name Silently Drops the Node

A T3D `K2Node_CallFunction` whose `MemberName` doesn't exist (I used `SelectDouble`) simply doesn't appear after Ctrl+V; no error. Look the C++ name up in the engine header (`D:/Epic Games/UE_5.7/Engine/Source/Runtime/Engine/Classes/Kismet/KismetMathLibrary.h` has `SelectFloat(double A, double B, bool bPickA)`). Python's snake_case name (`select_float`) is a hint, not proof.

## TTS Is Per-Instance: Electromagnetic Payton On, DC Motor Payton Off (Nov 2026)

`BP_Payton` class default `bUseTTS = False` (the "Remove TTS for instructor for now" commit). Demo_LVL contains two Paytons, one in each level instance (`BP_Payton_C_1` in `ElectroMagneticRotationLesson` and in `DCMotorLesson`). Requested setup: Electromagnetic instance `bUseTTS = True`, DC motor instance False. Set it on the real map (`find_object(None, '/Game/ElectromagneticRotationLesson/ElectroMagneticRotationLesson.ElectroMagneticRotationLesson:PersistentLevel.BP_Payton_C_1')`, `modify()`, `set_editor_property('bUseTTS', True)`, `save_loaded_asset(world)`), not the CDO.

**Open risk:** `UMetahumanLessonStep::PlayVoiceLine` triggers `GetAllActorsOfClass(AMetahumanBase)[0]`, i.e. whichever Payton the world lists first, not the one near the lesson. With two Paytons in Demo_LVL the lesson may talk through the other one (a TTS-off Payton shows subtitles but is silent). Subtitles come from the VRPawn, so seeing subtitles doesn't prove which Payton was triggered. Also note an interrupted script can still have run: re-read state (here it had set the CDO to True) before assuming nothing happened.
