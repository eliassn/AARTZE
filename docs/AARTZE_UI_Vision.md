# AARTZE Editor UI — 24h Visual + Feature Spec

Goal: Match the clarity and productivity of UE’s editor using our stack (ImGui + docking), with no blocking modals and zero perceptible UI lag.

Target Layout (dock preset)

```
┌──────────────────────────────────────────────────────────────────────────────┐
│ Menu Bar (File | Window | Tools | Help)                                     │
├──────────────────────────────────────────────────────────────────────────────┤
│ Toolbar: [Play] | Perspective | Lit | Snap [ 1.00 ] | 1 2 3 | Camera Speed  │
├───────────────┬───────────────────────────────────────────┬──────────────────┤
│ Scene Outliner│                 Viewport                   │     Details      │
│ (folders)     │ (gizmo, overlays, drag & drop)            │ (search, props)  │
├───────────────┴───────────────────────────────────────────┴──────────────────┤
│                              Content Browser                                 │
│ Breadcrumb | Folder tree | Grid (thumbnails) | Context menu | Drag & drop    │
├──────────────────────────────────────────────────────────────────────────────┤
│ Status: Save All | Content Drawer | Output Log | Cmd | Streaming: N          │
└──────────────────────────────────────────────────────────────────────────────┘
```

Theme (AARTZEStyle)

- Typography: Inter/Roboto 17pt (fallback: Segoe UI). 1.2× scale on 4K.
- Colors: Deep dark bg (#17181C). Headers/hover accent magenta (#D51EAA). Mid panels (#2A2D34). Text (#F3F4F7).
- Metrics: 6px rounding, 8–10px padding, 4px gaps.

UE-grade Features (24h scope)

1) Commands & Menus
- Command registry + palette (Ctrl+P). Shortcuts: Ctrl+N/O/S/Shift+S, Space for Play/Stop. Menus/toolbar invoke commands.

2) Viewport
- Drag & drop asset spawns actor (raycast spawn-at-cursor planned). Gizmo translate/rotate/scale with snap. Overlays: grid/axes.

3) Details (Inspector)
- Search within categories. Per-property reset-to-default: Transform has Reset; more components follow.

4) Content Browser / Drawer
- Breadcrumb path, folder tree + grid. Context actions (Spawn, Delete, Favorite), drag & drop into Viewport.
- Async thumbnailing + cached indexing (avoid per-frame disk walks).

5) Outliner
- Folder grouping, search, per-item visibility/lock toggles. Context: Delete implemented; Duplicate/Focus queued.

6) Status Bar & Notifications
- Save All, Cmd palette button, background task progress (streaming). Non-blocking toasts.

Performance Budget

- UI frame ≤ 0.5 ms on 1080p; ≤ 1.0 ms on 4K. No blocking modals; file IO async or native pickers. No per-frame filesystem scanning.

Milestones (time-boxed)

- T+4h: Spawn at cursor (raycast); Inspector per-property reset; Window/Render command coverage.
- T+10h: Content Browser multi-select + context actions; Outliner menu actions; toolbar icons.
- T+18h: Async thumbnailing; Command palette “recent” quick actions; layout preset saved.
- T+24h: Polish pass (paddings/colors), tooltip audit, micro-profiler counters for UI.

Current Status (tracking)

- Completed: Dockspace + layout, menu bar, toolbar, viewport panel, outliner (Delete ctx), inspector (filter + Transform Reset), content browser/drawer with cached index & thumbnails, command palette, non-blocking toasts, status bar.
- In Progress: Spawn at cursor (raycast), per-property reset across components, Outliner Duplicate/Focus, palette recents.

