from PySide6 import QtWidgets, QtCore

try:
    from NodeGraphQt import NodeGraph
except Exception:
    NodeGraph = None

try:
    import aartzepy as ae
except Exception:
    ae = None


class NodeGraphDock(QtWidgets.QDockWidget):
    def __init__(self, title, parent=None):
        super().__init__(title, parent)
        self.setAllowedAreas(QtCore.Qt.BottomDockWidgetArea | QtCore.Qt.TopDockWidgetArea)
        if NodeGraph is None:
            self.setWidget(QtWidgets.QLabel("Install NodeGraphQt to enable Auto-mode"))
            return
        self.graph = NodeGraph()
        self.graph_widget = self.graph.widget
        self.setWidget(self.graph_widget)
        self._build()

    def _build(self):
        g = self.graph
        g.clear_session()
        # Add two simple nodes using built-in raw API
        n1 = g.create_node('nodes.BaseNode', name='Event Begin Play', color='#B58900')
        n2 = g.create_node('nodes.BaseNode', name='Print String')
        n1.set_pos(50, 50)
        n2.set_pos(300, 50)
        try:
            g.connect_ports(n1['out'], n2['in'])
        except Exception:
            pass
