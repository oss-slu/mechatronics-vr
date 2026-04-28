# Unreal Engine 5 — Shipping Build Packaging Guide

## Overview

This document outlines the steps required to package a shipping build for an Unreal Engine 5 project. A shipping build is the final, optimized version of the project intended for distribution or deployment. Unlike Development or Debug builds, a Shipping build strips out console commands, logging, and debug tooling to produce a lean, release-ready executable.

Follow this guide any time a new packaging run is required — for example, after a significant feature merge, prior to a release milestone, or when validating a build for end-user delivery.

---

## Prerequisites

Before beginning the packaging process, confirm the following:

- **Unreal Engine 5** is installed and the correct engine version is associated with the project's `.uproject` file.
- The project compiles and runs successfully in the Editor without errors.
- All intended feature branches have been merged into the target branch (typically `main`) prior to packaging. Packaging from an incomplete branch will produce a build that does not reflect the final intended state.
- Sufficient disk space is available on the output drive. UE5 shipping builds can range from several hundred megabytes to multiple gigabytes depending on asset volume.

---

## Step 1 — Set the Build Configuration to Shipping

1. Open the project in the Unreal Engine 5 Editor.
2. In the toolbar, click the dropdown next to the **Launch** button (it may currently read `Development` or `DebugGame`).
3. Select **Shipping** from the build configuration dropdown.

> **Note:** Shipping mode disables `UE_LOG`, console variable access, and many debug utilities. If any project code relies on these at runtime, it should be wrapped in preprocessor guards (`#if !UE_BUILD_SHIPPING`) before packaging.

---

## Step 2 — Configure Project Settings

Navigate to **Edit → Project Settings** and review the following sections before packaging:

### General

- **Project Name** and **Version** should reflect the current release.
- Ensure the **Company Name**, **Copyright Notice**, and **Description** fields are populated if the build is intended for external distribution.

### Maps & Modes

- Under **Default Maps**, confirm that the **Game Default Map** and **Advanced Settings → Transition Map** point to the correct levels.
- Shipping builds will launch directly into the Game Default Map. An incorrect entry here will produce a build that opens the wrong level or fails to start.

### Packaging

Navigate to **Edit → Project Settings → Packaging**:

- Set **Build Configuration** to `Shipping`.
- Enable **Full Rebuild** for the first packaging run of a new release to avoid stale artifacts from previous builds.
- Confirm the **Staging Directory** (the output path for the packaged build) is set to a known, accessible location.
- If the project uses Blueprint-only assets, enable **Cook everything in the project content directory** to ensure no assets are missed.

---

## Step 3 — Package the Project

1. In the Editor menu, navigate to **Platforms → [Target Platform] → Package Project**.
   - For a Windows desktop build, select **Platforms → Windows → Package Project**.
2. A file dialog will prompt for an output directory. Select or create a destination folder for the packaged build.
3. Click **Select Folder**. The packaging process will begin.

The output log will appear in the bottom-right of the Editor. Packaging duration varies based on project size and whether a full rebuild is being performed — expect anywhere from a few minutes to over an hour for large projects.

---

## Step 4 — Monitor the Output Log

During packaging, monitor the **Output Log** panel (`Window → Output Log`) for errors or warnings.

Common issues to watch for:

| Message Type | Example | Action |
|---|---|---|
| Cook error | `LogCook: Error: Failed to cook asset` | Identify and fix the referenced asset, then repackage |
| Missing redirect | `LogLinker: Warning: Unable to find object` | Verify asset references and fix broken redirectors |
| Shader compile failure | `LogShaderCompilers: Error` | Check material nodes for incompatibilities with Shipping config |
| Plugin not enabled | `LogInit: Warning: Plugin X not found` | Enable the required plugin in `.uproject` and rebuild |

A successful packaging run will end with:

```
BUILD SUCCESSFUL
```

in the Output Log. If the process ends with `BUILD FAILED`, scroll up to locate the first error and resolve it before attempting another packaging run.

---

## Step 5 — Validate the Packaged Executable

Once packaging completes, navigate to the output directory and confirm the following:

- The expected executable (e.g., `MechatronicsVR.exe` on Windows) is present.
- Launch the executable directly — **outside of the Unreal Editor** — to simulate the end-user experience.
- Confirm the application opens to the correct default map without errors.
- If the project targets VR hardware, connect the headset and verify that immersive mode initializes correctly. Desktop validation alone is not sufficient to confirm VR readiness.

---

## Step 6 — Document the Build

After validation, create or update the project's packaging report to record:

- The date and branch the build was packaged from
- The build configuration used
- Desktop validation status
- VR validation status (if applicable)
- Any known issues or pending follow-up items

This ensures the team maintains a clear record of each build's state and readiness for release.

---

## Common Errors & Troubleshooting

### Build Fails with No Clear Error

- Try enabling **Full Rebuild** in Project Settings → Packaging and repackage.
- Delete the `Intermediate/` and `Saved/Cooked/` directories from the project root and repackage from clean.

### Executable Launches but Crashes Immediately

- Run the executable from a terminal to capture log output. On Windows, open a Command Prompt and run:
  ```
  MechatronicsVR.exe -log
  ```
  This forces the log window to appear and surfaces runtime errors that are otherwise silent in Shipping mode.

### VR Headset Not Detected

- Confirm the VR plugin (e.g., OpenXR or SteamVR) is enabled in the `.uproject` file.
- Verify the headset runtime (SteamVR, Oculus, etc.) is running before launching the executable.
- Ensure the VR plugin is not excluded from Shipping builds — check **Edit → Plugins** and confirm the plugin is not set to `Editor Only`.

### Packaged Build Loads the Wrong Map

- Return to **Edit → Project Settings → Maps & Modes** and verify the **Game Default Map** entry. Rebuild after correcting it.

---

## Notes

- Always package from a clean, fully merged branch. Packaging mid-development or from a feature branch will produce a build that does not represent the final intended state.
- Shipping builds should not be distributed until VR functionality (if applicable) and all target platform behaviors have been explicitly validated.
- If the packaging output directory is on a network drive or external disk, ensure the drive remains connected throughout the full packaging process to prevent incomplete output.

---

