"""Outliner widget for AARTZE editor.

Lists the scene hierarchy and exposes selection signals. Icons are loaded from
Qt resources when available, with fallbacks to qtawesome.
"""
from __future__ import annotations
from PySide6 import QtWidgets, QtCore, QtGui
from . import icons


class OutlinerWidget(QtWidgets.QTreeWidget):
    selectionChangedName = QtCore.Signal(str)

    def __init__(self, parent: QtWidgets.QWidget | None = None) -> None:
        super().__init__(parent)
        self.setHeaderHidden(True)
        self.setColumnCount(3)
        self.setObjectName("Tree")
        self.setAlternatingRowColors(False)
        self.populate()
        self.currentItemChanged.connect(self._emit_sel)

    def _icon(self, name: str) -> QtGui.QIcon:
        # Prefer qrc path
        qrc = QtGui.QIcon(f":/icons/{name}.svg")
        if not qrc.isNull():
            return qrc
        # Fallback to qtawesome
        ico = icons.get(name)
        if ico is not None:
            return ico
        return QtGui.QIcon()

    def populate(self) -> None:
        self.clear()
        world = QtWidgets.QTreeWidgetItem(self, ["World"])  # root
        for name, ico in [("Camera", "camera"), ("Cube", "mesh"), ("Light", "light")]:
            it = QtWidgets.QTreeWidgetItem(world, [name])
            it.setIcon(0, self._icon(ico))
        world.setExpanded(True)

    def _emit_sel(self, cur: QtWidgets.QTreeWidgetItem | None, _prev=None) -> None:
        if cur is None:
            return
        self.selectionChangedName.emit(cur.text(0))

