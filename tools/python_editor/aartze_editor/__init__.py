"""
Bootstraps the Python editor package so it can import the compiled engine
module (aartze/pybind) without manual PYTHONPATH hacks.

It searches common build output folders for a matching aartzepy*.pyd for the
current Python ABI (e.g., cp312 or cp313) and prepends that directory to
sys.path. On Windows, it also exposes that directory to the DLL loader so that
dependent runtime DLLs next to the module can be found.
"""

from __future__ import annotations

import os
import sys
from pathlib import Path


def _python_abi_tag() -> str:
    # e.g. cp312, cp313
    return f"cp{sys.version_info.major}{sys.version_info.minor}"


def _maybe_add_path(p: Path) -> None:
    sp = str(p)
    if sp not in sys.path:
        sys.path.insert(0, sp)
    # Help Windows resolve dependent DLLs sitting next to the module
    try:
        if hasattr(os, "add_dll_directory") and p.is_dir():
            os.add_dll_directory(sp)  # type: ignore[attr-defined]
    except Exception:
        pass


def _find_engine_dir() -> Path | None:
    here = Path(__file__).resolve()
    # repo root: tools/python_editor/aartze_editor/__init__.py -> parents[3]
    try:
        root = here.parents[3]
    except IndexError:
        root = here.parent

    abi = _python_abi_tag()

    # Allow override via env var
    env_dir = os.getenv("AARTZE_PY_DIR")
    if env_dir:
        d = Path(env_dir)
        # If it directly points to the module, return the folder
        if d.suffix.lower() == ".pyd":
            return d.parent
        if d.is_dir():
            return d

    # Common build output locations
    # Start with known folders, then discover any custom build* dirs
    candidates = [
        root / "build" / "Release",
        root / "build" / "RelWithDebInfo",
        root / "build" / "Debug",
        root / "build",
        # Legacy location where some CMake scripts copy the .pyd
        root / "editor_py",
    ]
    try:
        for child in root.iterdir():
            name = child.name.lower()
            if child.is_dir() and name.startswith("build") and name not in {"build", "editor_py"}:
                candidates.extend([child / "Release", child / "RelWithDebInfo", child / "Debug", child])
    except Exception:
        pass

    for d in candidates:
        try:
            if not d.is_dir():
                continue
            hits = list(d.glob(f"aartzepy.*{abi}*.pyd"))
            if hits:
                return d
        except Exception:
            continue
    return None


def _bootstrap_engine_path() -> None:
    d = _find_engine_dir()
    if not d:
        # Still make vcpkg runtime DLLs discoverable if present; harmless otherwise
        try:
            here = Path(__file__).resolve()
            root = here.parents[3]
            vcpkg_bin = root / "vcpkg_installed" / "x64-windows" / "bin"
            if vcpkg_bin.is_dir():
                _maybe_add_path(vcpkg_bin)
        except Exception:
            pass
        return
    _maybe_add_path(d)


# Do it at import time so submodules (e.g., viewport) can import the engine
_bootstrap_engine_path()
