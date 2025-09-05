<#
 AARTZE Python Editor bootstrap (Windows PowerShell)

 - Ensures Python 3.12 is available via the py launcher
 - Creates a 3.12 virtualenv in .venv312
 - Installs PySide6 + NodeGraphQt
 - Builds the C++ binding (aartze.pyd) with CMake preset
 - Copies the module next to the Python editor
 - Runs editor_py/main.py using the venv

 Usage:
   powershell -ExecutionPolicy Bypass -File .\bootstrap.ps1
#>

Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'

function Write-Section($msg){
  Write-Host "`n=== $msg ===" -ForegroundColor Cyan
}

# 1) Check for Python
Write-Section "Checking Python"
function Ensure-Python312 {
  param()
  $pathsToProbe = @(
    "$env:LOCALAPPDATA\Programs\Python\Python312\python.exe",
    "$env:ProgramFiles\Python312\python.exe",
    "$env:ProgramFiles(x86)\Python312\python.exe"
  )
  # Prefer the launcher and check exit code explicitly
  $la = (Get-Command py -ErrorAction SilentlyContinue)
  if ($la) {
    & py -3.12 -c "import sys;print('ok')" | Out-Null
    if ($LASTEXITCODE -eq 0) { return $true }
  }
  foreach($p in $pathsToProbe){ if(Test-Path $p){ return $true } }
  return $false
}

function Get-Python312Exe {
  try { $null = & py -3.12 -c "import sys;print('ok')"; return 'py -3.12' } catch {}
  $candidates = @(
    "$env:LOCALAPPDATA\Programs\Python\Python312\python.exe",
    "$env:ProgramFiles\Python312\python.exe",
    "$env:ProgramFiles(x86)\Python312\python.exe"
  )
  foreach($p in $candidates){ if(Test-Path $p){ return $p } }
  return $null
}

if (-not (Ensure-Python312)) {
  Write-Warning "Python 3.12 not found via 'py -3.12'. Attempting winget install (requires Windows 10/11)."
  $winget = (Get-Command winget -ErrorAction SilentlyContinue)
  if ($winget) {
    Write-Section "Installing Python 3.12 via winget"
    try {
      & winget install -e --id Python.Python.3.12 --accept-source-agreements --accept-package-agreements
    } catch {
      Write-Warning "winget install may have failed or needs elevation: $_"
    }
  } else {
    Write-Warning "winget not available. Attempting direct installer download from python.org"
    $url = "https://www.python.org/ftp/python/3.12.4/python-3.12.4-amd64.exe"
    $tmp = Join-Path $env:TEMP "python312_installer.exe"
    try { Invoke-WebRequest -Uri $url -OutFile $tmp -UseBasicParsing } catch { Write-Error "Download failed. Install Python 3.12 manually from python.org"; throw }
    Write-Section "Running Python 3.12 installer (quiet)"
    try {
      & $tmp /quiet InstallAllUsers=0 PrependPath=1 Include_launcher=1 Include_pip=1
    } catch {
      Write-Warning "Installer returned an error (may still have installed): $_"
    }
  }
}

if (Ensure-Python312) {
  Write-Host "Python 3.12 detected" -ForegroundColor Green
  $preferredPy = Get-Python312Exe
  $venv = Join-Path $PSScriptRoot ".venv312"
} else {
  Write-Warning "Python 3.12 not detected. Using current Python (pinning packages accordingly)."
  $pyCmd = (Get-Command python -ErrorAction SilentlyContinue)
  if (-not $pyCmd) { throw "No 'python' command found on PATH. Install Python first." }
  $preferredPy = $pyCmd.Path
  $venv = Join-Path $PSScriptRoot ".venv"
}

# 2) Create venv
Write-Section "Creating virtual environment ($venv)"
if (!(Test-Path $venv)) {
  if ($preferredPy -eq 'py -3.12') {
    & py -3.12 -m venv $venv
    if ($LASTEXITCODE -ne 0) {
      Write-Warning "Failed to create venv with 'py -3.12'. Falling back to 'python'."
      & python -m venv $venv
    }
  } else {
    & "$preferredPy" -m venv $venv
  }
}
$python = Join-Path $venv "Scripts\python.exe"
if (!(Test-Path $python)) {
  Write-Warning "venv python not found at $python. Creating venv with system 'python'..."
  & python -m venv $venv
}
if (!(Test-Path $python)) { Write-Error "venv python not found at $python" }

# 3) Install packages
Write-Section "Installing Python packages (PySide6, Addons/WebEngine, NodeGraphQt, QtAwesome, PyOpenGL)"
& $python -m pip install --upgrade pip
# Detect runtime version inside venv
$vinfo = & $python -c "import sys; print(f'{sys.version_info.major} {sys.version_info.minor}')"
$vparts = ($vinfo | Out-String).Trim().Split(' ')
if ($vparts.Length -ge 2) { $maj=[int]$vparts[0]; $min=[int]$vparts[1] } else { $maj=3; $min=12 }
# QtWebEngine lives in the Addons wheel. Pin Addons to the same version as PySide6.
if ($maj -gt 3 -or ($maj -eq 3 -and $min -ge 13)) {
  & $python -m pip install "PySide6==6.9.2" "PySide6-Addons==6.9.2" NodeGraphQt QtAwesome PyOpenGL
} else {
  & $python -m pip install "PySide6==6.7.3" "PySide6-Addons==6.7.3" NodeGraphQt QtAwesome PyOpenGL
}

# 4) Build C++ binding with CMake preset
Write-Section "vcpkg (manifest mode)"
# This repo uses vcpkg manifest mode. Do NOT pass package names.
# CMake will resolve ports automatically during configure.
try {
  $vcpkg = Join-Path $PSScriptRoot "vcpkg/vcpkg.exe"
  if (Test-Path $vcpkg) {
    Write-Host "vcpkg detected; manifest will be consumed by CMake. Skipping explicit install." -ForegroundColor DarkGray
    # In this repo, we rely on vcpkg's default builtin registry. Do not rewrite builtin-baseline.
  } else {
    Write-Warning "vcpkg not found at vcpkg/vcpkg.exe; relying on toolchain file in presets."
  }
} catch { Write-Warning "vcpkg check failed: $_" }

Write-Section "Configuring & building C++ binding (Python module)"
& cmake --preset windows-vs -DBUILD_AARTZE_PYTHON=ON
& cmake --build build --config Release

# 5) Copy module next to the editor
Write-Section "Locating built module"
$pyd = Get-ChildItem -Recurse -ErrorAction SilentlyContinue (Join-Path $PSScriptRoot "build") -Include "aartzepy*.pyd","aartze*.pyd" | Select-Object -First 1
if (-not $pyd) {
  Write-Warning "Python module not found after Release build. Trying Debug build..."
  & cmake --build (Join-Path $PSScriptRoot "build") --config Debug
  $pyd = Get-ChildItem -Recurse -ErrorAction SilentlyContinue (Join-Path $PSScriptRoot "build") -Include "aartzepy*.pyd","aartze*.pyd" | Select-Object -First 1
}
if (-not $pyd) { Write-Error "Could not find Python .pyd in build/ (did the build succeed?)" }
$dst = Join-Path $PSScriptRoot "editor_py"
if (!(Test-Path $dst)) { New-Item -ItemType Directory -Path $dst | Out-Null }
Copy-Item -Force $pyd.FullName (Join-Path $dst $pyd.Name)
Write-Host "Copied $($pyd.Name) -> $dst" -ForegroundColor Green

# 6) Run the editor
Write-Section "Launching Python editor"
& $python (Join-Path $dst "main.py")
