# Lesson Level Setup

## Which Lesson Runs Is Decided by the Level's GameMode Override, Not the Map Name

`AMechatronicsGameMode::BeginPlay` auto-starts whatever `ULessonDataAsset` is in the GameMode's `AutoStartLessonData` (after a 0.5s timer). `ULessonManagerComponent` and `UMechatronicsGameInstance` do not pick a lesson on their own. So the lesson that runs is: level World Settings -> GameMode Override -> that GameMode's `AutoStartLessonData`.

**What went wrong (QuizExample, Oct 2026):** the Quiz Example played the DC motor demo lesson because:
- `GM_QuizExample` was duplicated from `GM_TestLesson` and still had `AutoStartLessonData = DA_DemoLessonFull`. `DA_QuizExample` was never assigned.
- `Content/LessonExamples/QuizExample.umap` and `Content/LessonExamples/QuizExample/QuizExample.umap` are both ~1.3 KB **ObjectRedirectors** (left from renames) that point to `QuizRoom.umap`. `QuizRoom` overrides the GameMode to `GM_TestLesson`, which also auto-starts `DA_DemoLessonFull`.
- Only `QuizWorld.umap` uses `GM_QuizExample`.

**Fix / checklist when creating a lesson level from a template:**
1. In the new GameMode BP's Class Defaults, set `AutoStartLessonData` (and `LevelLessonData`) to the new lesson's data asset.
2. In the level's World Settings, set GameMode Override to that GameMode.
3. After renaming/moving maps, right-click the folder -> **Fix Up Redirectors** so you don't open a stale redirector that leads to a different map.
4. Quick check without the editor: `strings -n 6 GM_X.uasset | grep DA_` shows which data asset a GameMode references; a .umap of only ~1 KB is a redirector.
