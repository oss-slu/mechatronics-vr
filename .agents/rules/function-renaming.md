# Function Renaming Rule

## When to Rename Functions

**If you change the functionality of an existing function, you must rename it to reflect the new behavior.**

### Rationale
- Function names are contracts with callers about what the function does
- Changing functionality without renaming causes confusion and introduces bugs
- Callers may rely on the original behavior without knowing it changed

### Examples

❌ **Bad**: Function kept its original name despite changed behavior
```cpp
// Original: UpdateGuideArrowTarget() spawned arrow actors
// Changed to: Only highlights outlines with same name
// Result: Misleading name, maintenance confusion
static void UpdateGuideArrowTarget(UAssembleStep* Step)
```

✅ **Good**: Function renamed to reflect new functionality
```cpp
// Renamed from UpdateGuideArrowTarget to UpdateGuideHighlight
// Now clearly indicates it only handles outline highlighting
static void UpdateGuideHighlight(UAssembleStep* Step)
```

### Guidelines
1. Use clear, descriptive names that indicate what the function actually does
2. Update all call sites when renaming
3. Consider if the function should be split into multiple functions instead
