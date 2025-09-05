set -euo pipefail
git submodule update --init --recursive
if [ ! -d vcpkg ]; then git clone https://github.com/microsoft/vcpkg.git; fi
./vcpkg/bootstrap-vcpkg.sh
python3 -m venv .venv
. .venv/bin/activate
pip install -U pip
pip install -r tools/python_editor/requirements.txt
echo "Configure with: cmake -S . -B build -DCMAKE_TOOLCHAIN_FILE=$(pwd)/vcpkg/scripts/buildsystems/vcpkg.cmake"

