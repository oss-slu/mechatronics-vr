# Documentation Standards and Subagent Quota Efficiency

## Human-Readable Function Documentation in Unreal Engine 5

When developing C++ and Blueprint systems in Unreal Engine, code comments directly feed the engine's reflection system and Blueprint node tooltips. Writing descriptive Doxygen comments in `.h` files ensures that students, capstone team members, and faculty can understand and maintain the code in both C++ and the Blueprint editor.

### Key Practices:
- Always use the `/** ... */` Javadoc/Doxygen syntax above function definitions.
- Provide a summary sentence explaining the function's high-level intent.
- Detail edge cases, state mutations, and delegate events triggered by the function.
- Explicitly describe `@param` (valid range, nullability) and `@return` values.
- In `.cpp` files, comment tricky algorithms, coordinate transformations, and physics constraint setups inline.

---

## Quota Efficiency and Subagent Management

To prevent rapid exhaustion of API quotas and avoid cluttering the primary agent context:

### 1. Leverage Tiered Models for Subagents
- **`flash` / `flash_lite`**: Perfect for scanning files, symbol cross-referencing, verifying whether methods are documented, and drafting docstrings.
- **`inherit` / `pro`**: Used exclusively when performing complex architectural work or deep multi-module debugging.

### 2. Narrow Scoping
- Limit subagents to specific files and specific function targets rather than broad directory-wide audits.
- Request only the extracted docstrings or concise findings in responses rather than full file dumps.

### 3. In-Situ Documentation
- Write documentation concurrently with function creation or modification. Do not defer documentation passes to future sessions, as re-reading large files consumes additional context and quota.
