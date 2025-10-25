# VEK

**VEK (Vantor Engine Kernel)** is my personal experimental project aimed at creating a scalable and modular framework for building rendering systems and game engines with a focus on performance, clarity, and developer productivity.

## Overview

VEK serves as a foundation layer for custom engine development, offering a clean and extensible architecture designed to support **real-time graphics**, **simulation**, and **hardware interaction**.
It provides a set of tools and abstractions that allow developers to rapidly prototype and evolve complex engine systems without sacrificing control or performance.

## Origin

The project traces its roots to the **Vantor Engine**, my earlier 3D engine framework. Many of the original core systems were extracted, refactored, and generalized into what is now known as the **Vantor Engine Kernel (VEK)**.
The goal was to turn a monolithic game engine into a reusable SDK—one that could serve as a base for new engines, experiments, or rendering backends.

## Design Goals

 - Scalability: Built to handle small prototypes and large-scale real-time applications alike.

 - Modularity: Each subsystem is self-contained, making it easy to integrate or replace.

 - Performance: Emphasis on minimal overhead and efficient memory management.

 - Cross-Platform Compatibility: Structured to support multiple platforms and hardware backends.

 - Clarity: Clean code organization and architecture-first design principles.

## Architecture

VEK is structured around core systems and extension modules, such as:

- Core Runtime: Handles memory management, threading, and event systems.

- Graphics Layer: Provides abstractions for rendering APIs (e.g., Vulkan, DirectX, OpenGL).

- Math and Utilities: Includes vector/matrix math, geometry utilities, and timing systems.

- Platform Layer: Encapsulates platform-specific logic for input, windowing, and device handling.

## Future Direction

The long-term vision for VEK is to evolve into a developer-focused SDK that simplifies the process of experimenting with engine designs, custom renderers, and low-level graphics interfaces.
Planned developments include:

- Advanced scene graph

- Cross-platform build and deployment support

- GPU-driven rendering pipelines

- Editor integration layer

## Collaboration

VEK is a **private hobby project** maintained by me and a small circle of friends.
It is not open for public contributions at this time. Feedback and discussions are welcome, but code submissions will only be accepted from collaborators explicitly invited to the project.

## License

This project is licensed under the GNU General Public License v3.0 (GPLv3).
See the LICENSE