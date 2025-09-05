from PySide6 import QtOpenGLWidgets, QtCore, QtGui

# Try engine bindings (prefer 'aartze', then 'aartzepy').
try:
    import aartze as eng
except Exception:
    try:
        import aartzepy as eng
    except Exception:
        eng = None


class _OrbitCamera:
    def __init__(self):
        self.target = [0.0, 0.0, 0.0]
        self.dist = 6.0
        self.yaw = 45.0
        self.pitch = 25.0
        self.pan = [0.0, 0.0]


class GLViewport(QtOpenGLWidgets.QOpenGLWidget):
    transformChanged = QtCore.Signal(tuple, tuple, tuple)  # (pos xyz), (rot xyz), (scale xyz)
    def __init__(self, parent=None):
        super().__init__(parent)
        self.setAutoFillBackground(False)
        self.cam = _OrbitCamera()
        self._last = None
        self.setFocusPolicy(QtCore.Qt.StrongFocus)
        # Simple selection/transform state for fallback mode
        self._sel_name = "Cube"
        self._pos = [0.0, 0.0, 0.0]
        self._rot = [0.0, 0.0, 0.0]
        self._scl = [1.0, 1.0, 1.0]

    def initializeGL(self):
        try:
            from OpenGL import GL
            GL.glEnable(GL.GL_DEPTH_TEST)
            GL.glClearColor(0.07, 0.07, 0.08, 1.0)
        except Exception:
            pass
        try:
            if eng:
                eng.init()
                # Pull initial cube transform if available
                cid = int(eng.get_entity_id("Cube")) if hasattr(eng, 'get_entity_id') else -1
                if cid >= 0 and hasattr(eng, 'get_transform'):
                    p, r = eng.get_transform(cid)
                    self._pos[:] = list(p)
                    self._rot[:] = list(r)
                    self.transformChanged.emit(tuple(self._pos), tuple(self._rot), tuple(self._scl))
        except Exception:
            pass

    def resizeGL(self, w, h):
        try:
            from OpenGL import GL
            GL.glViewport(0, 0, int(w), int(h))
        except Exception:
            pass
        try:
            if eng:
                eng.resize(int(w), int(h))
        except Exception:
            pass

    def paintGL(self):
        self.makeCurrent()
        draw_fallback = True
        try:
            if eng:
                eng.render()
                draw_fallback = False
        except Exception:
            draw_fallback = True

        if draw_fallback:
            try:
                from OpenGL import GL
                import math
                GL.glClear(GL.GL_COLOR_BUFFER_BIT | GL.GL_DEPTH_BUFFER_BIT)

                # Basic projection/view
                aspect = max(0.001, self.width() / max(1.0, float(self.height())))
                fov = 45.0
                znear, zfar = 0.1, 100.0

                def deg2rad(a): return a * math.pi / 180.0

                def perspective(fovy, a, n, f):
                    t = math.tan(deg2rad(fovy) / 2.0)
                    return [1/(a*t),0,0,0, 0,1/t,0,0, 0,0,-(f+n)/(f-n),-1, 0,0,-(2*f*n)/(f-n),0]

                def look_at(eye, center, up=(0,1,0)):
                    ex,ey,ez = eye; cx,cy,cz = center; ux,uy,uz = up
                    fx,fy,fz = cx-ex, cy-ey, cz-ez
                    fl = math.sqrt(fx*fx+fy*fy+fz*fz); fx/=fl; fy/=fl; fz/=fl
                    rl = math.sqrt((fy*uz-fz*uy)**2 + (fz*ux-fx*uz)**2 + (fx*uy-fy*ux)**2)
                    rx,ry,rz = (fy*uz-fz*uy)/rl, (fz*ux-fx*uz)/rl, (fx*uy-fy*ux)/rl
                    ux,uy,uz = (ry*fz-rz*fy, rz*fx-rx*fz, rx*fy-ry*fx)
                    return [rx,ux,-fx,0, ry,uy,-fy,0, rz,uz,-fz,0,
                            -(rx*ex+ry*ey+rz*ez), -(ux*ex+uy*ey+uz*ez), (fx*ex+fy*ey+fz*ez), 1]

                # Camera position from yaw/pitch/dist
                cy = deg2rad(self.cam.yaw); cp = deg2rad(self.cam.pitch)
                cx = self.cam.target[0]-self.cam.pan[0]; cz = self.cam.target[2]-self.cam.pan[1]
                x = cx + self.cam.dist*math.cos(cp)*math.cos(cy)
                y = self.cam.target[1] + self.cam.dist*math.sin(cp)
                z = cz + self.cam.dist*math.cos(cp)*math.sin(cy)

                P = perspective(fov, aspect, znear, zfar)
                V = look_at((x,y,z), (self.cam.target[0], self.cam.target[1], self.cam.target[2]), (0,1,0))

                # Fixed pipeline matrices
                GL.glMatrixMode(GL.GL_PROJECTION); GL.glLoadMatrixf((GL.GLfloat * 16)(*P))
                GL.glMatrixMode(GL.GL_MODELVIEW);  GL.glLoadMatrixf((GL.GLfloat * 16)(*V))

                # Grid on XZ
                GL.glLineWidth(1.0)
                GL.glColor4f(1,1,1,0.08)
                GL.glBegin(GL.GL_LINES)
                N=20; S=1.0
                for i in range(-N, N+1):
                    GL.glVertex3f(i*S,0,-N*S); GL.glVertex3f(i*S,0,N*S)
                    GL.glVertex3f(-N*S,0,i*S); GL.glVertex3f(N*S,0,i*S)
                GL.glEnd()

                # Axis lines
                GL.glLineWidth(2.0)
                GL.glBegin(GL.GL_LINES)
                GL.glColor3f(1,0,0); GL.glVertex3f(0,0,0); GL.glVertex3f(2,0,0)
                GL.glColor3f(0,1,0); GL.glVertex3f(0,0,0); GL.glVertex3f(0,2,0)
                GL.glColor3f(0,0,1); GL.glVertex3f(0,0,0); GL.glVertex3f(0,0,2)
                GL.glEnd()

                # Simple camera gizmo near origin (Blender-like outline)
                GL.glPushMatrix()
                GL.glTranslatef(-3.0, 1.6, -2.2)
                GL.glColor3f(1.0, 0.64, 0.0)
                L=1.2; n=0.15; f=0.45
                GL.glLineWidth(2.0)
                GL.glBegin(GL.GL_LINES)
                # tip to far corners
                GL.glVertex3f(0,0,0); GL.glVertex3f(L, f, f)
                GL.glVertex3f(0,0,0); GL.glVertex3f(L, f,-f)
                GL.glVertex3f(0,0,0); GL.glVertex3f(L,-f, f)
                GL.glVertex3f(0,0,0); GL.glVertex3f(L,-f,-f)
                # far rectangle
                GL.glVertex3f(L, f, f);  GL.glVertex3f(L, f,-f)
                GL.glVertex3f(L, f,-f);  GL.glVertex3f(L,-f,-f)
                GL.glVertex3f(L,-f,-f);  GL.glVertex3f(L,-f, f)
                GL.glVertex3f(L,-f, f);  GL.glVertex3f(L, f, f)
                GL.glEnd()
                GL.glBegin(GL.GL_TRIANGLES)
                GL.glVertex3f(0.1, 0, 0); GL.glVertex3f(-0.1, 0.18, 0); GL.glVertex3f(-0.1,-0.18, 0)
                GL.glEnd()
                GL.glPopMatrix()

                # Cube
                def v(x,y,z): GL.glVertex3f(x,y,z)
                GL.glColor3f(0.95,0.76,0.2)
                GL.glBegin(GL.GL_QUADS); s=0.5
                GL.glPushMatrix()
                GL.glTranslatef(self._pos[0], self._pos[1], self._pos[2])
                GL.glNormal3f(1,0,0);  v(s,-s,-s); v(s,-s,s); v(s,s,s); v(s,s,-s)
                GL.glNormal3f(-1,0,0); v(-s,-s,s); v(-s,-s,-s); v(-s,s,-s); v(-s,s,s)
                GL.glNormal3f(0,1,0);  v(-s,s,-s); v(s,s,-s); v(s,s,s); v(-s,s,s)
                GL.glNormal3f(0,-1,0); v(-s,-s,s); v(s,-s,s); v(s,-s,-s); v(-s,-s,-s)
                GL.glNormal3f(0,0,1);  v(-s,-s,s); v(-s,s,s); v(s,s,s); v(s,-s,s)
                GL.glNormal3f(0,0,-1); v(s,-s,-s); v(s,s,-s); v(-s,s,-s); v(-s,-s,-s)
                GL.glPopMatrix()
                GL.glEnd()
            except Exception:
                pass

        # Axis triad overlay (Blender-like, top-right with circles)
        p = QtGui.QPainter(self)
        p.setRenderHint(QtGui.QPainter.Antialiasing, True)
        margin = 24
        center = QtCore.QPointF(self.width() - margin, margin)
        def circle(pt, r, color, label):
            col = QtGui.QColor(color)
            pen = QtGui.QPen(col); pen.setWidth(2); p.setPen(pen); p.setBrush(QtCore.Qt.NoBrush)
            p.drawEllipse(pt, r, r)
            f = QtGui.QFont(); f.setPixelSize(10); f.setBold(True); p.setFont(f)
            p.setPen(QtGui.QPen(col)); p.drawText(QtCore.QRectF(pt.x()-r, pt.y()-r, 2*r, 2*r), QtCore.Qt.AlignCenter, label)
        # lines from center
        def line(to, color):
            pen = QtGui.QPen(QtGui.QColor(color)); pen.setWidth(2); p.setPen(pen)
            p.drawLine(center, to)
        xpt = QtCore.QPointF(center.x()+22, center.y())
        ypt = QtCore.QPointF(center.x(), center.y()+22)
        zpt = QtCore.QPointF(center.x()-16, center.y()+16)
        line(xpt, '#ef4444'); line(ypt, '#22c55e'); line(zpt, '#3b82f6')
        circle(xpt, 10, '#ef4444', 'X'); circle(ypt, 10, '#22c55e', 'Y'); circle(zpt, 10, '#3b82f6', 'Z')
        p.end()

    # Camera controls (Alt+mouse)
    def mousePressEvent(self, e):
        self._last = e.position()

    def mouseMoveEvent(self, e):
        if self._last is None:
            return
        if e.modifiers() & QtCore.Qt.AltModifier:
            d = e.position() - self._last
            if eng:
                try:
                    if e.buttons() & QtCore.Qt.LeftButton:
                        eng.camera_orbit_delta(float(-d.x()*0.3), float(-d.y()*0.3))
                    elif e.buttons() & QtCore.Qt.MiddleButton:
                        eng.camera_pan_delta(float(d.x()*0.01), float(-d.y()*0.01))
                    elif e.buttons() & QtCore.Qt.RightButton:
                        eng.camera_dolly_factor(float(1.0 - d.y()*0.005))
                except Exception:
                    pass
            else:
                self.cam.yaw -= d.x()*0.3; self.cam.pitch -= d.y()*0.3
                self.cam.pitch = max(-89.0, min(89.0, self.cam.pitch))
            self.update()
        else:
            # Object move (G): drag on XZ plane
            if e.buttons() & QtCore.Qt.LeftButton:
                d = e.position() - self._last
                self._pos[0] += float(d.x())*0.01
                self._pos[2] -= float(d.y())*0.01
                # Propagate to engine if available
                try:
                    if eng and hasattr(eng, 'get_entity_id') and hasattr(eng, 'set_transform'):
                        cid = int(eng.get_entity_id(self._sel_name))
                        eng.set_transform(cid, float(self._pos[0]), float(self._pos[1]), float(self._pos[2]),
                                          float(self._rot[0]), float(self._rot[1]), float(self._rot[2]))
                except Exception:
                    pass
                # Notify UI
                self.transformChanged.emit(tuple(self._pos), tuple(self._rot), tuple(self._scl))
                self.update()
        self._last = e.position()

    def mouseReleaseEvent(self, e):
        self._last = None

    def wheelEvent(self, e):
        if eng:
            try:
                eng.camera_dolly_factor(0.9 if e.angleDelta().y() > 0 else 1.1)
            except Exception:
                pass
        else:
            self.cam.dist *= 0.9 if e.angleDelta().y() > 0 else 1.1
            self.cam.dist = max(1.0, min(50.0, self.cam.dist))
        self.update()

    # --- External control from Properties panel ---
    @QtCore.Slot(float, float, float)
    def setLocation(self, x: float, y: float, z: float):
        self._pos[:] = [float(x), float(y), float(z)]
        try:
            if eng and hasattr(eng, 'get_entity_id') and hasattr(eng, 'set_transform'):
                cid = int(eng.get_entity_id(self._sel_name))
                eng.set_transform(cid, float(x), float(y), float(z),
                                  float(self._rot[0]), float(self._rot[1]), float(self._rot[2]))
        except Exception:
            pass
        self.update()

    # Selected entity from Outliner
    @QtCore.Slot(str)
    def setSelectedName(self, name: str):
        self._sel_name = name or "Cube"
        # Pull transform from engine if possible
        try:
            if eng and hasattr(eng, 'get_entity_id') and hasattr(eng, 'get_transform'):
                cid = int(eng.get_entity_id(self._sel_name))
                if cid >= 0:
                    p, r = eng.get_transform(cid)
                    self._pos[:] = list(p); self._rot[:] = list(r)
                    self.transformChanged.emit(tuple(self._pos), tuple(self._rot), tuple(self._scl))
                    self.update()
        except Exception:
            pass
