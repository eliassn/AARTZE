from PySide6 import QtCore, QtGui, QtWidgets
from widgets import ViewportWidget, OutlinerPanel, DetailsPanel, TimelineBar

APP_NAME = "AARTZE Python Editor"


class HeaderBar(QtWidgets.QToolBar):
    def __init__(self, parent=None):
        super().__init__(parent)
        self.setObjectName("HeaderBar")
        self.setMovable(False)
        self.setFloatable(False)
        self.setIconSize(QtCore.QSize(16, 16))
        self.setContentsMargins(0, 0, 0, 0)
        # Play controls
        self.playAct = self.addAction("Play")
        self.pauseAct = self.addAction("Pause")
        self.stopAct = self.addAction("Stop")
        self.addSeparator()
        # Object/Mode etc.
        self.objectMode = QtWidgets.QToolButton()
        self.objectMode.setObjectName("Pill")
        self.objectMode.setPopupMode(QtWidgets.QToolButton.InstantPopup)
        self.objectMode.setText("Object Mode")
        menu = QtWidgets.QMenu(self.objectMode)
        for m in [
            "Object Mode",
            "Edit Mode",
            "Sculpt Mode",
            "Vertex Paint",
            "Weight Paint",
            "Texture Paint",
        ]:
            menu.addAction(m)
        self.objectMode.setMenu(menu)
        self.addWidget(self.objectMode)

        for txt in ["Global", "Local", "Pivot ▾"]:
            b = QtWidgets.QToolButton()
            b.setObjectName("Pill")
            b.setText(txt)
            self.addWidget(b)

        self.addSeparator()
        for txt in ["Perspective ▾", "Shading ▾", "Overlays"]:
            b = QtWidgets.QToolButton()
            b.setObjectName("FlatBtn")
            b.setText(txt)
            self.addWidget(b)

        self.addStretch()

        self.platforms = QtWidgets.QToolButton()
        self.platforms.setObjectName("FlatBtn")
        self.platforms.setText("Platforms")
        self.addWidget(self.platforms)

    def addStretch(self):
        w = QtWidgets.QWidget()
        w.setSizePolicy(QtWidgets.QSizePolicy.Expanding, QtWidgets.QSizePolicy.Preferred)
        self.addWidget(w)


class TopMenu(QtWidgets.QFrame):
    modeChanged = QtCore.Signal(str)

    def __init__(self, parent=None):
        super().__init__(parent)
        self.setObjectName("TopMenu")
        lay = QtWidgets.QHBoxLayout(self)
        lay.setContentsMargins(8, 6, 8, 6)
        lay.setSpacing(8)

        left = QtWidgets.QHBoxLayout()
        left.setSpacing(10)
        badge = QtWidgets.QLabel()
        badge.setFixedSize(16, 16)
        badge.setObjectName("BrandDot")
        logo = QtWidgets.QLabel("AARTZE")
        logo.setObjectName("Brand")
        left.addWidget(badge)
        left.addWidget(logo)

        menus = QtWidgets.QHBoxLayout()
        menus.setSpacing(12)
        for m in ["File", "Edit", "Window", "Tools", "Build", "Select", "Actor", "Help"]:
            b = QtWidgets.QToolButton(text=m)
            b.setObjectName("MenuItem")
            b.setAutoRaise(True)
            menus.addWidget(b)
        left.addLayout(menus)

        right = QtWidgets.QHBoxLayout()
        right.setSpacing(8)
        state = QtWidgets.QLabel("learning")
        state.setObjectName("Dim")
        dot = QtWidgets.QLabel()
        dot.setFixedSize(8, 8)
        dot.setObjectName("GreenDot")
        right.addWidget(state)
        right.addWidget(dot)

        lay.addLayout(left)
        lay.addStretch()
        lay.addLayout(right)


class CenterPane(QtWidgets.QWidget):
    def __init__(self, parent=None):
        super().__init__(parent)
        root = QtWidgets.QVBoxLayout(self)
        root.setContentsMargins(8, 0, 8, 0)
        root.setSpacing(8)

        self.header = HeaderBar(self)
        root.addWidget(self.header)

        # Main 3-column area
        hsplit = QtWidgets.QSplitter(QtCore.Qt.Horizontal, self)
        hsplit.setObjectName("MainSplit")
        hsplit.setChildrenCollapsible(False)

        # Viewport (own widget -> no z-order fights)
        vpWrap = QtWidgets.QFrame()
        vpWrap.setObjectName("ViewportWrap")
        vplay = QtWidgets.QVBoxLayout(vpWrap)
        vplay.setContentsMargins(10, 10, 10, 10)
        self.viewport = ViewportWidget()
        vplay.addWidget(self.viewport)

        # Outliner + Details
        self.outliner = OutlinerPanel()
        self.details = DetailsPanel()

        hsplit.addWidget(vpWrap)
        hsplit.addWidget(self.outliner)
        hsplit.addWidget(self.details)
        hsplit.setStretchFactor(0, 8)
        hsplit.setStretchFactor(1, 2)
        hsplit.setStretchFactor(2, 2)

        root.addWidget(hsplit)

        # Timeline
        self.timeline = TimelineBar()
        root.addWidget(self.timeline)


class MainWindow(QtWidgets.QMainWindow):
    def __init__(self):
        super().__init__()
        self.setWindowTitle(APP_NAME)
        self.resize(1360, 800)
        self.setUnifiedTitleAndToolBarOnMac(True)

        # Top bar (like JSX)
        self.menuBarFrame = TopMenu()
        topContainer = QtWidgets.QWidget()
        topLayout = QtWidgets.QVBoxLayout(topContainer)
        topLayout.setContentsMargins(0, 0, 0, 0)
        topLayout.setSpacing(0)
        topLayout.addWidget(self.menuBarFrame)

        # Center
        self.center = CenterPane()
        central = QtWidgets.QWidget()
        cLay = QtWidgets.QVBoxLayout(central)
        cLay.setContentsMargins(0, 0, 0, 0)
        cLay.setSpacing(0)
        cLay.addWidget(topContainer)
        cLay.addWidget(self.center)
        self.setCentralWidget(central)

        # Status bar
        sb = self.statusBar()
        sb.setObjectName("StatusBar")
        sb.showMessage("All Saved   |   Revision Control")


def load_styles(app: QtWidgets.QApplication):
    import os
    qss_path = os.path.join(os.path.dirname(__file__), "aartze.qss")
    with open(qss_path, "r", encoding="utf-8") as f:
        app.setStyleSheet(f.read())


if __name__ == "__main__":
    import sys
    app = QtWidgets.QApplication(sys.argv)
    load_styles(app)
    w = MainWindow()
    w.show()
    sys.exit(app.exec())

