---
trigger: always_on
---

# Rule Authoring Standards

## Overview
This rule enforces strict structural and scoping requirements for all rule files created or maintained in `.agents/rules/`.

---

## 1. Single Topic Constraint
- Every rule file in `.agents/rules/` must address **exactly one single topic**.
- **No Compound Titles**: If a title contains the word "and" or covers multiple topics, split the content into separate, focused rule files (e.g., separate `relative-paths.md` from `environment-variables.md`).

---

## 2. Mandatory Always-On Trigger
- All rule files **must** include the following frontmatter at the very top:
  ```yaml
  ---
  trigger: always_on
  ---
  ```

---

## 3. Separation from Knowledge Base
- **Never duplicate rules into `.agents/knowledge/`**: Rules define prescriptive behavioral constraints and belong exclusively in `.agents/rules/`. The knowledge base is strictly reserved for lessons learned, troubleshooting steps, and architectural guides.
