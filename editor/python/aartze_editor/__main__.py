"""aartze_editor entry point wrapper.

Allows launching the editor with:
  python -m aartze_editor

It forwards to tools.python_editor.aartze_editor.build_window().
"""
from PySide6 import QtWidgets
from . import build_window


def main() -> int:
    app = QtWidgets.QApplication.instance() or QtWidgets.QApplication([])
    win = build_window()
    win.show()
    return app.exec()


if __name__ == "__main__":
    raise SystemExit(main())

