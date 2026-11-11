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
