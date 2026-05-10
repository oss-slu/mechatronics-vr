# Contributing to MechatronicsVR

Thank you for your interest in contributing to MechatronicsVR! This is a VR-based educational application for teaching mechatronics concepts through hands-on virtual assembly experiences.

## Table of Contents

- [Code of Conduct](#code-of-conduct)
- [Project Overview](#project-overview)
- [Getting Started](#getting-started)
- [Development Workflow](#development-workflow)
- [Coding Standards](#coding-standards)
- [Submitting Changes](#submitting-changes)
- [Reporting Issues](#reporting-issues)
- [Project Structure](#project-structure)

## Code of Conduct

This project is part of Open Source with SLU (OSS-SLU). All contributors are expected to:

- Be respectful, inclusive, and professional
- Provide constructive and helpful feedback
- Focus on collaboration and community benefit
- Show empathy and patience with other contributors
- Follow academic integrity standards

## Project Overview

MechatronicsVR is an educational VR application that allows users to:

- Learn mechatronics concepts through interactive lessons
- Assemble virtual mechanical and electrical components
- Understand how parts work together in real-world systems
- Practice assembly procedures in a safe virtual environment

### Target Platforms

- **Primary**: Meta Quest (Android)
- **Secondary**: Windows PC (for development and desktop VR)

### Key Technologies

- Unreal Engine 5.6
- Meta XR SDK
- Assembly Framework Plugin (custom snap-based assembly system)

## Getting Started

Please refer to **ONBOARDING.md** for instructions on how to get started

## Development Workflow

### Branch Strategy

We use a feature branch workflow:

```bash
main                    # Stable, production-ready code
├── feature/xyz         # New features
├── fix/issue-123       # Bug fixes
├── docs/readme-update  # Documentation
└── refactor/xyz        # Code refactoring
```

### Creating a Feature Branch

```bash
git checkout main
git pull origin main
git checkout -b feature/your-feature-name
```

### Commit Guidelines

Write clear, descriptive commit messages:

```
Add power supply assembly step

- Create PowerSupplyActor with snap points
- Add lesson step for power supply connection
- Update UI to show power supply instructions

Closes #45
```

Format:
- First line: Brief summary (50 chars or less)
- Blank line
- Body: Detailed explanation if needed
- Reference related issues

### Testing Your Changes

Before submitting:

1. **Build in Development configuration** - Ensure no compile errors
2. **Test in Editor** - Verify basic functionality
3. **Test in VR** - If you have a headset, test VR interactions
4. **Test on Quest** - For major features, test on actual hardware

## Coding Standards

### C++ Style

Follow Unreal Engine conventions:

```cpp
// Class naming
UCLASS()
class MECHATRONICSVR_API AMyActor : public AActor
{
    GENERATED_BODY()

public:
    // Properties with UPROPERTY for Blueprint access
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "My Category")
    float MyValue;

    // Functions with documentation
    /**
     * Performs an important action
     * @param Parameter Description of parameter
     * @return Description of return value
     */
    UFUNCTION(BlueprintCallable, Category = "My Category")
    bool DoSomething(int32 Parameter);

private:
    // Private members with b prefix for booleans
    bool bIsActive;
};
```

### Naming Conventions

| Type | Convention | Example |
|------|------------|---------|
| Classes | Prefix by type | `AMyActor`, `UMyComponent` |
| Interfaces | I prefix | `IMyInterface` |
| Enums | E prefix | `EMyState` |
| Structs | F prefix | `FMyStruct` |
| Booleans | b prefix | `bIsEnabled` |
| Functions | PascalCase | `GetCurrentState()` |

### Blueprint Standards

- Use clear, descriptive node names
- Comment complex logic with Comment nodes
- Organize with Reroute nodes for clarity
- Keep functions focused and modular

### Lesson System

When creating new lessons:

1. Create a `ULessonDataAsset` for the lesson
2. Define steps using appropriate step types:
   - `UAssembleStep` - For assembly tasks
   - `UFocusStep` - For focusing on specific parts
   - `UInteractionStep` - For interactions
3. Configure validation and feedback
4. Test the complete lesson flow

## Submitting Changes

### Pull Request Process

1. **Update your branch**
   ```bash
   git fetch origin
   git rebase origin/main
   ```

2. **Push to your fork/branch**
   ```bash
   git push origin feature/your-feature-name
   ```

3. **Create a Pull Request** on GitHub

4. **Fill out the PR template**:
   ```markdown
   ## Description
   What does this PR do?

   ## Type of Change
   - [ ] Bug fix
   - [ ] New feature
   - [ ] Breaking change
   - [ ] Documentation

   ## Testing
   How was this tested?
   - [ ] Tested in Editor
   - [ ] Tested in VR Preview
   - [ ] Tested on Quest device

   ## Screenshots/Videos
   If applicable, add visual documentation.

   ## Checklist
   - [ ] Code follows project style
   - [ ] Self-review completed
   - [ ] Documentation updated
   - [ ] No new warnings
   ```

### Review Process

- PRs require at least one review
- Address all feedback before merging
- Squash commits if requested
- Delete branch after merge

## Reporting Issues

### Bug Reports

Include:

1. **Environment**
   - UE version, OS, VR hardware
   - Steps to reproduce

2. **Expected vs Actual Behavior**

3. **Logs/Screenshots**
   - Check `Saved/Logs/` for crash logs
   - Include screenshots of visual bugs

### Feature Requests

Describe:
- The educational value of the feature
- How students would benefit
- Any technical considerations

## Project Structure

```
MechatronicsVR/
├── Config/                    # Engine and project configuration
├── Content/                   # All content assets
│   ├── Lessons/              # Lesson data assets
│   ├── Parts/                # Part meshes and blueprints
│   └── UI/                   # UI widgets
├── Plugins/
│   ├── AssemblyFramework/    # Snap assembly system
│   └── MetaXR/               # Meta Quest support
├── Source/
│   └── MechatronicsVR/
│       ├── Lesson/           # Lesson system
│       ├── Private/          # Implementation files
│       └── Public/           # Header files
└── MechatronicsVR.uproject
```

### Key Source Files

| File | Purpose |
|------|---------|
| `PartActor.h/cpp` | VR-grabbable assembly parts |
| `GrabComponent.h/cpp` | VR grab interaction |
| `LessonManagerComponent.h/cpp` | Lesson flow control |
| `AssembleStep.h/cpp` | Assembly lesson steps |
| `LessonDataAsset.h` | Lesson configuration |

## Questions?

- Open an issue with the "question" label
- Contact project maintainers
- Check existing issues and documentation

Thank you for contributing to MechatronicsVR and helping students learn mechatronics!
