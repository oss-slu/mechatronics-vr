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
