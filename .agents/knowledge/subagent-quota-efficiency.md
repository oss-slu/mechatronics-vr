---
trigger: always_on
---

# Subagent Quota Efficiency

## Overview
This knowledge note covers how to manage delegated work efficiently while preserving the primary agent's context and minimizing quota usage.

## Model Tiering
- Use `flash` or `flash_lite` for scanning files, symbol lookup, and documentation drafting.
- Reserve `inherit` or `pro` for architecture work, deep debugging, or multi-step reasoning.

## Best Practices
- Keep subagent prompts narrow and specific to the files or functions being examined.
- Request only targeted excerpts, summaries, or drafted doc blocks instead of whole-file dumps.
- Write documentation while editing code rather than postponing it to a later pass.
- Use `Workspace: "inherit"` when possible to reduce unnecessary workspace duplication.

## Why It Matters
Narrow delegated tasks reduce quota consumption, preserve the main working context, and keep reviews focused on only the needed code paths.
