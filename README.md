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

## Building

### Prerequisites

- Visual Studio 2022 or later (Windows)
- C++ compiler with C++17 or later support
- vcpkg (for dependency management)

### Build Steps

1. Clone the repository:
```bash
git clone https://github.com/NeonStarrySky/NeonEngine.git
cd NeonEngine
```

2. Install dependencies using vcpkg:
```bash
vcpkg install --config=vcpkg-configuration.json
```

3. Open the solution file:
```bash
NeonEngine.slnx
```

4. Build using Visual Studio or command line:
```bash
msbuild NeonEngine.slnx /p:Configuration=Release /p:Platform=x64
```

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
