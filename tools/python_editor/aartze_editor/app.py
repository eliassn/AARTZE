from PySide6 import QtWidgets, QtGui, QtCore
from .viewport import GLViewport
from . import icons


def _pill(btn: QtWidgets.QPushButton):
    btn.setCursor(QtCore.Qt.PointingHandCursor)
    btn.setProperty("pill", True)
    return btn


class BrandBar(QtWidgets.QFrame):
    def __init__(self):
        super().__init__()
        self.setObjectName("TopMenu")
        h = QtWidgets.QHBoxLayout(self)
        h.setContentsMargins(8, 6, 8, 6)
        h.setSpacing(8)
        # left
        left = QtWidgets.QHBoxLayout(); left.setSpacing(10)
        dot = QtWidgets.QLabel(); dot.setFixedSize(16, 16); dot.setObjectName("BrandDot"); left.addWidget(dot)
        brand = QtWidgets.QLabel("AARTZE"); brand.setObjectName("Brand"); left.addWidget(brand)
        menus = QtWidgets.QHBoxLayout(); menus.setSpacing(12)
        for m in ["File","Edit","Window","Tools","Build","Select","Actor","Help"]:
            b = QtWidgets.QToolButton(text=m); b.setObjectName("MenuItem"); b.setAutoRaise(True); menus.addWidget(b)
        left.addLayout(menus)
        h.addLayout(left)
        h.addStretch()
        # right
        right = QtWidgets.QHBoxLayout(); right.setSpacing(6)
        state = QtWidgets.QLabel("learning"); state.setObjectName("Dim"); right.addWidget(state)
        gdot = QtWidgets.QLabel(); gdot.setFixedSize(8,8); gdot.setObjectName("GreenDot"); right.addWidget(gdot)
        h.addLayout(right)


class ModeBar(QtWidgets.QFrame):
    modeChanged = QtCore.Signal(str)
    workspaceChanged = QtCore.Signal(str)
    def __init__(self):
        super().__init__()
        self.setObjectName("ModeBar")
        h = QtWidgets.QHBoxLayout(self); h.setContentsMargins(8,4,8,4); h.setSpacing(8)

        # Workspace tabs (Blender-like)
        self.tabs = QtWidgets.QTabBar(objectName="WorkspaceTabs")
        self.tabs.setExpanding(False)
        self.tabs.setDrawBase(False)
        self.tabs.setMovable(False)
        ws_names = ["Layout","Modeling","Sculpting","UV Editing","Texture Paint","Shading","Animation","Rendering","Compositing","Geometry Nodes","Scripting"]
        icon_map = {
            "Layout":"layout", "Modeling":"modeling", "Sculpting":"sculpting", "UV Editing":"uv",
            "Texture Paint":"texture", "Shading":"shading", "Animation":"animation", "Rendering":"rendering",
            "Compositing":"compositing", "Geometry Nodes":"geometry-nodes", "Scripting":"scripting"
        }
        for n in ws_names:
            idx = self.tabs.addTab(n)
            ico = icons.get(icon_map.get(n, ""))
            if ico is not None:
                self.tabs.setTabIcon(idx, ico)
        self.tabs.setCurrentIndex(0)
        self.tabs.currentChanged.connect(lambda i: self.workspaceChanged.emit(ws_names[i]))
        h.addWidget(self.tabs, 1)

        # Mode switcher (Editor / Auto-mode / C++-mode)
        self.modeBtn = QtWidgets.QToolButton(text="Editor"); self.modeBtn.setObjectName("Pill")
        self.modeBtn.setPopupMode(QtWidgets.QToolButton.InstantPopup)
        m = QtWidgets.QMenu(self.modeBtn)
        for key,label in [("editor","Editor"),("auto","Auto-mode"),("code","C++-mode")]:
            act = m.addAction(label)
            act.triggered.connect(lambda _=False, k=key, l=label: (self.modeBtn.setText(l), self.modeChanged.emit(k)))
        self.modeBtn.setMenu(m)
        h.addWidget(self.modeBtn)


class GridBack(QtWidgets.QWidget):
    def paintEvent(self, e):
        p = QtGui.QPainter(self)
        p.fillRect(self.rect(), QtGui.QColor(18,19,23))
        p.setPen(QtGui.QPen(QtGui.QColor(255,255,255,25), 1))
        step = 24
        r = self.rect()
        for x in range(r.left(), r.right(), step): p.drawLine(x, r.top(), x, r.bottom())
        for y in range(r.top(), r.bottom(), step): p.drawLine(r.left(), y, r.right(), y)
        p.end()


def _outliner_panel():
    wrap = QtWidgets.QFrame(); wrap.setObjectName("LeftPanel")
    v = QtWidgets.QVBoxLayout(wrap); v.setContentsMargins(8,8,8,8); v.setSpacing(0)
    head = QtWidgets.QHBoxLayout(); head.setSpacing(6)
    title = QtWidgets.QLabel("Outliner"); title.setObjectName("PanelTitle"); head.addWidget(title); head.addStretch()
    sb = QtWidgets.QToolButton(); icons.set_icon(sb, "search", color="#9aa3b2"); head.addWidget(sb)
    v.addLayout(head)
    tree = QtWidgets.QTreeWidget(); tree.setObjectName("Tree"); tree.setHeaderHidden(True); tree.setColumnCount(3)
    tree.setColumnWidth(0, 160); tree.setColumnWidth(1, 24); tree.setColumnWidth(2, 24)

    # Try to fetch entity ids from engine
    try:
        import aartze as _eng
    except Exception:
        try:
            import aartzepy as _eng
        except Exception:
            _eng = None

    def eng_id(name: str) -> int:
        try:
            if _eng and hasattr(_eng, 'get_entity_id'):
                return int(_eng.get_entity_id(name))
        except Exception:
            pass
        return -1

    def make_item(name: str, parent=None):
        it = QtWidgets.QTreeWidgetItem(parent if parent else tree, [name, "", ""])
        it.setData(0, QtCore.Qt.UserRole+0, eng_id(name))
        it.setData(0, QtCore.Qt.UserRole+1, True)   # visible
        it.setData(0, QtCore.Qt.UserRole+2, False)  # locked
        # Assign left icon using bundled Lucide SVGs
        icon_key = {"World":"world", "Camera":"camera", "Cube":"mesh", "Light":"light"}.get(name)
        if icon_key:
            try:
                ico = icons.get(icon_key)
                if ico is not None:
                    it.setIcon(0, ico)
            except Exception:
                pass
        update_icons(it)
        return it

    def update_icons(it: QtWidgets.QTreeWidgetItem):
        vis = bool(it.data(0, QtCore.Qt.UserRole+1))
        lok = bool(it.data(0, QtCore.Qt.UserRole+2))
        eye_btn = QtWidgets.QToolButton(); eye_btn.setToolTip("Visible" if vis else "Hidden")
        icons.set_icon(eye_btn, "eye" if vis else "eye-slash")
        lock_btn = QtWidgets.QToolButton(); lock_btn.setToolTip("Locked" if lok else "Unlocked")
        icons.set_icon(lock_btn, "lock" if lok else "unlock")
        tree.setItemWidget(it, 1, eye_btn)
        tree.setItemWidget(it, 2, lock_btn)
        def toggle_vis():
            nid = int(it.data(0, QtCore.Qt.UserRole+0) or -1)
            newv = not vis
            it.setData(0, QtCore.Qt.UserRole+1, newv)
            if _eng and hasattr(_eng, 'set_visible') and nid >= 0:
                try: _eng.set_visible(nid, bool(newv))
                except Exception: pass
            update_icons(it)
        def toggle_lock():
            nid = int(it.data(0, QtCore.Qt.UserRole+0) or -1)
            newv = not lok
            it.setData(0, QtCore.Qt.UserRole+2, newv)
            if _eng and hasattr(_eng, 'set_locked') and nid >= 0:
                try: _eng.set_locked(nid, bool(newv))
                except Exception: pass
            update_icons(it)
        eye_btn.clicked.connect(toggle_vis)
        lock_btn.clicked.connect(toggle_lock)

    # Populate
    world = make_item("World")
    for n in ("Camera","Cube","Light"):
        make_item(n, world)
    world.setExpanded(True)
    v.addWidget(tree)
    # expose tree for selection wiring
    wrap.tree = tree  # type: ignore[attr-defined]
    return wrap


class PropsPanel(QtWidgets.QFrame):
    locationChanged = QtCore.Signal(float, float, float)
    rotationChanged = QtCore.Signal(float, float, float)
    scaleChanged    = QtCore.Signal(float, float, float)

    def __init__(self):
        super().__init__()
        self.setObjectName("RightPanel")
        v = QtWidgets.QVBoxLayout(self); v.setContentsMargins(8,8,8,8); v.setSpacing(8)
        title = QtWidgets.QLabel("Details"); title.setObjectName("PanelTitle"); v.addWidget(title)

        # Import tabs header (UI only for now)
        pills = QtWidgets.QHBoxLayout(); pills.setSpacing(6)
        stack = QtWidgets.QStackedWidget()
        tabs = [("Blend (.blend)", "*.blend"),("FBX (.fbx)","*.fbx"),("glTF (.gltf/.glb)","*.gltf *.glb"),("OBJ (.obj)","*.obj"),("USD (.usd/.usdz)","*.usd *.usdz")]
        for i,(label,acc) in enumerate(tabs):
            b = QtWidgets.QToolButton(text=label); b.setObjectName("PillTab"); b.setCheckable(True); b.setAutoExclusive(True)
            if i==0: b.setChecked(True)
            b.clicked.connect(lambda _=False, idx=i: stack.setCurrentIndex(idx))
            pills.addWidget(b)
            page = QtWidgets.QWidget(); pv = QtWidgets.QVBoxLayout(page); pv.setContentsMargins(0,6,0,0); pv.setSpacing(6)
            pv.addWidget(QtWidgets.QLabel(f"Import {label}"))
            path = QtWidgets.QLineEdit(); path.setPlaceholderText("ÃƒÂ¢Ã¢â€šÂ¬Ã‚Â¦"); path.setObjectName("ImportPath")
            choose = QtWidgets.QPushButton("Choose FileÃƒÂ¢Ã¢â€šÂ¬Ã‚Â¦"); choose.setObjectName("ChooseBtn")
            pv.addWidget(path); pv.addWidget(choose); pv.addStretch(); stack.addWidget(page)
        v.addLayout(pills); v.addWidget(stack)

        # Transform groups with live bindings
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

        # Wire signals (avoid feedback loops with a guard)
        self._updating = False
        def emit_loc():
            if self._updating: return
            self.locationChanged.emit(*(s.value() for s in self._loc))
        for s in self._loc:
            s.valueChanged.connect(emit_loc)

    @QtCore.Slot(tuple, tuple, tuple)
    def setFromViewport(self, pos: tuple, rot: tuple, scl: tuple):
        self._updating = True
        for i,v in enumerate(pos): self._loc[i].setValue(float(v))
        for i,v in enumerate(rot): self._rot[i].setValue(float(v))
        for i,v in enumerate(scl): self._scl[i].setValue(float(v))
        self._updating = False


def _editor_toolbar():
    bar = QtWidgets.QToolBar(); bar.setObjectName("HeaderBar"); bar.setMovable(False)
    p = _pill(QtWidgets.QPushButton("")); p.setToolTip("Play (Begin Play)"); icons.set_icon(p, "play", color="#f59e0b"); bar.addWidget(p)
    q = _pill(QtWidgets.QPushButton("")); q.setToolTip("Pause"); icons.set_icon(q, "pause", color="#cbd5e1"); bar.addWidget(q)
    s = _pill(QtWidgets.QPushButton("")); s.setToolTip("Stop"); icons.set_icon(s, "stop", color="#cbd5e1"); bar.addWidget(s)
    bar.addSeparator()
    # Mode dropdown
    obj = QtWidgets.QToolButton(text="Object Mode"); obj.setToolTip("Selection Mode"); obj.setObjectName("Pill"); obj.setPopupMode(QtWidgets.QToolButton.InstantPopup)
    om = QtWidgets.QMenu(obj)
    for m in ["Object Mode","Edit Mode","Sculpt Mode","Vertex Paint","Weight Paint","Texture Paint"]:
        om.addAction(m)
    obj.setMenu(om); bar.addWidget(obj)
    # Orientation / pivot
    for txt in ["Global","Local","Pivot ÃƒÂ¢Ã¢â‚¬â€œÃ‚Â¾"]:
        b = QtWidgets.QToolButton(text=txt); b.setToolTip(txt); bar.addWidget(b)
    # Snapping
    mag = QtWidgets.QToolButton(); mag.setToolTip("Snapping"); icons.set_icon(mag, "magnet", color="#93c5fd"); bar.addWidget(mag)
    inc = QtWidgets.QToolButton(text="Increment ÃƒÂ¢Ã¢â‚¬â€œÃ‚Â¾"); inc.setToolTip("Snap Increment"); bar.addWidget(inc)
    prop = QtWidgets.QToolButton(text="Proportional Editing"); prop.setToolTip("Proportional Editing"); bar.addWidget(prop)
    bar.addSeparator()
    vb = QtWidgets.QToolButton(text="Perspective ÃƒÂ¢Ã¢â‚¬â€œÃ‚Â¾"); vb.setToolTip("View"); bar.addWidget(vb)
    sh = QtWidgets.QToolButton(text="Shading ÃƒÂ¢Ã¢â‚¬â€œÃ‚Â¾"); sh.setToolTip("Shading"); bar.addWidget(sh)
    ov = QtWidgets.QToolButton(text="Overlays"); ov.setToolTip("Overlays"); bar.addWidget(ov)
    bar.addSeparator()
    plat = QtWidgets.QToolButton(text="Platforms"); plat.setToolTip("Platforms"); bar.addWidget(plat)
    return bar


class EditorMode(QtWidgets.QWidget):
    def __init__(self):
        super().__init__()
        root = QtWidgets.QVBoxLayout(self); root.setContentsMargins(8,0,8,0); root.setSpacing(8)
        # Top play/pause/stop removed; use bottom timeline controls only
        split = QtWidgets.QSplitter(QtCore.Qt.Horizontal)
        # viewport wrapper with rounded frame
        vpWrap = QtWidgets.QFrame(); vpWrap.setObjectName("ViewportWrap")
        # Overlay layout: viewport + tool shelf overlay
        grid = QtWidgets.QGridLayout(vpWrap); grid.setContentsMargins(0,0,0,0)
        grid.setSpacing(0)
        self.viewport = GLViewport();
        self.viewport.setSizePolicy(QtWidgets.QSizePolicy.Expanding, QtWidgets.QSizePolicy.Expanding)
        self.viewport.setMinimumSize(800, 480)
        grid.addWidget(self.viewport, 0, 0)
        shelf = QtWidgets.QFrame(); shelf.setAttribute(QtCore.Qt.WA_TransparentForMouseEvents, False)
        sv = QtWidgets.QVBoxLayout(shelf); sv.setContentsMargins(6,6,6,6); sv.setSpacing(6)
        tools = [
            ("select", "stop", "Select" , "#e5e7eb"),
            ("cursor", "crosshair", "Cursor", "#22d3ee"),
            ("move",   "move", "Move (G)", "#22c55e"),
            ("rotate", "rotate", "Rotate (R)", "#f59e0b"),
            ("scale",  "scale", "Scale (S)", "#3b82f6"),
            ("brush",  "brush", "Annotate", "#a78bfa"),
            ("measure","ruler", "Measure", "#94a3b8"),
            ("layers", "layers","Collections", "#e5e7eb"),
            ("camera", "camera","Camera", "#eab308"),
            ("sun",    "sun",   "Light", "#fbbf24"),
            ("stack",  "layers","Layers", "#cbd5e1"),
        ]
        for _, key, tip, col in tools:
            b = QtWidgets.QToolButton(); b.setFixedSize(28,28); b.setToolTip(tip); icons.set_icon(b, key, color=col); sv.addWidget(b)
        sv.addStretch()
        # place shelf over viewport, left centered
        grid.addWidget(shelf, 0, 0, QtCore.Qt.AlignLeft | QtCore.Qt.AlignVCenter)
        self.shelf = shelf
        split.addWidget(vpWrap)
        outliner = _outliner_panel(); split.addWidget(outliner)
        self.outliner = outliner
        self.props = PropsPanel()
        split.addWidget(self.props)
        split.setStretchFactor(0,8); split.setStretchFactor(1,2); split.setStretchFactor(2,2)
        # Make the viewport dominate by default
        split.setSizes([1400, 340, 360])
        root.addWidget(split)
        # timeline
        root.addWidget(_timeline())

        # Bind viewport <-> properties
        self.viewport.transformChanged.connect(self.props.setFromViewport)
        self.props.locationChanged.connect(self.viewport.setLocation)
        # Selection from Outliner ÃƒÂ¢Ã¢â‚¬Â Ã¢â‚¬â„¢ viewport + props
        try:
            tree = outliner.tree  # type: ignore[attr-defined]
            def on_sel(cur, prev=None):
                name = cur.text(0) if cur else "Cube"
                self.viewport.setSelectedName(name)
            tree.currentItemChanged.connect(on_sel)
            # Default select Cube so gizmo/props hydrate
            it = tree.topLevelItem(0)
            if it and it.childCount() > 0:
                for i in range(it.childCount()):
                    if it.child(i).text(0) == 'Cube':
                        tree.setCurrentItem(it.child(i))
                        break
        except Exception:
            pass


class AutoMode(QtWidgets.QWidget):
    def __init__(self):
        super().__init__()
        grid = QtWidgets.QGridLayout(self); grid.setContentsMargins(8,8,8,8); grid.setSpacing(8)
        # Palette
        left = QtWidgets.QFrame(); left.setObjectName("LeftPanel")
        lv = QtWidgets.QVBoxLayout(left); lv.setContentsMargins(8,8,8,8); lv.setSpacing(6)
        lv.addWidget(QtWidgets.QLabel("Palette", objectName="PanelTitle"))
        sa = QtWidgets.QScrollArea(); sa.setWidgetResizable(True)
        cont = QtWidgets.QWidget(); cv = QtWidgets.QVBoxLayout(cont)
        for cat in ["Events","Input","Actors","Math","Utilities","Rendering","Audio","Particles","Physics","AI","Flow","Custom","Plugins","Scripting","Animation","Cinematics"]:
            gb = QtWidgets.QGroupBox(cat); inner = QtWidgets.QVBoxLayout(gb)
            inner.addWidget(QtWidgets.QLabel(f"Add {cat} Node"))
            inner.addWidget(QtWidgets.QLabel(f"Sample {cat} Node"))
            cv.addWidget(gb)
        cv.addStretch(); sa.setWidget(cont)
        lv.addWidget(sa)

        # Graph
        center = QtWidgets.QFrame(); center.setObjectName("ViewportWrap")
        ctv = QtWidgets.QVBoxLayout(center); ctv.setContentsMargins(10,10,10,10)
        top = QtWidgets.QHBoxLayout(); top.addWidget(QtWidgets.QLabel("Auto-mode Graph", objectName="PanelTitle")); top.addStretch()
        run = QtWidgets.QToolButton(text="Run"); icons.set_icon(run, "play-circle"); top.addWidget(run)
        dbg = QtWidgets.QToolButton(text="Debug"); icons.set_icon(dbg, "bug"); top.addWidget(dbg)
        ctv.addLayout(top)
        ctv.addWidget(GridBack())

        # Details
        right = QtWidgets.QFrame(); right.setObjectName("RightPanel")
        rv = QtWidgets.QVBoxLayout(right); rv.setContentsMargins(8,8,8,8)
        rv.addWidget(QtWidgets.QLabel("Details", objectName="PanelTitle"))
        rv.addWidget(QtWidgets.QLabel("Node Properties (placeholder)"))
        rv.addWidget(QtWidgets.QLabel("Variables (placeholder)"))
        rv.addStretch()

        # Output bottom across all columns is omitted for brevity; use status bar
        grid.addWidget(left,   0,0)
        grid.addWidget(center, 0,1)
        grid.addWidget(right,  0,2)
        grid.setColumnStretch(0,3); grid.setColumnStretch(1,6); grid.setColumnStretch(2,3)


class ShadingMode(QtWidgets.QWidget):
    def __init__(self):
        super().__init__()
        grid = QtWidgets.QGridLayout(self); grid.setContentsMargins(8,8,8,8); grid.setSpacing(8)
        # Left: Browser + Image panel (placeholders)
        left = QtWidgets.QFrame(objectName="LeftPanel")
        lv = QtWidgets.QVBoxLayout(left); lv.setContentsMargins(8,8,8,8); lv.setSpacing(8)
        lv.addWidget(QtWidgets.QLabel("Browser", objectName="PanelTitle"))
        lv.addWidget(GridBack())
        lv.addWidget(QtWidgets.QLabel("Inspector", objectName="PanelTitle"))
        lv.addWidget(GridBack())

        # Center: Shader nodes
        center = QtWidgets.QFrame(objectName="ViewportWrap")
        cv = QtWidgets.QVBoxLayout(center); cv.setContentsMargins(10,10,10,10)
        top = QtWidgets.QHBoxLayout(); top.addWidget(QtWidgets.QLabel("Shader Editor", objectName="PanelTitle")); top.addStretch(); cv.addLayout(top)
        cv.addWidget(GridBack())

        # Right: Properties
        right = QtWidgets.QFrame(objectName="RightPanel")
        rv = QtWidgets.QVBoxLayout(right); rv.setContentsMargins(8,8,8,8)
        rv.addWidget(QtWidgets.QLabel("World", objectName="PanelTitle"))
        rv.addWidget(QtWidgets.QLabel("Use Nodes"))
        rv.addStretch()

        grid.addWidget(left, 0,0)
        grid.addWidget(center, 0,1)
        grid.addWidget(right, 0,2)
        grid.setColumnStretch(0,3); grid.setColumnStretch(1,6); grid.setColumnStretch(2,3)


class CodeMode(QtWidgets.QWidget):
    def __init__(self):
        super().__init__()
        grid = QtWidgets.QGridLayout(self); grid.setContentsMargins(8,8,8,8); grid.setSpacing(8)
        # Explorer
        left = QtWidgets.QFrame(); left.setObjectName("LeftPanel")
        lv = QtWidgets.QVBoxLayout(left); lv.setContentsMargins(8,8,8,8)
        head = QtWidgets.QHBoxLayout(); head.addWidget(QtWidgets.QLabel("Explorer", objectName="PanelTitle")); head.addStretch()
        n1 = QtWidgets.QToolButton(); icons.set_icon(n1, "file-plus"); head.addWidget(n1)
        n2 = QtWidgets.QToolButton(); icons.set_icon(n2, "folder-plus"); head.addWidget(n2)
        lv.addLayout(head)
        lv.addWidget(QtWidgets.QLabel("AARTZE"))
        for fn in ["main.cpp","engine.cpp","imgui_layer.cpp"]:
            lv.addWidget(QtWidgets.QPushButton(fn))

        # Editor
        center = QtWidgets.QFrame(); center.setObjectName("ViewportWrap")
        cv = QtWidgets.QVBoxLayout(center); cv.setContentsMargins(8,8,8,8)
        tabs = QtWidgets.QHBoxLayout();
        tabs.addWidget(QtWidgets.QLabel("main.cpp")); tabs.addWidget(QtWidgets.QLabel("engine.cpp")); tabs.addStretch()
        br = QtWidgets.QToolButton(text="Build & Run"); icons.set_icon(br, "play-circle"); tabs.addWidget(br)
        dbg = QtWidgets.QToolButton(text="Debug"); icons.set_icon(dbg, "bug"); tabs.addWidget(dbg)
        cv.addLayout(tabs)
        # Monaco via QtWebEngine (loads local HTML which pulls CDN)
        try:
            from PySide6 import QtWebEngineWidgets  # type: ignore
            view = QtWebEngineWidgets.QWebEngineView()
            import pathlib
            base_dir = pathlib.Path(__file__).resolve().parent
            html_path = base_dir / "monaco.html"
            thirdparty_local = base_dir.parent / "thirdparty" / "monaco" / "min"
            # Prefer offline monaco if present
            use_local = (thirdparty_local / "vs" / "loader.js").exists()
            try:
                src = html_path.read_text(encoding="utf-8")
            except Exception:
                src = "<html><body><pre>Monaco HTML missing</pre></body></html>"
            code = "#include <cstdio>\nint main(){\n    printf(\"Hello AARTZE\\n\");\n    return 0;\n}\n"
            src = src.replace("__INITIAL_CODE__", code)
            if use_local:
                base_url = QtCore.QUrl.fromLocalFile(str(thirdparty_local) + "/")
                src = src.replace("__MONACO_BASE__", base_url.toString())
                view.setHtml(src, baseUrl=base_url)
            else:
                cdn = "https://cdn.jsdelivr.net/npm/monaco-editor@0.51.0/min/"
                src = src.replace("__MONACO_BASE__", cdn)
                view.setHtml(src, baseUrl=QtCore.QUrl(cdn))
            cv.addWidget(view)
        except Exception:
            # Fallback to plain text editor if WebEngine not present
            edit = QtWidgets.QPlainTextEdit(); edit.setPlainText("#include <cstdio>\nint main(){printf(\"Hello AARTZE\\n\");return 0;}")
            cv.addWidget(edit)

        # Right
        right = QtWidgets.QFrame(); right.setObjectName("RightPanel")
        rv = QtWidgets.QVBoxLayout(right); rv.setContentsMargins(8,8,8,8)
        rv.addWidget(QtWidgets.QLabel("Problems / Outline", objectName="PanelTitle"))
        rv.addWidget(QtWidgets.QLabel("No problems detected."))
        rv.addWidget(QtWidgets.QLabel("Terminal", objectName="PanelTitle"))
        rv.addWidget(QtWidgets.QLabel("$ _"))
        rv.addStretch()

        grid.addWidget(left,   0,0)
        grid.addWidget(center, 0,1)
        grid.addWidget(right,  0,2)
        grid.setColumnStretch(0,2); grid.setColumnStretch(1,7); grid.setColumnStretch(2,3)


def _timeline():
    w = QtWidgets.QFrame(); w.setObjectName("Timeline")
    h = QtWidgets.QHBoxLayout(w); h.setContentsMargins(8,8,8,8)
    for name in ("stop","play","pause"):
        b = QtWidgets.QToolButton(); icons.set_icon(b, name); h.addWidget(b)
    h.addSpacing(10)
    h.addWidget(QtWidgets.QLabel("Start")); s = QtWidgets.QSpinBox(); s.setValue(1); h.addWidget(s)
    h.addWidget(QtWidgets.QLabel("End")); e = QtWidgets.QSpinBox(); e.setValue(250); h.addWidget(e)
    bar = QtWidgets.QProgressBar(); bar.setRange(0,100); bar.setValue(0); bar.setTextVisible(False); h.addWidget(bar, 1)
    h.addWidget(QtWidgets.QLabel("Frame")); f = QtWidgets.QSpinBox(); f.setValue(1); h.addWidget(f)
    return w


def build_window() -> QtWidgets.QMainWindow:
    win = QtWidgets.QMainWindow()
    win.setWindowTitle("AARTZE Python Editor")
    win.resize(1440, 900)

    # Shell layout
    central = QtWidgets.QWidget(); root = QtWidgets.QVBoxLayout(central); root.setContentsMargins(0,0,0,0); root.setSpacing(0)
    root.addWidget(BrandBar())
    modebar = ModeBar(); root.addWidget(modebar)

    stack = QtWidgets.QStackedWidget()
    # Create lightweight pages first; lazy-create CodeMode on demand to avoid WebEngine startup delay
    pages: dict[str, QtWidgets.QWidget] = {}
    key_to_index: dict[str,int] = {}

    def add_page(key: str, widget: QtWidgets.QWidget):
        pages[key] = widget
        key_to_index[key] = stack.addWidget(widget)

    add_page("editor", EditorMode())
    add_page("auto", AutoMode())
    add_page("shading", ShadingMode())
    # code page is created lazily
    root.addWidget(stack, 1)

    # Status bottom (as in JSX footer)
    status = QtWidgets.QFrame(); s = QtWidgets.QHBoxLayout(status); s.setContentsMargins(8,6,8,6); s.setSpacing(16)

    def status_icon(name: str, tooltip: str):
        b = QtWidgets.QToolButton()
        b.setToolTip(tooltip)
        b.setAutoRaise(True)
        b.setCursor(QtCore.Qt.PointingHandCursor)
        b.setToolButtonStyle(QtCore.Qt.ToolButtonIconOnly)
        icons.set_icon(b, name)
        return b

    left = QtWidgets.QHBoxLayout(); left.setSpacing(12)
    left.addWidget(status_icon("hdd", "Content Drawer"))
    left.addWidget(status_icon("terminal", "Output Log"))
    left.addWidget(status_icon("terminal", "Cmd"))
    s.addLayout(left)
    s.addStretch()
    right = QtWidgets.QHBoxLayout(); right.setSpacing(12)
    right.addWidget(status_icon("save", "All Saved"))
    right.addWidget(status_icon("branch", "Revision Control"))
    s.addLayout(right)
    root.addWidget(status)

    def on_mode(k: str):
        # Lazy create CodeMode when first requested
        if k == "code" and "code" not in pages:
            w = CodeMode()
            add_page("code", w)
        idx = key_to_index.get(k, 0)
        stack.setCurrentIndex(idx)
    modebar.modeChanged.connect(on_mode)

    # Workspace switching maps to pages (Layout/Modeling ÃƒÂ¢Ã¢â‚¬Â Ã¢â‚¬â„¢ editor, Shading ÃƒÂ¢Ã¢â‚¬Â Ã¢â‚¬â„¢ shading)
    def on_workspace(name: str):
        if name == "Shading":
            stack.setCurrentIndex(key_to_index.get("shading", 0))
        else:
            stack.setCurrentIndex(key_to_index.get("editor", 0))
    modebar.workspaceChanged.connect(on_workspace)

    # Start in editor view
    on_mode("editor")

    win.setCentralWidget(central)

    # ---------- stylesheet ----------
    qss_path = __file__.replace("app.py", "qss/aartze_dark.qss")
    try:
        with open(qss_path, "r", encoding="utf8") as f:
            win.setStyleSheet(f.read())
    except Exception:
        pass

    # ---------- global shortcuts ----------
    def toggle_maximize():
        ed = pages.get("editor")
        if not ed:
            return
        # Hide/show outliner and props to maximize viewport
        hidden = not ed.props.isVisible()
        ed.props.setVisible(hidden)
        try:
            ed.outliner.setVisible(hidden)
        except Exception:
            pass

    def toggle_props():
        ed = pages.get("editor");
        if ed: ed.props.setVisible(not ed.props.isVisible())

    def toggle_tools():
        ed = pages.get("editor");
        if ed: ed.shelf.setVisible(not ed.shelf.isVisible())

    QtGui.QShortcut(QtGui.QKeySequence("Ctrl+Space"), win, activated=toggle_maximize)
    QtGui.QShortcut(QtGui.QKeySequence("N"), win, activated=toggle_props)
    QtGui.QShortcut(QtGui.QKeySequence("T"), win, activated=toggle_tools)

    return win


def _row(label: str):
    w = QtWidgets.QWidget(); lay = QtWidgets.QHBoxLayout(w); lay.setContentsMargins(0, 0, 0, 0); lay.setSpacing(8)
    lab = QtWidgets.QLabel(label); lab.setProperty("muted", True)
    lay.addWidget(lab)
    for a in ("X", "Y", "Z"):
        box = QtWidgets.QDoubleSpinBox(); box.setDecimals(3); box.setRange(-1e6, 1e6)
        if label == "Scale":
            box.setValue(1.0)
        lay.addWidget(QtWidgets.QLabel(a)); lay.addWidget(box)
    lay.addStretch(1)
    return w


def _props_panel():
    w = QtWidgets.QWidget(); v = QtWidgets.QVBoxLayout(w); v.setContentsMargins(8, 8, 8, 8); v.setSpacing(10)
    # Import tabs (hook to Assimp later)
    tabs = QtWidgets.QTabWidget()
    for n in ("Blend (.blend)", "FBX (.fbx)", "glTF (.gltf/.glb)", "OBJ (.obj)", "USD (.usd/.usdz)"):
        tabs.addTab(QtWidgets.QWidget(), n)
    v.addWidget(tabs)
    v.addWidget(QtWidgets.QLabel("Transform"))
    v.addWidget(_row("Location"))
    v.addWidget(_row("Rotation"))
    v.addWidget(_row("Scale"))
    v.addStretch(1)
    return w


    
