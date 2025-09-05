"""Inspector panel for AARTZE editor.

Contains Transform groups (Location/Rotation/Scale) and emits change signals
when the user edits values. External code can call set_from_engine to update
the UI from engine state without feedback loops.
"""
from __future__ import annotations
from PySide6 import QtWidgets, QtCore


class InspectorWidget(QtWidgets.QFrame):
    locationChanged = QtCore.Signal(float, float, float)
    rotationChanged = QtCore.Signal(float, float, float)
    scaleChanged    = QtCore.Signal(float, float, float)

    def __init__(self, parent: QtWidgets.QWidget | None = None) -> None:
        super().__init__(parent)
        self.setObjectName("RightPanel")
        v = QtWidgets.QVBoxLayout(self); v.setContentsMargins(8,8,8,8); v.setSpacing(8)
        v.addWidget(QtWidgets.QLabel("Details", objectName="PanelTitle"))

        self._loc = [QtWidgets.QDoubleSpinBox() for _ in range(3)]
        self._rot = [QtWidgets.QDoubleSpinBox() for _ in range(3)]
        self._scl = [QtWidgets.QDoubleSpinBox() for _ in range(3)]
        for s in self._loc + self._rot + self._scl:
            s.setDecimals(3); s.setRange(-1e6, 1e6)
        for s in self._scl: s.setValue(1.0)

        def group(title: str, spins: list[QtWidgets.QDoubleSpinBox]):
            gb = QtWidgets.QGroupBox(title); gb.setObjectName("PanelGroup"); g = QtWidgets.QGridLayout(gb)
            for i,cap in enumerate(["X","Y","Z"]):
                g.addWidget(QtWidgets.QLabel(cap), 0, 1+i); g.addWidget(spins[i], 1, 1+i)
            return gb
        v.addWidget(group("Location", self._loc))
        v.addWidget(group("Rotation", self._rot))
        v.addWidget(group("Scale",    self._scl))
        v.addStretch()

        self._updating = False
        def emit_loc():
            if self._updating: return
            self.locationChanged.emit(*(s.value() for s in self._loc))
        for s in self._loc: s.valueChanged.connect(emit_loc)

    @QtCore.Slot(tuple, tuple, tuple)
    def set_from_engine(self, pos: tuple, rot: tuple, scl: tuple) -> None:
        self._updating = True
        for i,v in enumerate(pos): self._loc[i].setValue(float(v))
        for i,v in enumerate(rot): self._rot[i].setValue(float(v))
        for i,v in enumerate(scl): self._scl[i].setValue(float(v))
        self._updating = False

