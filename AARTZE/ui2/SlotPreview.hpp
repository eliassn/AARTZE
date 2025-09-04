#pragma once
#include <memory>
#include <vector>
#include "Widget.hpp"
#include "../ui_slot/Core.hpp"
#include "../ui_slot/Canvas.hpp"
#include "../ui_slot/Border.hpp"
#include "../ui_slot/StackPanel.hpp"
#include "../ui_slot/Spacer.hpp"

struct GLFWwindow; // fwd decl

namespace ui2 {

// Renders a small UiSlot layout inside a panel to validate Arrange.
class SlotPreview : public Widget {
public:
    SlotPreview();
    void layout(const Rect& b) override;
    void paint(DrawList& dl) override;
    // Input binding
    void setWindow(GLFWwindow* w) { m_window = w; }
    void tick();
    // Selection access
    int selected() const { return m_selected; }
    UiCanvasSlot* selectedCanvasSlot();
private:
    std::shared_ptr<UiWidget> m_root;
    std::vector<UiArrangedItem> m_items;
    std::vector<UiCanvasSlot*> m_canvasSlots; // matches initial children order
    GLFWwindow* m_window{nullptr};
    int m_selected{-1};
};

} // namespace ui2
