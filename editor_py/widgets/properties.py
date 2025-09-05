from PySide6 import QtWidgets

try:
    import aartzepy as ae
except Exception:
    ae = None


class PropertiesWidget(QtWidgets.QWidget):
    def __init__(self, parent=None):
        super().__init__(parent)
        self.setObjectName("RightPanel")
        lay = QtWidgets.QVBoxLayout(self); lay.setContentsMargins(6,6,6,6); lay.setSpacing(6)

        # Import
        grp_imp = QtWidgets.QGroupBox("Import"); grp_imp.setObjectName("PanelGroup")
        v = QtWidgets.QVBoxLayout(grp_imp)
        self.path_edit = QtWidgets.QLineEdit()
        btn_browse = QtWidgets.QPushButton("Choose File…")
        btn_browse.clicked.connect(self.on_browse)
        v.addWidget(self.path_edit)
        v.addWidget(btn_browse)
        lay.addWidget(grp_imp)

        # Transform
        grp_tr = QtWidgets.QGroupBox("Transform"); grp_tr.setObjectName("PanelGroup")
        g = QtWidgets.QGridLayout(grp_tr)
        self.pos = [QtWidgets.QDoubleSpinBox() for _ in range(3)]
        self.rot = [QtWidgets.QDoubleSpinBox() for _ in range(3)]
        self.scl = [QtWidgets.QDoubleSpinBox() for _ in range(3)]
        for ds in self.pos + self.rot + self.scl:
            ds.setRange(-1e6, 1e6); ds.setDecimals(3); ds.setSingleStep(0.1)
        for i, lbl in enumerate(["X","Y","Z"]):
            g.addWidget(QtWidgets.QLabel(lbl), 0, 1+i); g.addWidget(self.pos[i], 1, 1+i)
            g.addWidget(QtWidgets.QLabel(lbl), 2, 1+i); g.addWidget(self.rot[i], 3, 1+i)
            g.addWidget(QtWidgets.QLabel(lbl), 4, 1+i); g.addWidget(self.scl[i], 5, 1+i)
        g.addWidget(QtWidgets.QLabel("Location"), 1, 0)
        g.addWidget(QtWidgets.QLabel("Rotation"), 3, 0)
        g.addWidget(QtWidgets.QLabel("Scale"),    5, 0)
        lay.addWidget(grp_tr)

        # Bind changes to aartzepy
        for ds in self.pos + self.rot + self.scl:
            ds.valueChanged.connect(self.apply_transform)

    def on_browse(self):
        path, _ = QtWidgets.QFileDialog.getOpenFileName(self, "Import Model", "", "Models (*.obj *.fbx *.gltf *.glb *.blend *.usd *.usdz);;All files (*.*)")
        if path:
            self.path_edit.setText(path)
            if ae:
                ok = ae.import_file(path)
                if not ok:
                    QtWidgets.QMessageBox.warning(self, "Import", "Import failed")

    def apply_transform(self):
        if not ae:
            return
        from math import isnan
        def v(vals):
            return ae.Vec3(float(vals[0].value()), float(vals[1].value()), float(vals[2].value()))
        ae.set_transform(v(self.pos), v(self.rot), v(self.scl))
