---
trigger: always_on
---

# Unreal Editor Session Reuse

## Overview
This rule prevents unnecessary duplicate Unreal Editor instances during validation and testing work.

## Required Behavior
- Before launching a new Unreal Editor process, check whether an editor instance is already running for this project.
- If an Unreal Editor is already open and the project is active, reuse that session instead of creating another instance.
- Prefer the existing editor session for Unreal MCP validation, automation, and test execution.

## Mandatory Workflow
1. Check for an active Unreal Editor process for the MechatronicsVR project.
2. Check that the Unreal Remote Control API and MCPUnreal endpoints are responding.
3. Reuse the existing live session for building, testing, and runtime validation.
4. Only start a new Unreal instance when no valid editor session exists or the active session is clearly unavailable.

## Prohibited Action
- Never start a second Unreal Editor instance while an existing project editor session is already running and usable.
- Do not spawn a fresh Unreal process simply because the previous workflow attempted an editor launch without checking the current state first.
