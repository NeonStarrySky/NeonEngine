> **Disclaimer:** This README is AI-generated and may overstate the current completeness of the project. Some features may be incomplete, inaccurate, or not yet implemented.
# NeonEngine 🎮

A modern C++ graphics engine built for high-performance 3D rendering and game development.

## Overview

NeonEngine is a lightweight yet powerful graphics engine designed to provide developers with a flexible foundation for creating 3D applications and games. Built with modern C++ and leveraging industry-standard libraries, it offers efficient rendering capabilities with a clean and intuitive API.

## Features

- **Modern C++ Architecture** - Built with C++17/20 standards for performance and maintainability
- **OpenGL Support** - Utilizes GLAD for latest OpenGL API access
- **Cross-Platform** - Supports both x64 and x86 architectures
- **Math Library** - Integrated GLM for robust mathematical operations
- **Window Management** - GLFW-based windowing and input handling
- **Logging** - spdlog integration for efficient logging
- **Asset Loading** - STB library support for image and asset processing

## Dependencies

The project uses **vcpkg** for dependency management:

- **GLAD** - OpenGL loader (with latest GL API)
- **GLFW3** - Window and input management
- **GLM** - Mathematics library for graphics programming
- **STB** - Image loading and processing
- **spdlog** - Fast C++ logging library

## Building (Windows)

### Requirements

- Windows and Visual Studio C++ Build Tools with the **v145** platform toolset and a Windows 10 SDK.
- [vcpkg](https://github.com/microsoft/vcpkg), available as `vcpkg.exe` on `PATH` or through the `VCPKG_ROOT` environment variable.
- PowerShell.

The project uses C++23. Dependencies are described in `vcpkg.json`; the repository's `vcpkg-configuration.json` pins the vcpkg registry baseline.

### Build with the script

Clone the repository, then run this from its root in PowerShell:

```powershell
git clone https://github.com/NeonStarrySky/NeonEngine.git
cd NeonEngine
./scripts/build.ps1
```

The script installs the manifest dependencies for `x64-windows`, locates MSBuild through `PATH` or Visual Studio Installer's `vswhere`, and builds the solution in x64 Release mode. The executable and runtime DLLs are written to `x64/Release/`.

To build Debug instead, run `./scripts/build.ps1 -Configuration Debug`. If dependencies are already installed, pass `-SkipDependencyInstall` to skip the vcpkg install step.

### Build manually

From the repository root, install dependencies and build with MSBuild:

```powershell
vcpkg install --triplet x64-windows
msbuild .\NeonEngine.slnx /m /p:Configuration=Release /p:Platform=x64 /v:minimal
```

The solution is `NeonEngine.slnx`, and the C++ project is `Engine/Engine.vcxproj`.

### Run

Shaders are loaded from paths relative to the process working directory. Start the executable with `Engine` as its working directory:

```powershell
$repo = (Get-Location).Path
Push-Location .\Engine
try {
    & "$repo\x64\Release\Engine.exe"
} finally {
    Pop-Location
}
```

The program opens an OpenGL window and runs until the window is closed or Escape is pressed.

## Project Structure

```
NeonEngine/
├── Engine/                    # Core engine implementation
├── tool/                      # Development tools and utilities
├── NeonEngine.slnx           # Visual Studio solution file
├── vcpkg.json                # vcpkg manifest for dependencies
├── vcpkg-configuration.json  # vcpkg configuration
└── README.md                 # This file
```

## Getting Started

### Basic Usage

```cpp
// Example of basic engine usage
#include "Engine.h"

int main() {
    // Initialize engine
    NeonEngine::Engine engine;
    
    // Your rendering code here
    
    return 0;
}
```

## Architecture

The engine follows a modular architecture:

- **Rendering System** - Hardware-accelerated graphics rendering
- **Input System** - Cross-platform input handling
- **Math System** - Vector and matrix operations
- **Asset System** - Efficient resource management
- **Logging System** - Comprehensive debugging and logging

## License

This project is licensed under the MIT License - see the [LICENSE.txt](LICENSE.txt) file for details.

## Contributing

Contributions are welcome! Please feel free to submit a Pull Request.

## Roadmap

- [ ] Enhanced shader management system
- [ ] Physics engine integration
- [ ] Audio system
- [ ] Scene graph optimization
- [ ] Advanced debugging tools
- [ ] Documentation and tutorials

## Support

For issues, questions, or suggestions, please open an issue on GitHub.

---

**Author:** [@NeonStarrySky](https://github.com/NeonStarrySky)

**Created:** March 14, 2026
