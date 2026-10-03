# NeonEngine build and run guide

These instructions were checked against the Visual Studio project and a successful local build. Recheck the project files if its toolchain or layout changes; the README may be stale.

## Requirements

- Windows and Visual Studio C++ Build Tools with the `v145` platform toolset and a Windows 10 SDK.
- The solution is `NeonEngine.slnx`; the C++ project is `Engine/Engine.vcxproj`.
- C++23 is selected in the project.
- Dependencies are declared in the root `vcpkg.json`. The project enables vcpkg manifest mode and uses `vcpkg_installed` as its install directory for x64.

## Build

Run these commands from the repository root in PowerShell. Prefer x64 Release:

```powershell
$msbuild = (Get-Command msbuild.exe -ErrorAction SilentlyContinue).Source
if (-not $msbuild) {
    $vswhere = Join-Path ${env:ProgramFiles(x86)} 'Microsoft Visual Studio\Installer\vswhere.exe'
    $vs = & $vswhere -latest -products '*' -requires Microsoft.Component.MSBuild -property installationPath
    $msbuild = Join-Path $vs 'MSBuild\Current\Bin\MSBuild.exe'
}
& $msbuild .\NeonEngine.slnx /m /p:Configuration=Release /p:Platform=x64 /v:minimal
if ($LASTEXITCODE -ne 0) { throw "MSBuild failed with exit code $LASTEXITCODE" }
```

The executable is `x64/Release/Engine.exe`. MSBuild also places the required `fmt.dll` and `glfw3.dll` beside it. For a Debug build, change `Release` to `Debug` in the build command; the executable is then `x64/Debug/Engine.exe`.

If the manifest dependencies are absent, install vcpkg and run `vcpkg install --triplet x64-windows` from the repository root, then build again. This checkout already had the manifest dependencies installed when the build was verified.

## Run

Shader filenames are opened relative to the process working directory as `assets/shaders/...`. The source assets are under `Engine/assets`, so run the executable with `Engine` as its working directory. From the repository root:

```powershell
$repo = (Get-Location).Path
Push-Location .\Engine
try {
    & "$repo\x64\Release\Engine.exe"
} finally {
    Pop-Location
}
```

The program opens an OpenGL window and runs its render loop until the window is closed or Escape is pressed. Keep the executable path on the output directory while setting the working directory to `Engine`; starting it with an unrelated working directory can make shader loading fail.

## Verified build and known warnings

On 2026-10-03, the x64 Release solution build completed successfully with MSBuild 18.10.1 and Windows SDK 10.0.26100.0. It produced `x64/Release/Engine.exe`. The build emitted warnings for `APIENTRY` macro redefinition, a `size_t` to `int` conversion in `Engine/core/engine.cpp`, and `double` to `float` conversion in `Engine/main.cpp`; these warnings did not prevent linking.

In restricted environments, MSBuild may fail during SDK discovery with an access-denied error under `%LOCALAPPDATA%\Microsoft SDKs` before compiling source files. If so, allow the build process to read the installed Windows SDK discovery metadata and retry. A successful retry in the environment above used the installed SDK; the initial failure was an environment permission issue, not a source compile error.
