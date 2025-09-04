#include "SlotPreview.hpp"
#include "StyleSet.hpp"
#include <GLFW/glfw3.h>

namespace ui2 {

using std::make_shared;

SlotPreview::SlotPreview()
{
    auto canvas = std::make_shared<UiCanvas>();

    // Left panel
    {
        auto border = make_shared<UiBorder>();
        border->padding = {8,8,8,8};
        auto stack = make_shared<UiStackPanel>(UiOrientation::Vertical);
        stack->Add(make_shared<UiSpacer>(glm::vec2{140,22}));
        stack->Add(make_shared<UiSpacer>(glm::vec2{140,28}));
        stack->Add(make_shared<UiSpacer>(glm::vec2{140,18}));
        border->SetChild(stack);
        UiCanvasSlot& s = canvas->Add(border);
        s.anchors = {{0,0},{0,0}};
        s.offset = {10,10,200,120};
        s.alignment = {0,0};
        m_canvasSlots.push_back(&s);
    }

    // Right panel
    {
        auto border = make_shared<UiBorder>();
        border->padding = {6,6,6,6};
        border->hAlign = UiHAlign::Center; border->vAlign = UiVAlign::Center;
        border->SetChild(make_shared<UiSpacer>(glm::vec2{120,60}));
        UiCanvasSlot& s = canvas->Add(border);
        s.anchors = {{1,0},{1,0}};
        s.offset = {-210,10,200,120};
        s.alignment = {0,0};
        m_canvasSlots.push_back(&s);
    }

    // Bottom bar stretched
    {
        auto border = make_shared<UiBorder>();
        border->padding = {4,4,4,4};
        border->SetChild(make_shared<UiSpacer>(glm::vec2{100,24}));
        UiCanvasSlot& s = canvas->Add(border);
        s.anchors = {{0,1},{1,1}};
        s.offset = {10,-40,10,30};
        s.alignment = {0,0};
        m_canvasSlots.push_back(&s);
    }

    m_root = canvas;
}

void SlotPreview::layout(const Rect& b)
{
    Widget::layout(b);
    m_items.clear();
    UiGeometry parent{}; parent.rect = { {b.x,b.y}, {b.w,b.h} }; parent.dpiScale = 1.0f;
    UiArranged arr; if (m_root) m_root->Arrange(parent, arr); m_items = std::move(arr.items);
}

void SlotPreview::paint(DrawList& dl)
{
    auto s = GetStyle();
    unsigned accent = ((unsigned)(s.Accent.a*255)<<24)|((unsigned)(s.Accent.b*255)<<16)|((unsigned)(s.Accent.g*255)<<8)|((unsigned)(s.Accent.r*255));
    unsigned panel  = ((unsigned)(s.PanelBg.a*120)<<24)|((unsigned)(s.PanelBg.b*255)<<16)|((unsigned)(s.PanelBg.g*255)<<8)|((unsigned)(s.PanelBg.r*255));
    dl.rectFilled(m_bounds.x, m_bounds.y, m_bounds.w, m_bounds.h, panel);
    for (size_t i=0;i<m_items.size();++i)
    {
        const auto& r = m_items[i].geo.rect;
        dl.rectFilled(r.pos.x, r.pos.y, r.size.x, r.size.y, accent);
    }
    if (m_selected >= 0 && m_selected < (int)m_items.size())
    {
        const auto& r = m_items[(size_t)m_selected].geo.rect;
        unsigned sel = ((unsigned)(s.Accent.a*140)<<24)|((unsigned)(230)<<16)|((unsigned)(40)<<8)|((unsigned)(200));
        dl.rectFilled(r.pos.x-2, r.pos.y-2, r.size.x+4, 2, sel);
        dl.rectFilled(r.pos.x-2, r.pos.y+r.size.y, r.size.x+4, 2, sel);
        dl.rectFilled(r.pos.x-2, r.pos.y, 2, r.size.y, sel);
        dl.rectFilled(r.pos.x+r.size.x, r.pos.y, 2, r.size.y, sel);
    }
}

void SlotPreview::tick()
{
    if (!m_window) return;
    double mx,my; glfwGetCursorPos(m_window,&mx,&my);
    static bool prevPressed=false; bool pressed = glfwGetMouseButton(m_window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS;
    if (pressed && !prevPressed)
    {
        for (int i=(int)m_items.size()-1;i>=0;--i)
        {
            const auto& r = m_items[(size_t)i].geo.rect;
            if (mx >= r.pos.x && mx <= r.pos.x + r.size.x && my >= r.pos.y && my <= r.pos.y + r.size.y)
            { m_selected = i; break; }
        }
    }
    prevPressed = pressed;

    UiCanvasSlot* slot = selectedCanvasSlot();
    if (!slot) return;
    float step = (glfwGetKey(m_window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) ? 10.0f : 1.0f;

    if (glfwGetKey(m_window, GLFW_KEY_R) == GLFW_PRESS)
    { slot->anchors = {{0,0},{0,0}}; slot->offset = {0,0,1,1}; slot->alignment = {0.5f,0.5f}; }

    bool left  = glfwGetKey(m_window, GLFW_KEY_LEFT)  == GLFW_PRESS;
    bool right = glfwGetKey(m_window, GLFW_KEY_RIGHT) == GLFW_PRESS;
    bool up    = glfwGetKey(m_window, GLFW_KEY_UP)    == GLFW_PRESS;
    bool down  = glfwGetKey(m_window, GLFW_KEY_DOWN)  == GLFW_PRESS;
    bool ctrl  = glfwGetKey(m_window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS || glfwGetKey(m_window, GLFW_KEY_RIGHT_CONTROL) == GLFW_PRESS;
    bool alt   = glfwGetKey(m_window, GLFW_KEY_LEFT_ALT) == GLFW_PRESS || glfwGetKey(m_window, GLFW_KEY_RIGHT_ALT) == GLFW_PRESS;

    if (ctrl)
    {
        if (left)  { slot->anchors.min.x -= step*0.01f; slot->anchors.max.x -= step*0.01f; }
        if (right) { slot->anchors.min.x += step*0.01f; slot->anchors.max.x += step*0.01f; }
        if (up)    { slot->anchors.min.y -= step*0.01f; slot->anchors.max.y -= step*0.01f; }
        if (down)  { slot->anchors.min.y += step*0.01f; slot->anchors.max.y += step*0.01f; }
    }
    else if (alt)
    {
        if (left)  slot->alignment.x -= step*0.01f;
        if (right) slot->alignment.x += step*0.01f;
        if (up)    slot->alignment.y -= step*0.01f;
        if (down)  slot->alignment.y += step*0.01f;
    }
    else
    {
        if (left)  slot->offset.left  -= step;
        if (right) slot->offset.left  += step;
        if (up)    slot->offset.top   -= step;
        if (down)  slot->offset.top   += step;
        if (glfwGetKey(m_window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
        {
            if (left)  slot->offset.right -= step;
            if (right) slot->offset.right += step;
            if (up)    slot->offset.bottom -= step;
            if (down)  slot->offset.bottom += step;
        }
    }
}

UiCanvasSlot* SlotPreview::selectedCanvasSlot()
{
    if (m_selected < 0) return nullptr;
    size_t idx = (size_t)m_selected;
    if (idx < m_canvasSlots.size()) return m_canvasSlots[idx];
    return nullptr;
}

} // namespace ui2
