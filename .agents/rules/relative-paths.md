---
trigger: always_on
---

# Relative Paths Standard

## Overview
To maintain portability across different operating systems (Windows, Linux, WSL2), team members, and CI/CD environments, **hardcoded absolute paths must never be written**.

---

## 1. Prohibit Hardcoded Absolute Paths
- **Never** hardcode machine-specific absolute paths (e.g., `/mnt/d/...`, `D:/Capstone/...`, `C:/Users/...`, `/home/username/...`) in any codebase files, scripts, configuration files, tests, or documentation.
- All paths within the repository must be specified as **relative paths** (relative to the workspace/project root, script execution directory, or current module).

---

## 2. Implementation Guidelines

### In Scripts (Bash / PowerShell / Python)
- Derive paths dynamically from the script's own location:
  - **Bash**: `SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"` and resolve relative paths from `SCRIPT_DIR`.
  - **Python**: `Path(__file__).resolve().parent` to locate project assets or root directory.
  - **PowerShell**: `$PSScriptRoot` to compute relative paths.

### In C++ / Unreal Engine Code
- Use Unreal Engine's built-in directory helpers instead of raw strings:
  - `FPaths::ProjectDir()`
  - `FPaths::ProjectContentDir()`
  - `FPaths::ProjectSavedDir()`
  - `FPaths::ProjectPluginsDir()`

### In Configuration & Tool Files
- Prefer relative path specifications (e.g., relative to project root or config file location).
