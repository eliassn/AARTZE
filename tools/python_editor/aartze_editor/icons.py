from typing import Optional

try:
    import qtawesome as qta  # type: ignore
except Exception:  # pragma: no cover
    qta = None  # gracefully degrade if not installed


_MAP = {
    "play": ["fa5s.play"],
    "pause": ["fa5s.pause"],
    "stop": ["fa5s.stop"],
    "monitor": ["fa5s.desktop"],
    "camera": ["fa5s.camera"],
    "sun": ["fa5s.sun"],
    "layers": ["fa5s.layer-group", "fa5s.layers"],
    "boxes": ["fa5s.boxes", "fa5s.cubes"],
    "box": ["fa5s.box", "fa5s.cube"],
    "folder": ["fa5s.folder"],
    "chevron-down": ["fa5s.chevron-down"],
    "chevron-right": ["fa5s.chevron-right"],
    "search": ["fa5s.search"],
    "hdd": ["fa5s.hdd"],
    "terminal": ["fa5s.terminal"],
    "save": ["fa5s.save"],
    "branch": ["fa5s.code-branch"],
    "loader": ["fa5s.sync"],
    "move": ["fa5s.arrows-alt"],
    "rotate": ["fa5s.sync-alt", "fa5s.redo"],
    "scale": ["fa5s.expand-arrows-alt"],
    "magnet": ["fa5s.magnet"],
    "grid": ["fa5s.th-large"],
    "eye": ["fa5s.eye"],
    "eye-slash": ["fa5s.eye-slash"],
    "lock": ["fa5s.lock"],
    "unlock": ["fa5s.unlock"],
    "ruler": ["fa5s.ruler-combined", "fa5s.ruler"],
    "crosshair": ["fa5s.crosshairs"],
    "brush": ["fa5s.paint-brush"],
    "shapes": ["fa5s.shapes"],
    "code": ["fa5s.code"],
    "file-code": ["fa5s.file-code"],
    "play-circle": ["fa5s.play-circle"],
    "bug": ["fa5s.bug"],
    "settings": ["fa5s.cog"],
    "file-plus": ["fa5s.file-medical"],
    "folder-plus": ["fa5s.folder-plus"],
    # Workspace menu additions
    "layout": ["fa5s.th-large", "fa5s.border-all"],
    "modeling": ["fa5s.cube", "fa5s.box"],
    "sculpting": ["fa5s.paint-brush", "fa5s.draw-polygon"],
    "uv": ["fa5s.expand-arrows-alt", "fa5s.object-ungroup"],
    "texture": ["fa5s.brush"],
    "shading": ["fa5s.adjust", "fa5s.shapes"],
    "animation": ["fa5s.film", "fa5s.running"],
    "rendering": ["fa5s.camera"],
    "compositing": ["fa5s.layer-group", "fa5s.project-diagram"],
    "geometry-nodes": ["fa5s.code-branch", "fa5s.boxes"],
    "scripting": ["fa5s.code"],
}


def get(name: str, color: str = "#e5e7eb"):
    if qta is None:
        return None
    for key in _MAP.get(name, []):
        try:
            return qta.icon(key, color=color)
        except Exception:
            continue
    return None


def set_icon(widget, name: str, color: str = "#e5e7eb") -> Optional[object]:
    ico = get(name, color)
    if ico is not None and hasattr(widget, "setIcon"):
        widget.setIcon(ico)
        return ico
    return None
