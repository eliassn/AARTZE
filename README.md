# AARTZE
# AARTZE
## Quick start

Use the setup script for your OS. It bootstraps vcpkg and the Python editor deps.

Windows (PowerShell):

```
scripts/setup_env.ps1
```

Linux/macOS:

```
bash scripts/setup_env.sh
```

Configure & build (Visual Studio):

```
cmake -S . -B build -G "Visual Studio 17 2022" -DCMAKE_TOOLCHAIN_FILE=%CD%\vcpkg\scripts\buildsystems\vcpkg.cmake -DAARTZE_BUILD_EDITOR=ON
cmake --build build --config RelWithDebInfo
```

Configure & build (Linux/macOS):

```
cmake -S . -B build -DCMAKE_TOOLCHAIN_FILE=$(pwd)/vcpkg/scripts/buildsystems/vcpkg.cmake -DAARTZE_BUILD_EDITOR=ON
cmake --build build --config RelWithDebInfo -j
```

Run the Python editor:

```
python -m aartze_editor
```
