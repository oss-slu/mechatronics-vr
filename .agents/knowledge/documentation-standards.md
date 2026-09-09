---
trigger: always_on
---

# Documentation Standards

## Overview
This knowledge note covers the project guidance for writing clear human-readable documentation in Unreal Engine C++ and Blueprint code.

## Why Documentation Matters
When developing C++ and Blueprint systems in Unreal Engine, code comments feed the engine reflection system and Blueprint node tooltips. Writing descriptive Doxygen comments in `.h` files ensures that students, capstone team members, and faculty can understand and maintain the code in both C++ and the Blueprint editor.

## Key Practices
- Use `/** ... */` Javadoc/Doxygen syntax above function declarations.
- Provide a summary sentence explaining the function's high-level intent.
- Detail edge cases, state mutations, and delegate events triggered by the function.
- Explicitly describe `@param` and `@return` values.
- In `.cpp` files, comment tricky algorithms, coordinate transformations, and physics constraint setups inline.

## Example Pattern
```cpp
/**
 * Briefly explains the function's purpose.
 *
 * Explains lifecycle assumptions, side effects, or delegate calls.
 *
 * @param InputName Description and constraints.
 * @return Meaning of the value returned.
 */
UFUNCTION(BlueprintCallable, Category = "Mechatronics|Subsystem")
ReturnType FunctionName(ParameterType InputName);
```

## Guidance
- Explain the "why" and "what" rather than restating the function name.
- Document preconditions and side effects clearly.
- Keep comments useful to both programmers and students working in Blueprint editor contexts.
