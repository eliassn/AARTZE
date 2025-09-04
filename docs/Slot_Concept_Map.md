# UE → AARTZE Slot/Slate Concept Mapping

Goal: Mirror UE/Slate’s composable widget + slot system while using distinct AARTZE names/APIs and small behavior changes to avoid verbatim copying.

Core Types
- UE `SWidget` → AARTZE `UiWidget` (base; measure/arrange/paint; input hooks)
- UE `SLeafWidget` → AARTZE `UiLeaf` (optional helper base for leaf widgets)
- UE `SPanel` → AARTZE `UiPanel` (container; manages children + slots)
- UE `PanelChildren` → AARTZE `UiChildren` (vector-like child accessor)
- UE `PanelSlot` (various nested `FSlot`) → AARTZE `UiPanelSlot` (base) + per-container slot structs
- UE `FGeometry` → AARTZE `UiGeometry` (rect, scale, transforms)
- UE `FSlateLayoutTransform`/`FSlateRenderTransform` → AARTZE `UiTransform`
- UE `FArrangedChildren` → AARTZE `UiArranged` (list of arranged child geometries)
- UE `EVisibility` → AARTZE `UiVisibility` (Visible, HitTestInvisible, SelfHitTestInvisible, Collapsed)
- UE `FMargin` → AARTZE `UiMargin`
- UE `FSlateBrush` → AARTZE `UiBrush` (image/rounded-rect/solid; colors, borders)
- UE `FSlateColor` → AARTZE `UiColor`
- UE `FSlateFontInfo` → AARTZE `UiFont`
- UE `FPaintArgs`/`FSlateWindowElementList` → AARTZE `UiPaintCtx` (target draw list + layers)

Lifecycle & Layout
- UE `ComputeDesiredSize` → AARTZE `Measure(UiMeasureCtx)` returns desired size
- UE `OnArrangeChildren` → AARTZE `Arrange(const UiGeometry& parent, UiArranged& out)`
- UE `OnPaint` → AARTZE `Paint(UiPaintCtx&, const UiGeometry&)`
- UE `Tick` → AARTZE `Tick(float dt)`
- UE invalidation panel → AARTZE `UiInvalidation` (dirty flags per subtree)

Input & Focus
- UE `OnMouseButtonDown/Up/Move/Wheel` → AARTZE `OnPointerDown/Up/Move/Wheel`
- UE `SupportsKeyboardFocus`/`OnKeyDown` → AARTZE `IsFocusable`/`OnKeyDown`
- UE focus manager → AARTZE `UiFocusMgr` (tab order, capture)

Container Widgets & Slots
- UE `SOverlay` + `SOverlay::FOverlaySlot`
  → AARTZE `UiOverlay` + `UiOverlaySlot { UiMargin Padding; UiHAlign HAlign; UiVAlign VAlign; int ZOrder; }`
- UE `SCanvas` + `SCanvas::FSlot`
  → AARTZE `UiCanvas` + `UiCanvasSlot { UiAnchors Anchors; Vec2 Position; Vec2 Size; bool AutoSize; Vec2 Alignment; int ZOrder; }`
- UE `SBorder` (content + brush)
  → AARTZE `UiBorder { UiBrush Brush; UiMargin Padding; UiHAlign HAlign; UiVAlign VAlign; child }`
- UE `SSizeBox` (width/height overrides)
  → AARTZE `UiSizeBox { std::optional<float> WidthOverride, HeightOverride, MinWidth, MinHeight, MaxWidth, MaxHeight; child }`
- UE `SHorizontalBox`/`SVerticalBox` + `SBoxPanel::FSlot`
  → AARTZE `UiStackPanel(orientation)` + `UiStackSlot { UiMargin Padding; UiHAlign HAlign; UiVAlign VAlign; UiSizeRule SizeRule; float Fill; }`
- UE `SGridPanel` + `SGridPanel::FSlot`
  → AARTZE `UiGrid` + `UiGridSlot { int Row, Column, RowSpan, ColumnSpan; UiMargin Padding; UiHAlign HAlign; UiVAlign VAlign; }`
- UE `SUniformGridPanel`
  → AARTZE `UiUniformGrid` + `UiUniformGridSlot { Row, Column; }`
- UE `SScrollBox`
  → AARTZE `UiScrollBox { Orientation; Scrollbar; Content }`
- UE `SSpacer`
  → AARTZE `UiSpacer { Vec2 Size }`
- UE `SImage`
  → AARTZE `UiImage { UiBrush Brush; UiColor Tint }`
- UE `STextBlock`
  → AARTZE `UiText { string Text; UiTextStyle Style; Wrap; Justification }`
- UE `SButton` + `SBorder` composition
  → AARTZE `UiButton { UiBrush Normal/Hovered/Pressed; Pressed(), Released() signals; child }`

Enums & Value Types
- UE `EHorizontalAlignment`/`EVerticalAlignment` → AARTZE `UiHAlign`/`UiVAlign` (Fill, Left/Center/Right; Top/Center/Bottom)
- UE `EOrientation` → AARTZE `UiOrientation` (Horizontal/Vertical)
- UE `ESlateSizeRule` → AARTZE `UiSizeRule` (Auto, Fill)
- UE `FAnchors` → AARTZE `UiAnchors { Vec2 Min, Max }`

Binding & Data Flow
- UE `TAttribute<T>`/`Delegates` → AARTZE `UiBinding<T>` or `std::function<T()>` + change notifications (signal bus). Prefer pull-based binding with explicit `Invalidate(DesiredSize/Layout/Paint)` on changes.

Serialization & Editing
- UE UMG blueprint graph + reflection → AARTZE JSON asset: `{ widget: "UiCanvas", children: [...], slot: {...} }`
- Editor exposes slot properties (Padding/Alignment/Anchors/ZOrder/etc.) with per-property Reset; content browser thumbnails from preview render.

Painting Backends
- UE Slate RHI batching → AARTZE backends:
  - Editor preview: ImGui draw list bridge (fast prototyping)
  - Runtime: engine renderer (OpenGL/Vulkan) `UiPainter` that emits quads, text, nine-slice, clips, layers

Behavioral Deviations (intentional)
- Distinct names and parameter orders; default padding differs from UE
- JSON-based assets instead of reflection-driven defaults
- Optional reactive bindings; no UE-style `SNew` macros

Minimal Interface Sketch (illustrative)
```cpp
struct UiGeometry { Rect rect; float scale{1.f}; Mat3 xform; };
struct UiMeasureCtx { float dpi; }; struct UiPaintCtx { DrawList* dl; int layer; };

struct UiWidget {
  virtual Vec2 Measure(const UiMeasureCtx&) = 0;
  virtual void Arrange(const UiGeometry& parent, UiArranged& out) = 0;
  virtual void Paint(UiPaintCtx&, const UiGeometry&) = 0;
  virtual void Tick(float dt) {}
  virtual bool OnPointerDown(const PointerEvent&) { return false; }
  virtual ~UiWidget() = default;
};

struct UiPanelSlot { UiMargin Padding; UiHAlign HAlign; UiVAlign VAlign; };
```

Next Actions
- Confirm reference repo path (nafs-engine-release)
- Freeze naming and slot property set per container
- Draft headers under `AARTZE/ui_slot/` following this map
