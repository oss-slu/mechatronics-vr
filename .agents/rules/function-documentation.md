---
trigger: always_on
---

# Function Documentation Standards

## Overview
This rule ensures that all C++ and Blueprint code written or modified within the MechatronicsVR project is systematically documented for human developers, educators, and students.

---

## 1. Header Files (`.h`)
Every function declaration written or modified by the agent (and any undocumented function encountered during modifications) **must** include a structured, human-readable Doxygen block directly above it. Unreal Header Tool (UHT) parses these blocks into Blueprint node tooltips.

### Format:
```cpp
/**
 * Brief plain-English summary of what the function accomplishes.
 *
 * Extended explanation covering non-obvious details: edge cases, lifecycle
 * assumptions (e.g., must be called after BeginPlay), state mutations, or
 * delegate broadcasts.
 *
 * @param ParameterName Purpose and constraints of the parameter (e.g., non-null, valid range).
 * @return Meaning of the return value and any error indicators.
 * @note Any important execution constraints (e.g., Authority-only, VR tick timing).
 * @see RelatedClassOrFunction
 */
UFUNCTION(BlueprintCallable, Category = "Mechatronics|Subsystem")
ReturnType FunctionName(ParameterType ParameterName);
```

### Guidelines for Human Readability:
1. **Explain the "Why" and "What"**: Do not simply restate the function name (e.g., avoid `/** Sets the value */ void SetValue()`). Explain what setting the value impacts in the lesson or physics assembly system.
2. **Document Parameters & Preconditions**: Specify if pointers can be `nullptr`, valid numeric ranges, or required states (e.g., "Must be attached before calling").
3. **State Side Effects**: Mention if the function alters actor transforms, spawns components, triggers sound, or invokes multicast delegates.

---

## 2. Source Files (`.cpp`)
- Complex or multi-step algorithms must contain clear step-by-step comments explaining the reasoning behind the implementation steps.
- Avoid obvious restatements; focus on design rationale, coordinate space conversions, or physics constraint assumptions.
