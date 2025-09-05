import sys, importlib, traceback, pathlib
from PySide6 import QtWidgets, QtCore
from watchdog.observers import Observer
from watchdog.events import FileSystemEventHandler

APP_MOD = "aartze_editor.app"
PKG_ROOT = pathlib.Path(__file__).resolve().parents[1]


class Reloader(FileSystemEventHandler):
    def __init__(self, restart_cb, *globs):
        super().__init__()
        self.restart_cb = restart_cb
        self.globs = globs
        self._pending = False

    def on_any_event(self, _):
        if self._pending:
            return
        self._pending = True
        QtCore.QTimer.singleShot(150, self._emit)

    def _emit(self):
        self._pending = False
        self.restart_cb()


def make_main_window():
    mod = importlib.import_module(APP_MOD)
    importlib.reload(mod)  # live reload the UI module
    return mod.build_window()  # module must expose build_window()


def main():
    sys.path.insert(0, str(PKG_ROOT))  # ensure `aartze_editor` is importable
    # Ensure OpenGL sharing and QtQuick API alignment before QApplication
    try:
        from PySide6.QtQuick import QQuickWindow, QSGRendererInterface
        from PySide6.QtGui import QSurfaceFormat
        QtCore.QCoreApplication.setAttribute(QtCore.Qt.AA_ShareOpenGLContexts, True)
        QQuickWindow.setGraphicsApi(QSGRendererInterface.OpenGL)
        fmt = QSurfaceFormat(); fmt.setRenderableType(QSurfaceFormat.OpenGL); QSurfaceFormat.setDefaultFormat(fmt)
    except Exception:
        pass
    app = QtWidgets.QApplication(sys.argv)

    win = make_main_window()
    win.show()

    def restart():
        nonlocal win
        try:
            new = make_main_window()
        except Exception:
            print("Reload failed:\n", traceback.format_exc())
            return
        # swap windows without killing the event loop
        new.setGeometry(win.geometry())
        new.show()
        win.close()
        win = new

    # watch .py, .qss
    observer = Observer()
    handler = Reloader(restart, "*.py", "*.qss")
    observer.schedule(handler, str(PKG_ROOT / "aartze_editor"), recursive=True)
    observer.start()
    try:
        sys.exit(app.exec())
    finally:
        observer.stop(); observer.join()


if __name__ == "__main__":
    main()
