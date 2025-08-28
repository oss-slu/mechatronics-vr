# MechatronicsVR

Developed in the Unreal Engine

A Virtual Reality training system built in Unreal Engine for mechatronics assembly and education. Learn mechanical assembly through immersive VR interactions with realistic part manipulation and guided lessons.


## Overview

MechatronicsVR provides an interactive VR environment for learning mechanical assembly skills. Users can grab, manipulate, and assemble mechanical parts with realistic physics while following structured lessons.

### Key Features

- **VR Assembly Training**: Hands-on learning with realistic part interaction
- **Physics-Based Manipulation**: Natural grab and place mechanics with physics simulation
- **Snap-Based Assembly**: Parts automatically connect at predefined attachment points
- **Guided Lessons**: Step-by-step instruction system with progress tracking
- **Visual Feedback**: Ghost previews and highlighting for assembly guidance

## System Components

### VR Interaction
- Motion controller support with grab mechanics
- Beam pointing system for distant object selection
- Haptic feedback and visual highlighting

### Assembly System
- **Parts**: Physics-enabled objects with assembly capabilities
- **Snap Points**: Predefined connection locations with compatibility rules
- **Validation**: Smart checking for correct part connections
- **Preview**: Ghost visualization showing valid placement locations

### Lesson Management
- **Structured Learning**: Data-driven lesson definitions
- **Step Progression**: Guided workflow through assembly processes
- **Progress Tracking**: Monitor completion and provide feedback
- **UI Integration**: Instruction display and user guidance

## Technical Stack

- **Engine**: Unreal Engine 5
- **Platform**: Windows VR (SteamVR compatible)
- **Language**: C++ with Blueprint integration
- **Input**: Enhanced Input System for VR controls

## Getting Started

1. Open the project in Unreal Engine 5
2. Ensure VR headset is connected and configured
3. Load a lesson map from Content/Maps
4. Enter VR mode to begin training

### VR Controls
- **Trigger**: Grab and release parts
- **Point**: Use beam to highlight distant objects
- **Move**: Teleport or room-scale movement

## Project Structure

The system is built around three core architectures:

1. **VR Grab System**: Handle object interaction and manipulation
2. **Part Assembly System**: Manage snap connections and validation
3. **Lesson Manager System**: Control learning progression and UI

Each system is designed to work independently while integrating seamlessly for the complete training experience.