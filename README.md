# NeonEngine

NeonEngine is a Windows C++23 project for experimenting with an OpenGL sprite renderer and a small entity/component framework. The current executable is a rendering and performance demo; this repository is still under active development and is not a complete game engine.

## What is in the repository

- A GLFW window and OpenGL rendering code using GLAD.
- Basic entity, component-storage, and physics code.
- Three sprite rendering paths that can be compared in the demo: individual draws, array rendering, and texture-grouped array rendering.
- A demo that creates 100,000 moving colored sprites and periodically logs frame timing and FPS statistics.

The demo currently creates its colored textures in memory. Image loading and texture-management classes are also present, but are not used for the demo's sprite colors.

## Build requirements

- Windows x64.
- Visual Studio C++ Build Tools with the **v145** platform toolset and a Windows 10 SDK.
- PowerShell.
- vcpkg. Set `VCPKG_ROOT` to the vcpkg directory or add `vcpkg.exe` to `PATH`.

The project is `Engine/Engine.vcxproj`, included in `NeonEngine.slnx`. It uses C++23 and vcpkg manifest mode. Dependencies are declared in `vcpkg.json`; the registry baseline is in `vcpkg-configuration.json`.

## Build

Clone the repository, change to its root directory, then run:

```powershell
git clone https://github.com/NeonStarrySky/NeonEngine.git
cd NeonEngine
./scripts/build.ps1
```

The script installs the manifest dependencies for the `x64-windows` triplet, locates MSBuild from `PATH` or Visual Studio Installer, then builds x64 Release. The executable and runtime DLLs are placed in `x64/Release/`.

To build Debug, use:

```powershell
./scripts/build.ps1 -Configuration Debug
```

If dependencies are already installed, `./scripts/build.ps1 -SkipDependencyInstall` skips the vcpkg install command. For a fresh setup, install vcpkg first and set `VCPKG_ROOT` to the directory containing `vcpkg.exe`.

## Run the demo

The window can be closed with **Escape** or the window close button. Press **1**, **2**, or **3** to switch rendering modes:

| Key | Rendering mode |
| --- | --- |
| `1` | Individual sprite draws |
| `2` | Sprite array renderer (default) |
| `3` | Texture-grouped sprite array renderer |

The program writes diagnostic and frame-rate information to its logger while running. Shader files are loaded from paths relative to the process working directory, so launch the executable from the repository root:

```powershell
& .\x64\Release\Engine.exe
```

For a Debug build, use `& .\x64\Debug\Engine.exe` instead. The working directory should remain the repository root so `Engine/assets/shaders/` resolves correctly.

## Source layout

```text
Engine/
  core/                 Engine settings, logging, input, entity/component code
  gameplay/             Gameplay-related declarations
  graphics/             Image, texture, frame-rate, and rendering code
    gl/                 GLFW/OpenGL window, mesh, shader, and renderer code
  assets/shaders/       GLSL shaders used by the demo
  main.cpp              Demo entry point and render loop
scripts/
  build.ps1             Windows dependency setup and build helper
NeonEngine.slnx         Visual Studio solution
vcpkg.json              vcpkg manifest
```

## License

The project includes an MIT license in [LICENSE.txt](LICENSE.txt).
