"""
Thin wrapper package that re-exports the editor found under
tools/python_editor/aartze_editor so both layouts work:

  python -m aartze_editor

This keeps IDE and CMake integration flexible.
"""
from importlib import import_module as _imp

_real = _imp('tools.python_editor.aartze_editor')

# Re-export top-level symbols used by __main__ and external scripts
build_window = getattr(_real, 'build_window', None)

