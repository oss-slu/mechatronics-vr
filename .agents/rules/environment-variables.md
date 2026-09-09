---
trigger: always_on
---

# Environment Variables Standard

## Overview
When external machine-dependent paths or system installations (such as engine installs or external tool binaries) are required, environment variables must be used. Whenever an environment variable is introduced, a clear setup mechanism must be provided.

---

## 1. Setup Mechanism Requirement
Whenever an environment variable is introduced or required for a system, tool, or script:
1. **Provide a Clear Setup Mechanism**: There must always be an easy, documented way for others (teammates, contributors, CI) to configure the environment variable.
2. **Setup Methods**:
   - Provide an example configuration file (e.g., `.env.example` or template config).
   - Document the variable's purpose, default value, and configuration instructions in the relevant `README.md` or setup guide.
   - Provide reasonable dynamic fallbacks in scripts/code whenever possible (e.g., auto-detecting default install locations if the variable is unset).
3. **Naming Conventions**: Use clear uppercase snake_case names prefixed with the project or tool name (e.g., `MECHATRONICS_PROJECT_DIR`, `UE_ENGINE_DIR`, `MCP_UNREAL_PROJECT`).

---

## 2. Implementation Guidelines

### In Scripts
- Support environment variable overrides with sensible defaults:
  ```bash
  PROJECT_DIR="${MECHATRONICS_PROJECT_DIR:-$(cd "$SCRIPT_DIR/.." && pwd)}"
  ```

### In C++ / Unreal Engine Code
- Read environment variables via `FPlatformMisc::GetEnvironmentVariable(TEXT("VARIABLE_NAME"))` with appropriate error handling and fallback values.

### In Configuration & Tool Files
- Reference environment variables (e.g., `${VAR_NAME}`).
- Provide a setup script or `.env.example` file that generates the local configuration.
