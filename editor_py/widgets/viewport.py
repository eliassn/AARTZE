from PySide6 import QtWidgets, QtCore
from PySide6.QtOpenGLWidgets import QOpenGLWidget

try:
    import aartzepy as ae
except Exception:
    ae = None


class ViewportWidget(QOpenGLWidget):
    def __init__(self, parent=None):
        super().__init__(parent)
        self.setMinimumSize(320, 240)
        self.setUpdateBehavior(QOpenGLWidget.UpdateBehavior.PartialUpdate)

    def initializeGL(self):
        if ae:
            ae.init()

    def resizeGL(self, w, h):
        if ae:
            ae.resize(int(w), int(h))

    def paintGL(self):
        if ae:
            ae.draw()
        # schedule next frame
        self.update()
