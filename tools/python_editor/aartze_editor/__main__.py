from PySide6 import QtWidgets, QtCore
from .app import build_window

def main():
    # Align GL/QtQuick as in integrated runner
    try:
        from PySide6.QtQuick import QQuickWindow, QSGRendererInterface
        from PySide6.QtGui import QSurfaceFormat
        QtCore.QCoreApplication.setAttribute(QtCore.Qt.AA_ShareOpenGLContexts, True)
        QQuickWindow.setGraphicsApi(QSGRendererInterface.OpenGL)
        fmt = QSurfaceFormat(); fmt.setRenderableType(QSurfaceFormat.OpenGL); QSurfaceFormat.setDefaultFormat(fmt)
    except Exception:
        pass
    app = QtWidgets.QApplication([])
    w = build_window(); w.show()
    raise SystemExit(app.exec())

if __name__ == "__main__":
    main()

