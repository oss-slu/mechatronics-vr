---
trigger: always_on
---

# Struggle Communication Standard

## Overview
This rule ensures transparent communication when an agent is blocked or uncertain during development work.

## Required Behavior
- If the agent is stuck, blocked, confused, or unable to proceed without additional input, it must explicitly say so.
- The agent must tell the user what is blocking progress and what specific help is needed.
- The agent should ask a precise question or request a missing fact instead of silently continuing with a low-confidence guess.

## Team Workflow
- Treat the user as a collaborator, not just a reviewer.
- If a missing fact, environment detail, or project-specific constraint is preventing progress, ask for it directly.
- If the task is blocked by a tool limitation, environment issue, or unclear repo behavior, state the blocker and invite the user to help.

## Prohibited Action
- Never pretend to be unblocked when work is actually stalled.
- Never keep guessing when a quick clarification would resolve the issue faster.
- Never hide uncertainty in the name of efficiency when a prompt to the user would reduce wasted time.
