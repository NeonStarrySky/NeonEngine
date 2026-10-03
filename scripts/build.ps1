param(
    [ValidateSet('Debug', 'Release')]
    [string]$Configuration = 'Release',
    [ValidateSet('x64')]
    [string]$Platform = 'x64',
    [switch]$SkipDependencyInstall
)

$ErrorActionPreference = 'Stop'
$repoRoot = Split-Path -Parent $PSScriptRoot
$solution = Join-Path $repoRoot 'NeonEngine.slnx'

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

$msbuild = Find-MSBuild

if (-not $SkipDependencyInstall) {
    $vcpkg = $null
    if ($env:VCPKG_ROOT) {
        $candidate = Join-Path $env:VCPKG_ROOT 'vcpkg.exe'
        if (Test-Path $candidate) { $vcpkg = $candidate }
    }
    if (-not $vcpkg) {
        $command = Get-Command vcpkg.exe -ErrorAction SilentlyContinue
        if ($command) { $vcpkg = $command.Source }
    }
    if (-not $vcpkg) {
        $installedStatus = Join-Path $repoRoot 'vcpkg_installed\vcpkg\status'
        if (Test-Path $installedStatus) {
            Write-Host 'vcpkg.exe was not found; using the manifest dependencies already installed in vcpkg_installed.'
        }
        else {
            throw 'vcpkg was not found. Install vcpkg, set VCPKG_ROOT to its directory (or add vcpkg.exe to PATH), then rerun this script.'
        }
    }

    if ($vcpkg) {
        Push-Location $repoRoot
        try {
            & $vcpkg install --triplet x64-windows
            if ($LASTEXITCODE -ne 0) {
                throw "vcpkg dependency installation failed with exit code $LASTEXITCODE"
            }
        }
        finally {
            Pop-Location
        }
    }
}

& $msbuild $solution /m "/p:Configuration=$Configuration" "/p:Platform=$Platform" /v:minimal
if ($LASTEXITCODE -ne 0) {
    throw "MSBuild failed with exit code $LASTEXITCODE"
}

$exe = Join-Path $repoRoot "$Platform\$Configuration\Engine.exe"
Write-Host "Build succeeded: $exe"
