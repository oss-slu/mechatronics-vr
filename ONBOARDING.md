# MechatronicsVR Developer Onboarding

## Table of Contents
- [Prerequisites](#prerequisites)
- [Recommended IDE](#recommended-ide)
- [Configuring Visual Studio Build Tools](#configuring-visual-studio-build-tools)
- [Cloning and First Build](#cloning-and-first-build)
- [Troubleshooting](#troubleshooting)

## Prerequisites
- Unreal Engine 5.7
- Visual Studio Build Tools
- Windows 10/11

Unreal Engine can be downloaded [HERE](https://www.unrealengine.com/en-US/download)

Visual Studio Build Tools can be downloaded [HERE](https://visualstudio.microsoft.com/downloads/?q=build+tools)

## Recommended IDE
Jetbrains for Unreal Engine is the recommended IDE

Jetbrains for Unreal Engine can be downloaded [HERE](https://www.jetbrains.com/lp/rider-unreal/)

## Configuring Visual Studio Build Tools

- As shown below, ensure in **Workloads** that **Desktop Development with C++** and **.NET Multi-platform App UI Development** are installed

<img width="1321" height="593" alt="image" src="https://github.com/user-attachments/assets/c7dcd157-a8b9-4208-b301-9b9a9358991b" />

- As shown below, ensure in **Individual Components** that **.NET 8.0 Runtime** and **MSVC v143 v14.44-17.14** are installed

<img width="997" height="616" alt="image" src="https://github.com/user-attachments/assets/5e14b07e-e117-4999-b611-0e4681e27006" />

## Cloning and First Build

- Clone the project

```bash
git clone <url>
```
- Right click MechatronicsVR.uproject -> Generate Studio Visual Files
- Open MechatronicsVR.sln in an IDE
- Build the project
- The first build can take a while, do not be alarmed if it does

## Troubleshooting
The most common errors come from incorrect versions of MSVC or .NET, or an incorrect version of Unreal Engine is installed. If the initial build fails:
- Ensure that the correct version of Unreal Engine is installed
- Ensure that the correct version of .NET and MSVC are installed
If the build is still failing, the Unreal Engine build configuration could be wrong
- Check that BuildConfiguration.xml has the correct MSVC compiler version set, located at %AppData%\Unreal Engine\UnrealBuildTool\BuildConfiguration.xml

```xml
<Configuration xmlns="https://www.unrealengine.com/BuildConfiguration">
  <WindowsPlatform>
    <CompilerVersion>14.44</CompilerVersion>
  </WindowsPlatform>
</Configuration>
```
