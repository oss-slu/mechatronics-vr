---
trigger: always_on
---

## Agent Knowledge System

This project maintains a persistent knowledge base in `.agents/knowledge/`.  
Agents **must** read and write to this folder according to the rules below.

### On Startup — Load All Lessons

At the start of every session, read every `.md` file in `.agents/knowledge/` and load its contents into your context before taking any action. This is your accumulated project memory.

### Recording New Knowledge

When you make a mistake, discover a non-obvious fact about the codebase, or learn something new that would help future agents:

1. **Identify the topic** — pick a short, descriptive snake_case name (e.g., `blueprint_compilation`, `unreal_build_errors`).
2. **Check for an existing file** — look in `.agents/knowledge/` for a file whose topic matches.
   - If one **exists**: append your new finding to that file under a new `##` heading.
   - If none **exists**: create a new file named `<topic>.md`.
3. **Format** — every entry must include:
   - A `##` heading describing the specific lesson
   - What went wrong or what was learned
   - The correct approach or fix
   - *(Optional)* a code snippet or example

### Naming Convention

| Situation | Action |
|-----------|--------|
| New, unrelated topic | Create `.agents/knowledge/<topic>.md` |
| Related to existing topic | Append to existing file |
| Unsure? | Err on the side of appending to the closest existing topic |

### Example Entry

**File:** `.agents/knowledge/unreal_build_errors.md`

```markdown
## Missing UFUNCTION Specifier Causes Linker Error

Calling a Blueprint-exposed function without `UFUNCTION(BlueprintCallable)` compiles
in C++ but causes a linker error when packaging.

**Fix:** Always add the appropriate UFUNCTION specifier before exposing functions to Blueprints.
```
