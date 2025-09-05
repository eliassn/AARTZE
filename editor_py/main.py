import os
import sys
from PySide6 import QtWidgets, QtCore

try:
    import aartzepy as ae
except Exception as e:
    ae = None
    print("Warning: aartzepy not found, running UI-only: ", e)

from widgets.viewport import ViewportWidget
from widgets.outliner import OutlinerWidget
from widgets.properties import PropertiesWidget
from widgets.nodegraph import NodeGraphDock
from widgets.codeview import CodeViewDock


class MainWindow(QtWidgets.QMainWindow):
    def __init__(self):
        super().__init__()
        self.setWindowTitle("AARTZE Python Editor")
        self.resize(1600, 900)

        if ae:
            ae.init()

        # central splitter: viewport | (outliner, details)
        central = QtWidgets.QSplitter()
        central.setOrientation(QtCore.Qt.Orientation.Horizontal)

        self.viewport = ViewportWidget(self)
        self.outliner = OutlinerWidget(self)
        self.props    = PropertiesWidget(self)

        right = QtWidgets.QSplitter()
        right.setOrientation(QtCore.Qt.Orientation.Vertical)
        right.addWidget(self.outliner)
        right.addWidget(self.props)
        right.setStretchFactor(0, 2)
        right.setStretchFactor(1, 3)

        central.addWidget(self.viewport)
        central.addWidget(right)
        central.setStretchFactor(0, 3)
        central.setStretchFactor(1, 2)
        self.setCentralWidget(central)

        # docks for Auto-mode and Code-mode
        self.node_dock = NodeGraphDock("Auto-mode Graph", self)
        self.addDockWidget(QtCore.Qt.DockWidgetArea.BottomDockWidgetArea, self.node_dock)
        self.node_dock.hide()

        self.code_dock = CodeViewDock("C++ Code", self)
        self.addDockWidget(QtCore.Qt.DockWidgetArea.BottomDockWidgetArea, self.code_dock)
        self.code_dock.hide()

        # simple toolbar
        tb = self.addToolBar("Main")
        # Accent primary play button
        play_btn = QtWidgets.QToolButton()
        play_btn.setText("Play")
        play_btn.setProperty("accent", True)
        if ae:
            play_btn.clicked.connect(ae.begin_play)
        tb.addWidget(play_btn)

        pause_btn = QtWidgets.QToolButton(); pause_btn.setText("Pause")
        stop_btn  = QtWidgets.QToolButton();  stop_btn.setText("Stop")
        tb.addWidget(pause_btn)
        tb.addWidget(stop_btn)

        mode_combo = QtWidgets.QComboBox()
        mode_combo.addItems(["Editor","Auto-mode","C++-mode"]) 
        mode_combo.currentIndexChanged.connect(self.on_mode_changed)
        tb.addWidget(mode_combo)

    def on_mode_changed(self, idx: int):
        self.node_dock.setVisible(idx==1)
        self.code_dock.setVisible(idx==2)


def _apply_theme():
    base = os.path.dirname(__file__)
    # Prefer new aartze.qss if present; fallback to theme.qss
    for name in ("aartze.qss", "theme.qss"):
        qss_path = os.path.join(base, name)
        if QtCore.QFile.exists(qss_path):
            qfile = QtCore.QFile(qss_path)
            if qfile.open(QtCore.QIODevice.ReadOnly):
                QtWidgets.QApplication.instance().setStyleSheet(bytes(qfile.readAll()).decode())
                return
    print("No theme QSS found in", base)


if __name__ == "__main__":
    # Prefer the new JSX-matched UI if available (tools/python_editor)
    root = os.path.dirname(os.path.dirname(__file__))
    pkg_path = os.path.join(root, "tools", "python_editor")
    if os.path.isdir(pkg_path) and pkg_path not in sys.path:
        sys.path.insert(0, pkg_path)
    # Set GL sharing and force QtQuick to use OpenGL before QApplication
    try:
        from PySide6 import QtCore
        from PySide6.QtQuick import QQuickWindow, QSGRendererInterface
        from PySide6.QtGui import QSurfaceFormat
        QtCore.QCoreApplication.setAttribute(QtCore.Qt.AA_ShareOpenGLContexts, True)
        QQuickWindow.setGraphicsApi(QSGRendererInterface.OpenGL)
        fmt = QSurfaceFormat(); fmt.setRenderableType(QSurfaceFormat.OpenGL); QSurfaceFormat.setDefaultFormat(fmt)
    except Exception:
        pass

    # If the modern editor package imports, always use it; do NOT silently fallback.
    try:
        from aartze_editor.app import build_window  # type: ignore
        print("[AARTZE] Launching new aartze_editor UI")
        app = QtWidgets.QApplication(sys.argv)
        win = build_window()
        win.show()
        sys.exit(app.exec())
    except ModuleNotFoundError:
        # Only fallback when the package truly isn't present
        print("[AARTZE] aartze_editor not found; launching legacy editor_py UI")
        app = QtWidgets.QApplication(sys.argv)
        QtWidgets.QApplication.setStyle("Fusion")
        _apply_theme()
        w = MainWindow()
        w.show()
        sys.exit(app.exec())
    except Exception as e:
        # Surface the error instead of masking it with the legacy UI
        import traceback
        traceback.print_exc()
        raise
