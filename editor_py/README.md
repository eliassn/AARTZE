# AARTZE Python Editor (experimental)

Run:

- Install Python 3.10+ and pip packages:
  - `pip install PySide6 PySide6-QtWebEngine NodeGraphQt`
- Build the C++ binding (aartze-py):
  - `cmake --preset windows-vs`
  - `cmake --build build --config Release`
- Ensure the built module is importable:
  - Windows: copy `build/Release/aartzepy.pyd` next to `editor_py/main.py`, or add `build/Release` to `PYTHONPATH`.
- Start:
  - `python main.py`

This Qt editor uses the `aartze` C++ runtime via `aartzepy` for viewport draw hooks, transform, import, and simple logging.
