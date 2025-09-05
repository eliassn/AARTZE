from PySide6 import QtWidgets

class OutlinerWidget(QtWidgets.QTreeWidget):
    def __init__(self, parent=None):
        super().__init__(parent)
        self.setObjectName("Tree")
        self.setHeaderHidden(True)
        self.populate()

    def populate(self):
        self.clear()
        world = QtWidgets.QTreeWidgetItem(["World"])
        for name in ["Camera","Cube","Light"]:
            child = QtWidgets.QTreeWidgetItem([name])
            world.addChild(child)
        self.addTopLevelItem(world)
        world.setExpanded(True)
