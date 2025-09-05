from PySide6 import QtWidgets, QtCore


class CodeViewDock(QtWidgets.QDockWidget):
    def __init__(self, title, parent=None):
        super().__init__(title, parent)
        self.setAllowedAreas(QtCore.Qt.BottomDockWidgetArea | QtCore.Qt.TopDockWidgetArea)
        # Fallback: simple QTextEdit (Monaco via WebEngine could be added later)
        self.editor = QtWidgets.QPlainTextEdit()
        self.editor.setPlainText('#include <cstdio>\n\nint main(){\n    printf("Hello AARTZE\\n");\n    return 0;\n}\n')
        self.setWidget(self.editor)
