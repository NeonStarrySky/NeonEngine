param(
    [ValidateSet('Debug', 'Release')]
    [string]$Configuration = 'Release',
    [ValidateSet('x64')]
    [string]$Platform = 'x64',
    [string]$VcpkgInstalledDir,
    [switch]$Run
)

$ErrorActionPreference = 'Stop'

# This script is deliberately ASCII-only.
# Windows PowerShell 5.1 reads a BOM-less UTF-8 script file with the ANSI code page,
# which can swallow the newline after a non-ASCII comment line and silently merge the
# next line into that comment (that bug made an earlier version of this script skip the
# VcpkgInstalledDir default below).

# temp\main_flow_split\scripts -> temp\main_flow_split
$tempRoot = Split-Path -Parent $PSScriptRoot
$solution = Join-Path $tempRoot 'NeonEngine.slnx'

# Walk upwards to find the original repository root: the directory that owns
# vcpkg_installed\x64-windows\include (this temp project has no vcpkg_installed of its own).
$repoRoot = ''
$probe = $tempRoot
while ($probe -ne '') {
    if (Test-Path (Join-Path $probe 'vcpkg_installed\x64-windows\include')) {
        $repoRoot = $probe
        break
    }
    $parent = Split-Path -Parent $probe
    if ($parent -eq '' -or $parent -eq $probe) { break }
    $probe = $parent
}

if ($VcpkgInstalledDir -eq '' -or $null -eq $VcpkgInstalledDir) {
    if ($repoRoot -eq '') {
        throw 'Could not locate the original repository (a directory that contains vcpkg_installed\x64-windows\include). Pass -VcpkgInstalledDir explicitly.'
    }
    # Reuse the dependencies already installed in the original repository.
    $VcpkgInstalledDir = Join-Path $repoRoot 'vcpkg_installed'
}

function Find-MSBuild {
    $command = Get-Command msbuild.exe -ErrorAction SilentlyContinue
    if ($command) {
        return $command.Source
    }

    $vswhere = Join-Path ${env:ProgramFiles(x86)} 'Microsoft Visual Studio\Installer\vswhere.exe'
    if (Test-Path $vswhere) {
        $installPath = & $vswhere -latest -products '*' -requires Microsoft.Component.MSBuild -property installationPath
        if ($LASTEXITCODE -eq 0 -and $installPath) {
            $candidate = Join-Path $installPath 'MSBuild\Current\Bin\MSBuild.exe'
            if (Test-Path $candidate) {
                return $candidate
            }
        }
    }

    throw 'MSBuild was not found. Install Visual Studio C++ Build Tools with the v145 toolset and a Windows 10 SDK.'
}

if (-not (Test-Path $solution)) {
    throw "Solution not found: $solution"
}

if (-not (Test-Path (Join-Path $VcpkgInstalledDir 'x64-windows\include'))) {
    throw "vcpkg dependencies not found under: $VcpkgInstalledDir (run 'vcpkg install --triplet x64-windows' in the original repository first)"
}

$msbuild = Find-MSBuild
Write-Host "MSBuild          : $msbuild"
Write-Host "Solution (temp)  : $solution"
Write-Host "VcpkgInstalledDir: $VcpkgInstalledDir"

& $msbuild $solution /m "/p:Configuration=$Configuration" "/p:Platform=$Platform" "/p:VcpkgInstalledDir=$VcpkgInstalledDir" /v:minimal
if ($LASTEXITCODE -ne 0) {
    throw "MSBuild failed with exit code $LASTEXITCODE"
}

$exe = Join-Path $tempRoot "$Platform\$Configuration\Engine.exe"
if (-not (Test-Path $exe)) {
    throw "Build reported success but the executable was not found: $exe"
}

Write-Host "Build succeeded: $exe"

if ($Run) {
    # Shader paths are relative to the process working directory, so run with Engine as cwd.
    $engineDir = Join-Path $tempRoot 'Engine'
    Write-Host 'Running Engine.exe (working directory = Engine). Close the window or press Escape to exit.'
    Push-Location $engineDir
    try {
        & $exe
        Write-Host "Engine.exe exited with code $LASTEXITCODE"
    }
    finally {
        Pop-Location
    }
}
