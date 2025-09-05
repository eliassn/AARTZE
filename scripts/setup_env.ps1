$ErrorActionPreference = "Stop"
git submodule update --init --recursive
if (-not (Test-Path .\vcpkg)) { git clone https://github.com/microsoft/vcpkg.git }
if (Test-Path .\vcpkg\bootstrap-vcpkg.bat) { .\vcpkg\bootstrap-vcpkg.bat }
[Environment]::SetEnvironmentVariable('VCPKG_DEFAULT_TRIPLET','x64-windows','User')
[Environment]::SetEnvironmentVariable('VCPKG_FEATURE_FLAGS','manifests,versions','User')
python -m venv .venv
./.venv/Scripts/pip install -U pip
./.venv/Scripts/pip install -r tools/python_editor/requirements.txt
Write-Host "Done. Configure with: cmake -S . -B build -DCMAKE_TOOLCHAIN_FILE=$((Get-Item .\vcpkg\scripts\buildsystems\vcpkg.cmake).FullName)" -ForegroundColor Green

