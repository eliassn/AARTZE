from PySide6 import QtCore, QtGui, QtWidgets
from PySide6.QtOpenGLWidgets import QOpenGLWidget


class ViewportWidget(QOpenGLWidget):
    """Dedicated GL viewport (no UI drawn on top)."""

    def initializeGL(self):
        self.gl = self.context().functions()
        self.gl.glClearColor(0.07, 0.07, 0.08, 1.0)

    def resizeGL(self, w, h):
        self.gl.glViewport(0, 0, w, h)

    def paintGL(self):
        self.gl.glClear(self.gl.GL_COLOR_BUFFER_BIT | self.gl.GL_DEPTH_BUFFER_BIT)
        # simple grid (placeholder, replace with engine-rendered grid)
        p = QtGui.QPainter(self)
        p.setRenderHint(QtGui.QPainter.Antialiasing, False)
        p.fillRect(self.rect().adjusted(8, 8, -8, -8), QtGui.QColor(40, 41, 48, 130))
        p.setPen(QtGui.QPen(QtGui.QColor(255, 255, 255, 25), 1))
        step = 24
        r = self.rect().adjusted(18, 18, -18, -18)
        for x in range(r.left(), r.right(), step):
            p.drawLine(x, r.top(), x, r.bottom())
        for y in range(r.top(), r.bottom(), step):
            p.drawLine(r.left(), y, r.right(), y)
        p.end()


class GroupBox(QtWidgets.QGroupBox):
    def __init__(self, title):
        super().__init__(title)
        self.setObjectName("PanelGroup")


def _xyz_row(default=0.0, is_scale=False):
    row = QtWidgets.QHBoxLayout()
    row.setSpacing(6)

    def spin(val):
        sb = QtWidgets.QDoubleSpinBox()
        sb.setDecimals(3)
        sb.setRange(-1e6, 1e6)
        sb.setValue(1.0 if is_scale else default)
        sb.setButtonSymbols(QtWidgets.QAbstractSpinBox.NoButtons)
        sb.setObjectName("Spin")
        sb.setMinimumWidth(96)
        return sb

    for axis in ("X", "Y", "Z"):
        cap = QtWidgets.QLabel(axis)
        cap.setObjectName("AxisCap")
        cap.setFixedWidth(16)
        row.addWidget(cap)
        row.addWidget(spin(default))
    row.addStretch()
    return row


class DetailsPanel(QtWidgets.QFrame):
    def __init__(self):
        super().__init__()
        self.setObjectName("RightPanel")
        lay = QtWidgets.QVBoxLayout(self)
        lay.setContentsMargins(8, 8, 8, 8)
        lay.setSpacing(8)

        title = QtWidgets.QLabel("Details")
        title.setObjectName("PanelTitle")
        lay.addWidget(title)

        # Import tabs (pills)
        pills = QtWidgets.QHBoxLayout()
        pills.setSpacing(6)
        self.stack = QtWidgets.QStackedWidget()
        btns = []
        for i, (label, accept) in enumerate(
            [
                ("Blend (.blend)", "*.blend"),
                ("FBX (.fbx)", "*.fbx"),
                ("glTF (.gltf/.glb)", "*.gltf *.glb"),
                ("OBJ (.obj)", "*.obj"),
                ("USD (.usd/.usdz)", "*.usd *.usdz"),
            ]
        ):
            b = QtWidgets.QToolButton(text=label)
            b.setObjectName("PillTab")
            b.setCheckable(True)
            b.setAutoExclusive(True)
            if i == 0:
                b.setChecked(True)
            b.clicked.connect(lambda _=False, idx=i: self.stack.setCurrentIndex(idx))
            pills.addWidget(b)
            btns.append(b)

            page = QtWidgets.QWidget()
            v = QtWidgets.QVBoxLayout(page)
            v.setContentsMargins(0, 6, 0, 0)
            v.setSpacing(6)
            v.addWidget(QtWidgets.QLabel(f"Import {label}"))
            path = QtWidgets.QLineEdit()
            path.setPlaceholderText("…")
            choose = QtWidgets.QPushButton("Choose File…")
            v.addWidget(path)
            v.addWidget(choose)
            v.addStretch()
            self.stack.addWidget(page)

        lay.addLayout(pills)
        lay.addWidget(self.stack)

        # Transform blocks
        gb_t = GroupBox("Transform")
        v = QtWidgets.QVBoxLayout(gb_t)
        v.setSpacing(8)
        v.addWidget(QtWidgets.QLabel("Location"))
        v.addLayout(_xyz_row(0.0))
        rot_header = QtWidgets.QHBoxLayout()
        rot_header.addWidget(QtWidgets.QLabel("Rotation"))
        mode = QtWidgets.QToolButton(text="XYZ Euler ▾")
        mode.setObjectName("MiniPill")
        rot_header.addStretch()
        rot_header.addWidget(mode)
        v.addLayout(rot_header)
        v.addLayout(_xyz_row(0.0))
        v.addWidget(QtWidgets.QLabel("Scale"))
        v.addLayout(_xyz_row(1.0, is_scale=True))
        lay.addWidget(gb_t)

        for extra in ["Delta Transform", "Relations", "Collections", "Instancing", "Motion Paths"]:
            gb = GroupBox(extra)
            gv = QtWidgets.QVBoxLayout(gb)
            gv.addWidget(QtWidgets.QLabel("(placeholder)"))
            lay.addWidget(gb)

        lay.addStretch()


class OutlinerPanel(QtWidgets.QFrame):
    def __init__(self):
        super().__init__()
        self.setObjectName("LeftPanel")
        v = QtWidgets.QVBoxLayout(self)
        v.setContentsMargins(8, 8, 8, 8)
        v.setSpacing(8)
        title = QtWidgets.QLabel("Outliner")
        title.setObjectName("PanelTitle")
        v.addWidget(title)

        self.tree = QtWidgets.QTreeWidget()
        self.tree.setHeaderHidden(True)
        self.tree.setObjectName("Tree")
        world = QtWidgets.QTreeWidgetItem(["World"])
        for name in ["Camera", "Cube", "Light"]:
            QtWidgets.QTreeWidgetItem(world, [name])
        self.tree.addTopLevelItem(world)
        world.setExpanded(True)
        v.addWidget(self.tree)


class TimelineBar(QtWidgets.QFrame):
    def __init__(self):
        super().__init__()
        self.setObjectName("Timeline")
        h = QtWidgets.QHBoxLayout(self)
        h.setContentsMargins(8, 8, 8, 8)
        h.setSpacing(8)
        for txt in ["■", "▶", "⏸"]:
            b = QtWidgets.QToolButton(text=txt)
            b.setObjectName("FlatBtn")
            h.addWidget(b)
        h.addSpacing(10)
        for lbl in ["Start", "End", "Frame"]:
            h.addWidget(QtWidgets.QLabel(lbl))
            sb = QtWidgets.QSpinBox()
            sb.setRange(0, 999999)
            sb.setValue(1 if lbl != "End" else 250)
            sb.setObjectName("Spin")
            h.addWidget(sb)
        h.addStretch()

