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

## "Attempted to access index N from array Cues of length N" Fixed by Reordering Exec (Nov 2026)

**Symptom:** on the last cue of every `DT_Subtitles` row, PIE logs `Attempted to access index 3 from array Cues_... of length 3` plus `Accessed None reading structure SecondsUntilNext` on the `Set Timer by Function Name` node in BP_Payton's `SpeakNextCue`. It was always there; a longer row (EM_Reverse) just made it noticeable.

**Cause:** the pure nodes feeding the timer's `Time` (`GET Cues[SpeechCueIndex]` -> Break -> `SecondsUntilNext` -> SelectFloat) are re-evaluated when the timer executes, and `Set SpeechCueIndex` (index++) ran *before* the timer. So the timer read cue `index+1`: out of range on the last cue, and with `bUseTTS` off every caption waited for the *next* cue's `SecondsUntilNext`.

**Fix:** swap the order to `Display Subtitle -> Branch(bUseTTS) -> (Speak on Channel | else) -> Set Timer -> Set SpeechCueIndex`. Done via `/api/blueprints/modify` `disconnect_pins` (`execute`/`then` of the Set Index node) and `connect_pins` (`source_node_id`, `source_pin`, `target_node_id`, `target_pin`, `graph_name`, `blueprint_path`). General rule: a pure node reads variables at the moment of its *consumer*, not where it sits in the graph, so don't mutate a variable before an impure node that reads it through a pure chain. `/api/blueprints/modify` compiles but doesn't save: call `EditorAssetLibrary.save_asset` afterwards.

## Pasting a Function Call With a Wrong Name Silently Drops the Node

A T3D `K2Node_CallFunction` whose `MemberName` doesn't exist (I used `SelectDouble`) simply doesn't appear after Ctrl+V; no error. Look the C++ name up in the engine header (`D:/Epic Games/UE_5.7/Engine/Source/Runtime/Engine/Classes/Kismet/KismetMathLibrary.h` has `SelectFloat(double A, double B, bool bPickA)`). Python's snake_case name (`select_float`) is a hint, not proof.

## TTS Is Per-Instance: Electromagnetic Payton On, DC Motor Payton Off (Nov 2026)

`BP_Payton` class default `bUseTTS = False` (the "Remove TTS for instructor for now" commit). Demo_LVL contains two Paytons, one in each level instance (`BP_Payton_C_1` in `ElectroMagneticRotationLesson` and in `DCMotorLesson`). Requested setup: Electromagnetic instance `bUseTTS = True`, DC motor instance False. Set it on the real map (`find_object(None, '/Game/ElectromagneticRotationLesson/ElectroMagneticRotationLesson.ElectroMagneticRotationLesson:PersistentLevel.BP_Payton_C_1')`, `modify()`, `set_editor_property('bUseTTS', True)`, `save_loaded_asset(world)`), not the CDO.

**Open risk:** `UMetahumanLessonStep::PlayVoiceLine` triggers `GetAllActorsOfClass(AMetahumanBase)[0]`, i.e. whichever Payton the world lists first, not the one near the lesson. With two Paytons in Demo_LVL the lesson may talk through the other one (a TTS-off Payton shows subtitles but is silent). Subtitles come from the VRPawn, so seeing subtitles doesn't prove which Payton was triggered. Also note an interrupted script can still have run: re-read state (here it had set the CDO to True) before assuming nothing happened.

## Recorded Voice Lines Are Played From C++ When bUseTTS Is False (Nov 2026)

The TTS commit (6912c06) deleted BP_Payton's two `SpawnSoundAttached` nodes, so since then **no Payton ever played its `VoiceLine` recording**; a `bUseTTS = false` Payton (the DC motor lesson's) showed subtitles in silence. Fix, in C++ not Blueprint: `UMetahumanLessonStep::PlayVoiceLine` still calls `TriggerVoiceLine` (subtitles/lips), then reads the Blueprint bool by name (`FindFProperty<FBoolProperty>(Chosen->GetClass(), "bUseTTS")`) and, when it is false (or missing), plays the step's `VoiceLine` with `UGameplayStatics::SpawnSoundAttached` on the Payton's mesh. TTS Paytons are unaffected.
- The EM lesson's `EM_*` sounds are **duplicates of a real recording used only as map keys** (see "Giving Payton Lines"). Never set `bUseTTS = false` on the EM Payton, or those placeholder clips would play.
- Not stopped on step skip/stop: a skipped step's clip keeps playing to its end.
- Test: start PIE, then call `gm.stop_lesson()` + `gm.start_lesson(gm.AutoStartLessonData.next_lesson)`. In PIE `EditorAssetLibrary.load_asset` fails on assets that aren't already loaded ("Editor is currently in a play mode"), so get the DC asset through the loaded EM asset's `next_lesson`. Check `AudioComponent.is_playing()` ~1s after start (the intro clip is only 9.4s).

## A bUseTTS=false Payton Never Finishes a Row Whose Last Cue Is 0 (Nov 2026)

With TTS off, `SpeakNextCue` uses `cue.SecondsUntilNext` as the length of the `CheckSpeechFinished` timer. A last cue of `0` makes `Set Timer by Function Name` log `CheckSpeechFinished SetTimer passed a negative or zero time` and create **no timer**, so `FinishSpeaking` never runs and the step never completes (a lesson stuck on its welcome step, parts never get `bAllowSnapArrow`, no arrows). TTS Paytons ignore the value, so the zeros only broke the DC lesson. The DC rows (`Intro`, `ConnectPower`, `Core`, `Magnets`, `PostAssembly`, `ToQuiz`) were authored with a final `0`.

**Fix (data):** each non-TTS row's last cue = `max(1, recordingDuration - sum(previous cues)) + 0.5`, so the step completes just after the recording ends (Intro 3.9, ConnectPower 4.9, Core 15.5, Magnets 4.6, PostAssembly 7.8, ToQuiz 3.7). `EM_*` rows were left alone. **When adding or re-timing a row for a non-TTS Payton, never leave the last `SecondsUntilNext` at 0.** (A cleaner fix would clamp the timer's Time with `Max(x, 0.1)` in `SpeakNextCue`.)

**Testing pitfall that hid this:** `On Lesson Step Audio Completed` completes *whatever step is current*. If you fast-forward through the EM lesson (`go_to_step`) and then start the DC lesson, the EM Payton's still-running TTS line finishes a few seconds later and completes the DC step 0 for you, so the lesson appears to advance. To test a lesson's own completion, `gm.stop_lesson()`, wait ~50s for the other Payton's TTS to go quiet, then `start_lesson`, and poll `CurrentStepIndex` (DC step 1 should appear ~14s after start).
