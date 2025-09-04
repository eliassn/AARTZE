# AARTZE Slot UI — Working Plan & Progress

Objective
- Build a UE-like Slot/Slate UI system (widget tree + slot layout objects) adapted to AARTZE, inspired by UE patterns without copying code.
- Integrate with existing ImGui editor shell while keeping engine runtime independent of ImGui for shipping builds.

Reference
- Source to review: `nafs-engine-release/Engine/Source` (UE-like engine/structure) located alongside this repo. Using it as the main reference for Slot/Slate semantics.

Guiding Differences (to avoid verbatim copying)
- Names: Widget -> UiWidget, SWidget-like -> AWidget; Slot -> UiSlot; PanelSlot -> UiPanelSlot.
- API shape: method names and parameter ordering adjusted; default values differ; styling tokens and enums renamed.
- Serialization: JSON-based widget/slot description for editor/runtime parity (instead of UE reflection annotations).
- Event model: function callbacks + signal bus (no direct UE delegate names).

Phase 1 - Discovery (pending reference path)
- Inventory core concepts: SWidget, SPanel, SLeaf, PanelSlot, container widgets (Overlay, Canvas, SizeBox, Border, Grid).
- Identify layout attributes per slot (padding, alignment, anchors, z-order, size rules).
- Map lifecycle: construct, tick, paint, input routing.

Phase 2 - Minimal Viable Slot UI (API draft)
- Base: UiWidget (tick, draw, desired size), UiPanel (children mgmt), UiPanelSlot (layout props), UiTree (root + traversal).
- Containers: UiCanvas + CanvasSlot (absolute/anchors), UiOverlay + OverlaySlot (z-order), UiSizeBox, UiBorder.
- Text/Image: UiText, UiImage.
- Input: focus, hit-testing, bubbling.

Phase 3 - Editor Integration
- Inspector editors for widget + slot properties; per-property reset.
- JSON import/export for UI assets; Content Browser thumbnails.
- Viewport overlay to preview UI; drag-drop to add widgets.

Phase 4 - Parity & Polish
- Layout metrics parity; DPI scaling; style system; keyboard/gamepad nav.
- Performance budget: <=0.5ms @1080p UI.

Deliverables
- Header-only core interfaces + cpp implementations under `AARTZE/ui_slot/`.
- Demo scene: canvas with border, overlay, text; interactive resize/anchors.
- Tests: serialization roundtrip; layout math checks.

Open Questions
- Confirm nafs-engine-release location.
- Target runtime renderer (OpenGL/Vulkan) hooks for non-ImGui draw.
- Skinning requirements (theme tokens).

Progress Log
- [ ] Gather reference repo path (nafs-engine-release)
- [ ] Concept map (UE -> AARTZE names) committed
- [ ] API draft (headers) committed
- [ ] Minimal containers + slots implemented
- [ ] Editor property panels wired
- [ ] JSON serialization + preview
