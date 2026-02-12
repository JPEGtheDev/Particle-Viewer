# Particle-Viewer 

## TL;DR: 
A viewer for N-Body simulations.

___

## Installation

### Linux (Flatpak - Recommended)

Download the latest `.flatpak` file from [Releases](https://github.com/JPEGtheDev/Particle-Viewer/releases) and install:

```bash
flatpak install --user particle-viewer-<version>.flatpak
flatpak run org.particleviewer.ParticleViewer
```

**Benefits:**
- ✅ No dependency installation required
- ✅ Sandboxed and secure
- ✅ Works on any Linux distribution
- ✅ Single-file distribution

See [FLATPAK.md](docs/flatpak/FLATPAK.md) for detailed installation and building instructions.

### Building from Source

**Requirements:**
- CMake 3.24 or higher
- C++20 compatible compiler:
  - GCC 10 or later
  - Clang 11 or later  
  - MSVC 2019 or later (Visual Studio 2022 recommended)
- GLFW 3.3+ development libraries
- GLM development libraries
- OpenGL development libraries

**Build Steps:**
```bash
# Install dependencies (Ubuntu/Debian)
sudo apt-get install cmake build-essential libglfw3-dev libglm-dev

# Configure and build
cmake -B build -S .
cmake --build build

# Run
./build/Viewer
```

For development or other platforms, see the build instructions below.

___

## Long Description:

This is a project created by Jonathan Petz for Tarleton State University's Particle Modeling Group. The purpose of this application is to view N-Body simulations in a 3D environment. The secondary purpose of this application is to take screenshots, to be compiled into a video, in order to demonstrate the particle group's findings. The final purpose of this application is to allow me to create a pseudo game engine / hone in on my OpenGl programming skills, while also providing a viewer to the TSU Particle Modeling Group.

## Currently Used Libraries:

### Needs Development Libraries Installed Locally:
* [GLFW](https://github.com/glfw/glfw) 
* [GLM](https://github.com/g-truc/glm) 

### Embedded
* [stb](https://github.com/nothings/stb) 
* [Tiny File Dialogs](http://sourceforge.net/projects/tinyfiledialogs/)
* [GLAD](https://github.com/Dav1dde/glad)

### Downloaded at Build Time (via CMake FetchContent)
* [Dear ImGui](https://github.com/ocornut/imgui) - Immediate mode GUI for menus and debug tools

**Note:** For detailed license information and attributions for all third-party libraries, see [THIRD_PARTY_NOTICES.md](THIRD_PARTY_NOTICES.md).

___
## Utilities Used:
[CMAKE](https://cmake.org/")
___

## Features

### Window and Resolution Management

Particle-Viewer supports runtime resolution changes and fullscreen mode:

- **Resolution Selection**: Choose from 720p, 1080p, 1440p, or 4K via the View menu
- **Fullscreen Toggle**: Press **Alt+Enter** to toggle fullscreen mode
- **Manual Resizing**: Drag window edges to resize — the viewport adjusts automatically
- **Persistent Settings**: Window size and fullscreen state are saved between sessions
- **Monitor Constraints**: Automatically clamps resolution to your monitor's capabilities

See [Window Management Guide](docs/WINDOW_MANAGEMENT.md) for detailed instructions.

### Other Features

- 3D visualization of N-Body particle simulations
- Frame-by-frame playback control
- Screenshot capture and video frame recording
- Center-of-mass tracking and visualization
- ImGui-based menu system with debug overlay

___

## Contributing

We welcome contributions! Please follow our [Coding Standards](docs/CODING_STANDARDS.md) to maintain code quality and consistency.

**Key Guidelines:**
- Code formatting is enforced via `clang-format` (see `.clang-format`)
- Static analysis via `clang-tidy` (see `.clang-tidy`)
- Follow Microsoft C++ Core Guidelines
- Use [Conventional Commits](https://www.conventionalcommits.org/) for all commit messages
- All pull requests are checked automatically in CI

## Releases

This project uses **automated semantic versioning** with zero-manual releases. See [Release Process](docs/RELEASE_PROCESS.md) for details on:
- How versions are automatically determined from commit messages
- Conventional commit format for controlling version bumps
- Automated changelog generation
- Release workflow and best practices

___

## Why this is under a MIT license:
I wanted to be able to share the code that I have created for the group with anyone else who wants to create videos of their N-Body simulations. I also want people to use this as a starting point, and modify this application in order to make it better. This allows the group to continue the particle viewer, long after I have left the group / University, while also allowing outside sources to assist in any development issues / to improve the viewer.
