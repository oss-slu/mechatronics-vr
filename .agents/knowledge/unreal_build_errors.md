# Unreal Engine Build Errors and Solutions

## Old Compiled Code Causes Access Violation on Non-Existent Actor Classes

**Symptom:** `EXCEPTION_ACCESS_VIOLATION` when spawning an actor class during gameplay (e.g., `ASnapGuideArrowActor`), even though the class doesn't exist in source.

**Root Cause:** The `Binaries/` and `Intermediate/` folders contain old compiled code that still references deleted or renamed actor classes. The editor loads stale binaries instead of recompiling from current source.

**Fix:**
1. Close the Unreal Editor completely
2. Delete `Binaries/` and `Intermediate/` folders
3. Delete `.sln` file
4. Right-click `.uproject` → **Generate Visual Studio project files**
5. Open the newly generated `.sln` and **rebuild** the project
6. Reopen the editor

**Prevention:** After deleting code or actor classes, always clean build files before reopening the editor.

**Example:** The arrow visualization system was refactored to use `DrawDebugDirectionalArrow` instead of spawning `ASnapGuideArrowActor`. Old binaries tried to spawn the deleted class, causing a crash. Cleaning binaries and rebuilding fixed it.

## Editor Can Be Running Older C++ Than the Repo

Check `Binaries/Win64/UnrealEditor-MechatronicsVR.dll`'s timestamp against `git log` for the source you rely on. In Nov 2026 the DLL was from Oct 10, so `UGrabComponent::bIsGrabbable` (added later) didn't exist in the running editor (missing from Python `dir()` and the Details panel). If a property or fix "doesn't exist", rebuild with the editor closed before debugging further.

## Live Coding From Outside the Editor (Function-Body Changes Only)

For .cpp body changes (no UPROPERTY/UFUNCTION/class-layout changes) you don't need to close the editor: run `unreal.SystemLibrary.execute_console_command(None, "LiveCoding.Compile")` through MCP `execute_script`, then look for "Live coding succeeded" in the output log (pattern `Live coding`; there are no `LogLiveCoding` lines in the plugin's log). The patch only lives in the running editor: do a full `Build.bat MechatronicsVREditor Win64 Development -Project=... -WaitMutex` the next time the editor is closed. With the editor open, `Build.bat` refuses to build.

## Editor Crash Editing a Level Instance: "Shader compilation failures are Fatal" (Nov 2026)

**Symptom:** The editor crashes every time the Electromagnetic Rotation Level Instance is edited in Demo_LVL. `Saved/Logs/MechatronicsVR.log` shows `LogRHI: Error: Failed to create graphics pipeline` (Vertex/Pixel `<unknown>`, RTV format 0x18 = R10G10B10A2) then `PipelineStateCache.cpp Line 584 ... Shader compilation failures are Fatal`, on a Background Worker thread. Breadcrumb: `EditorVisualizeLevelInstance > PostProcessing > Scene`. It reproduced 4 times in a row, so it is deterministic.

**Cause:** Not project code or the level's contents. The GPU driver (RTX 3060 Laptop, NVIDIA 617.14, D3D12) fails to build the PSO for the editor's Level Instance highlight pass, and UE treats that as fatal. An Intel UHD iGPU is also present (Optimus laptop).

**Fix:** not verified yet. Try in order: clear NVIDIA's shader cache (`%LOCALAPPDATA%\NVIDIA\DXCache`) and the project's DerivedDataCache; roll back or update the NVIDIA driver (clean install); launch the editor with `-dx11`. Update this entry with whichever one works.
