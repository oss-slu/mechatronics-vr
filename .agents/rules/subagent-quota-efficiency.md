---
trigger: always_on
---

# Subagent Quota Efficiency Standards

## Overview
To maximize Gemini quota efficiency and preserve the primary agent's token context, follow these guidelines when delegating tasks to subagents.

---

## 1. Model Tiering for Delegated Tasks
1. **`flash_lite` or `flash`**:
   - Use for documentation drafting, reviewing code for missing comments, searching symbols, and running read-only inspection tasks.
   - Low token overhead and rapid turnaround.
2. **`inherit` (default) or `pro`**:
   - Reserved for complex architectural refactoring, tricky debugging sessions, or tasks requiring deep multi-step reasoning.

---

## 2. Subagent Best Practices
- **Narrow Task Prompts**: Pass specific file paths and target function names to subagents. Avoid open-ended instructions like "read the whole project and document everything".
- **Compact Responses**: Instruct subagents to return only relevant excerpts, drafted doc blocks, or concise summaries rather than reprinting entire source files.
- **In-Situ Documentation**: When implementing or editing code, write the documentation immediately within the same edit step. This eliminates the need for redundant scanning passes and context re-reads later.
- **Workspace Sharing**: Use `Workspace: "inherit"` when spawning subagents for research or doc generation to prevent disk cloning overhead.
